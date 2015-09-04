/*
  Copyright (c) by respective owners including Yahoo!, Microsoft, and
  individual contributors. All rights reserved.  Released under a BSD (revised)
  license as described in the file LICENSE.
*/
#include "search_dep_parser.h"
#include "gd.h"
#include "cost_sensitive.h"
#include "label_dictionary.h"   // for add_example_namespaces_from_example
#include "vw.h"
#include "vw_exception.h"

#define val_namespace 100 // valency and distance feature space
#define offset_const 344429

namespace DepParserTask         {  Search::search_task task = { "dep_parser", run, initialize, finish, setup, nullptr};  }

struct task_data {
  example *ex;
  size_t root_label, num_label;
  v_array<uint32_t> valid_actions, action_loss, gold_heads, gold_tags, stack, heads, tags, temp;
  v_array<uint32_t> children[6]; // [0]:num_left_arcs, [1]:num_right_arcs; [2]: leftmost_arc, [3]: second_leftmost_arc, [4]:rightmost_arc, [5]: second_rightmost_arc
  example * ec_buf[13];
  bool old_style_labels;
};

namespace DepParserTask {
using namespace Search;

const action SHIFT        = 1;
const action REDUCE_RIGHT = 2;
const action REDUCE_LEFT  = 3;

void initialize(Search::search& sch, size_t& /*num_actions*/, po::variables_map& vm) {
  vw& all = sch.get_vw_pointer_unsafe();
  task_data *data = new task_data();
  data->action_loss.resize(4,true);
  data->ex = NULL;
  sch.set_num_learners(3);
  sch.set_task_data<task_data>(data);

  new_options(all, "Dependency Parser Options")
  ("root_label", po::value<size_t>(&(data->root_label))->default_value(8), "Ensure that there is only one root in each sentence")
  ("num_label", po::value<size_t>(&(data->num_label))->default_value(12), "Number of arc labels")
  ("old_style_labels", "Use old hack of label information");
  add_options(all);

  check_option<size_t>(data->root_label, all, vm, "root_label", false, size_equal,
                       "warning: you specified a different value for --root_label than the one loaded from regressor. proceeding with loaded value: ", "");
  check_option<size_t>(data->num_label, all, vm, "num_label", false, size_equal,
                       "warning: you specified a different value for --num_label than the one loaded from regressor. proceeding with loaded value: ", "");
  check_option(data->old_style_labels, all, vm, "old_style_labels", false,
               "warning: you specified a different value for --old_style_labels than the one loaded from regressor. proceeding with loaded value: ");

  data->ex = VW::alloc_examples(sizeof(polylabel), 1);
  data->ex->indices.push_back(val_namespace);
  for(size_t i=1; i<14; i++)
    data->ex->indices.push_back((unsigned char)i+'A');
  data->ex->indices.push_back(constant_namespace);

  data->old_style_labels = vm.count("old_style_labels") > 0;

  const char* pair[] = {"BC", "BE", "BB", "CC", "DD", "EE", "FF", "GG", "EF", "BH", "BJ", "EL", "dB", "dC", "dD", "dE", "dF", "dG", "dd"};
  const char* triple[] = {"EFG", "BEF", "BCE", "BCD", "BEL", "ELM", "BHI", "BCC", "BJE", "BHE", "BJK", "BEH", "BEN", "BEJ"};
  vector<string> newpairs(pair, pair+19);
  vector<string> newtriples(triple, triple+14);
  all.pairs.swap(newpairs);
  all.triples.swap(newtriples);

  for (v_string* i = all.interactions.begin; i != all.interactions.end; ++i)
    i->delete_v();
  all.interactions.erase();
  for (vector<string>::const_iterator i = all.pairs.begin(); i != all.pairs.end(); ++i)
    all.interactions.push_back(string2v_string(*i));
  for (vector<string>::const_iterator i = all.triples.begin(); i != all.triples.end(); ++i)
    all.interactions.push_back(string2v_string(*i));

  sch.set_options(AUTO_CONDITION_FEATURES | NO_CACHING);
  sch.set_label_parser( COST_SENSITIVE::cs_label, [](polylabel&l) -> bool { return l.cs.costs.size() == 0; });
}

void finish(Search::search& sch) {
  task_data *data = sch.get_task_data<task_data>();
  data->valid_actions.delete_v();
  data->gold_heads.delete_v();
  data->gold_tags.delete_v();
  data->stack.delete_v();
  data->heads.delete_v();
  data->tags.delete_v();
  data->temp.delete_v();
  data->action_loss.delete_v();
  VW::dealloc_example(COST_SENSITIVE::cs_label.delete_label, *data->ex);
  free(data->ex);
  for (size_t i=0; i<6; i++) data->children[i].delete_v();
  delete data;
}

void inline add_feature(example& ex, uint32_t idx, unsigned char ns, size_t mask, uint32_t multiplier, bool audit=false) {
  feature f = {1.0f, (idx * multiplier) & (uint32_t)mask};
  ex.atomics[(int)ns].push_back(f);
  if (audit) {
    audit_data a = { nullptr, nullptr, f.weight_index, 1.f, true };
    ex.audit_features[(int)ns].push_back(a);
  }
}

void add_all_features(example& ex, example& src, unsigned char tgt_ns, size_t mask, uint32_t multiplier, uint32_t offset, bool audit=false) {
  for (unsigned char* ns = src.indices.begin; ns != src.indices.end; ++ns)
    if(*ns != constant_namespace) // ignore constant_namespace
      for (size_t k=0; k<src.atomics[*ns].size(); k++) {
        uint32_t i = src.atomics[*ns][k].weight_index / multiplier;
        feature  f = { 1., ((i + offset) * multiplier) & (uint32_t)mask };
        ex.atomics[tgt_ns].push_back(f);
        if (audit) {
          audit_data a = { nullptr, nullptr, f.weight_index, 1.f, true };
          ex.audit_features[tgt_ns].push_back(a);
        }
      }
}

void inline reset_ex(example *ex, bool audit=false) {
  ex->num_features = 0;
  ex->total_sum_feat_sq = 0;
  for(unsigned char *ns = ex->indices.begin; ns!=ex->indices.end; ns++) {
    ex->sum_feat_sq[(int)*ns] = 0;
    ex->atomics[(int)*ns].erase();
    if (audit) ex->audit_features[(int)*ns].erase();
  }
}

// arc-hybrid System.
uint32_t transition_hybrid(Search::search& sch, uint32_t a_id, uint32_t idx, uint32_t t_id) {
  task_data *data = sch.get_task_data<task_data>();
  v_array<uint32_t> &heads=data->heads, &stack=data->stack, &gold_heads=data->gold_heads, &gold_tags=data->gold_tags, &tags = data->tags;
  v_array<uint32_t> *children = data->children;
  if (a_id == SHIFT) {
    stack.push_back(idx);
    return idx+1;
  } else if (a_id == REDUCE_RIGHT) {
    uint32_t last   = stack.last();
    size_t   hd     = stack[ stack.size() - 2 ];
    heads[last]     = hd;
    children[5][hd] = children[4][hd];
    children[4][hd] = last;
    children[1][hd] ++;
    tags[last]      = t_id;
    sch.loss(gold_heads[last] != heads[last] ? 2 : (gold_tags[last] != t_id) ? 1.f : 0.f);
    assert(! stack.empty());
    stack.pop();
    return idx;
  } else if (a_id == REDUCE_LEFT) {
    uint32_t last    = stack.last();
    heads[last]      = idx;
    children[3][idx] = children[2][idx];
    children[2][idx] = last;
    children[0][idx] ++;
    tags[last]       = t_id;
    sch.loss(gold_heads[last] != heads[last] ? 2 : (gold_tags[last] != t_id) ? 1.f : 0.f);
    assert(! stack.empty());
    stack.pop();
    return idx;
  }
  THROW("transition_hybrid failed");
}

void extract_features(Search::search& sch, uint32_t idx,  vector<example*> &ec) {
  vw& all = sch.get_vw_pointer_unsafe();
  task_data *data = sch.get_task_data<task_data>();
  reset_ex(data->ex);
  size_t mask = sch.get_mask();
  uint32_t multiplier = all.wpp << all.reg.stride_shift;
  v_array<uint32_t> &stack = data->stack, &tags = data->tags, *children = data->children, &temp=data->temp;
  example **ec_buf = data->ec_buf;
  example &ex = *(data->ex);

  size_t n = ec.size();
  bool empty = stack.empty();
  size_t last = empty ? 0 : stack.last();

  for(size_t i=0; i<13; i++)
    ec_buf[i] = nullptr;

  // feature based on the top three examples in stack ec_buf[0]: s1, ec_buf[1]: s2, ec_buf[2]: s3
  for(size_t i=0; i<3; i++)
    ec_buf[i] = (stack.size()>i && *(stack.end-(i+1))!=0) ? ec[*(stack.end-(i+1))-1] : 0;

  // features based on examples in string buffer ec_buf[3]: b1, ec_buf[4]: b2, ec_buf[5]: b3
  for(size_t i=3; i<6; i++)
    ec_buf[i] = (idx+(i-3)-1 < n) ? ec[idx+i-3-1] : 0;

  // features based on the leftmost and the rightmost children of the top element stack ec_buf[6]: sl1, ec_buf[7]: sl2, ec_buf[8]: sr1, ec_buf[9]: sr2;
  for(size_t i=6; i<10; i++)
    if (!empty && last != 0&& children[i-4][last]!=0)
      ec_buf[i] = ec[children[i-4][last]-1];

  // features based on leftmost children of the top element in bufer ec_buf[10]: bl1, ec_buf[11]: bl2
  for(size_t i=10; i<12; i++)
    ec_buf[i] = (idx <=n && children[i-8][idx]!=0) ? ec[children[i-8][idx]-1] : 0;
  ec_buf[12] = (stack.size()>1 && *(stack.end-2)!=0 && children[2][*(stack.end-2)]!=0) ? ec[children[2][*(stack.end-2)]-1] : 0;

  // unigram features
  for(size_t i=0; i<13; i++) {
    uint32_t additional_offset = (uint32_t)(i*offset_const);
    if (!ec_buf[i])
      add_feature(ex, (uint32_t) 438129041 + additional_offset, (unsigned char)((i+1)+'A'), mask, multiplier);
    else
      add_all_features(ex, *ec_buf[i], 'A'+i+1, mask, multiplier, additional_offset, false);
  }

  // Other features
  temp.resize(10,true);
  temp[0] = empty ? 0: (idx >n? 1: 2+min(5, idx - last));
  temp[1] = empty? 1: 1+min(5, children[0][last]);
  temp[2] = empty? 1: 1+min(5, children[1][last]);
  temp[3] = idx>n? 1: 1+min(5 , children[0][idx]);
  for(size_t i=4; i<8; i++)
    temp[i] = (!empty && children[i-2][last]!=0)?tags[children[i-2][last]]:15;
  for(size_t i=8; i<10; i++)
    temp[i] = (idx <=n && children[i-6][idx]!=0)? tags[children[i-6][idx]] : 15;

  uint32_t additional_offset = val_namespace*offset_const;
  for(uint32_t j=0; j< 10; j++) {
    additional_offset += j* 1023;
    add_feature(ex, temp[j]+ additional_offset , val_namespace, mask, multiplier);
  }

  size_t count=0;
  for (unsigned char* ns = data->ex->indices.begin; ns != data->ex->indices.end; ns++) {
    data->ex->sum_feat_sq[(int)*ns] = (float) data->ex->atomics[(int)*ns].size();
    count+= data->ex->atomics[(int)*ns].size();
  }

  size_t new_count;
  float new_weight;
  INTERACTIONS::eval_count_of_generated_ft(all, *data->ex, new_count, new_weight);

  data->ex->num_features = count + new_count;
  data->ex->total_sum_feat_sq = (float) count + new_weight;
}

void get_valid_actions(v_array<uint32_t> & valid_action, uint32_t idx, uint32_t n, uint32_t stack_depth, uint32_t state) {
  valid_action.erase();
  if(idx<=n) // SHIFT
    valid_action.push_back( SHIFT );
  if(stack_depth >=2) // RIGHT
    valid_action.push_back( REDUCE_RIGHT );
  if(stack_depth >=1 && state!=0 && idx<=n) // LEFT
    valid_action.push_back( REDUCE_LEFT );
}

bool is_valid(uint32_t action, v_array<uint32_t> valid_actions) {
  for(size_t i=0; i< valid_actions.size(); i++)
    if(valid_actions[i] == action)
      return true;
  return false;
}

void get_gold_actions(Search::search &sch, uint32_t idx, uint32_t n, v_array<action>& gold_actions) {
  gold_actions.erase();
  task_data *data = sch.get_task_data<task_data>();
  v_array<uint32_t> &action_loss = data->action_loss, &stack = data->stack, &gold_heads=data->gold_heads, &valid_actions=data->valid_actions;
  size_t size = stack.size();
  uint32_t last = (size==0) ? 0 : stack.last();

  if (is_valid(1,valid_actions) &&( stack.empty() || gold_heads[idx] == last)) {
    gold_actions.push_back(1);
    return;
  }

  if (is_valid(3,valid_actions) && gold_heads[last] == idx) {
    gold_actions.push_back(3);
    return;
  }

  for(uint32_t i = 1; i<= 3; i++)
    action_loss[i] = (is_valid(i,valid_actions))?0:100;

  for(uint32_t i = 0; i<size-1; i++)
    if(idx <=n && (gold_heads[stack[i]] == idx || gold_heads[idx] == stack[i]))
      action_loss[1] += 1;
  if(size>0 && gold_heads[last] == idx)
    action_loss[1] += 1;

  for(uint32_t i = idx+1; i<=n; i++)
    if(gold_heads[i] == last|| gold_heads[last] == i)
      action_loss[3] +=1;
  if(size>0  && idx <=n && gold_heads[idx] == last)
    action_loss[3] +=1;
  if(size>=2 && gold_heads[last] == stack[size-2])
    action_loss[3] += 1;

  if(gold_heads[last] >=idx)
    action_loss[2] +=1;
  for(uint32_t i = idx; i<=n; i++)
    if(gold_heads[i] == last)
      action_loss[2] +=1;

  // return the best actions
  size_t best_action = 1;
  size_t count = 0;
  for(size_t i=1; i<=3; i++)
    if(action_loss[i] < action_loss[best_action]) {
      best_action= i;
      count = 1;
      gold_actions.erase();
      gold_actions.push_back(i);
    } else if (action_loss[i] == action_loss[best_action]) {
      count++;
      gold_actions.push_back(i);
    }
}

void setup(Search::search& sch, vector<example*>& ec) {
  task_data *data = sch.get_task_data<task_data>();
  v_array<uint32_t> &gold_heads=data->gold_heads, &heads=data->heads, &gold_tags=data->gold_tags, &tags=data->tags;
  uint32_t n = (uint32_t) ec.size();
  heads.resize(n+1, true);
  tags.resize(n+1, true);
  gold_heads.erase();
  gold_heads.push_back(0);
  gold_tags.erase();
  gold_tags.push_back(0);
  for (size_t i=0; i<n; i++) {
    v_array<COST_SENSITIVE::wclass>& costs = ec[i]->l.cs.costs;
    uint32_t head,tag;
    if (data->old_style_labels) {
      uint32_t label = costs[0].class_index;
      head = (label & 255) -1;
      tag  = label >> 8;
    } else {
      head = (costs.size() == 0) ? 0 : costs[0].class_index;
      tag  = (costs.size() <= 1) ? data->root_label : costs[1].class_index;
    }
    if (tag > data->num_label)
      THROW("invalid label " << tag << " which is > num actions=" << data->num_label);

    gold_heads.push_back(head);
    gold_tags.push_back(tag);
    heads[i+1] = 0;
    tags[i+1] = -1;
  }
  for(size_t i=0; i<6; i++)
    data->children[i].resize(n+1, true);
}

void run(Search::search& sch, vector<example*>& ec) {
  task_data *data = sch.get_task_data<task_data>();
  v_array<uint32_t> &stack=data->stack, &gold_heads=data->gold_heads, &valid_actions=data->valid_actions, &heads=data->heads, &gold_tags=data->gold_tags, &tags=data->tags;
  uint32_t n = (uint32_t) ec.size();

  stack.erase();
  stack.push_back((data->root_label==0)?0:1);
  for(size_t i=0; i<6; i++)
    for(size_t j=0; j<n+1; j++)
      data->children[i][j] = 0;

  v_array<action> gold_actions = v_init<action>();
  int count=1;
  uint32_t idx = ((data->root_label==0)?1:2);
  while(stack.size()>1 || idx <= n) {
    if(sch.predictNeedsExample())
      extract_features(sch, idx, ec);

    get_valid_actions(valid_actions, idx, n, (uint32_t) stack.size(), stack.empty() ? 0 : stack.last());
    get_gold_actions(sch, idx, n, gold_actions);

    // Predict the next action {SHIFT, REDUCE_LEFT, REDUCE_RIGHT}
    uint32_t a_id= Search::predictor(sch, (ptag) count)
                   .set_input(*(data->ex))
                   .set_oracle(gold_actions)
                   .set_allowed(valid_actions)
                   .set_condition_range(count-1, sch.get_history_length(), 'p')
                   .set_learner_id(0)
                   .predict();
    count++;

    uint32_t t_id = 0; // gold_tags[stack.last()]; // 0;
    if (a_id != SHIFT) {
      uint32_t gold_label = gold_tags[stack.last()];
      t_id = Search::predictor(sch, (ptag) count)
             .set_input(*(data->ex))
             .set_oracle(gold_label)
             .set_condition_range(count-1, sch.get_history_length(), 'p')
             .set_learner_id(a_id-1)
             .predict();
    }
    count++;
    idx = transition_hybrid(sch, a_id, idx, t_id);
  }

  heads[stack.last()] = 0;
  tags[stack.last()] = (uint32_t)data->root_label;
  sch.loss((gold_heads[stack.last()] != heads[stack.last()]));
  if (sch.output().good())
    for(size_t i=1; i<=n; i++)
      sch.output() << (heads[i])<<":"<<tags[i] << endl;
}
}
