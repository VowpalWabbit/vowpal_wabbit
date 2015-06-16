/*
  Copyright (c) by respective owners including Yahoo!, Microsoft, and
  individual contributors. All rights reserved.  Released under a BSD (revised)
  license as described in the file LICENSE.
*/
#include "search_dep_parser.h"
#include "gd.h"
#include "cost_sensitive.h"

#define val_namespace 100 // valency and distance feature space
#define offset_const 344429

namespace DepParserTask         {  Search::search_task task = { "dep_parser", run, initialize, finish, setup, nullptr};  }

struct task_data {
  example *ex;
  size_t root_label, num_label;
  v_array<uint32_t> valid_actions, valid_labels, action_loss, gold_heads, gold_tags, stack, heads, tags, temp;
  v_array<uint32_t> children[6]; // [0]:num_left_arcs, [1]:num_right_arcs; [2]: leftmost_arc, [3]: second_leftmost_arc, [4]:rightmost_arc, [5]: second_rightmost_arc
  example * ec_buf[13];
};

namespace DepParserTask {
  using namespace Search;

  void initialize(Search::search& srn, size_t& /*num_actions*/, po::variables_map& vm) {
    task_data *data = new task_data();
    data->action_loss.resize(4,true);
    data->ex = NULL;
    srn.set_num_learners(3);
    srn.set_task_data<task_data>(data);
    po::options_description dparser_opts("dependency parser options");
    dparser_opts.add_options()
        ("root_label", po::value<size_t>(&(data->root_label))->default_value(8), "Ensure that there is only one root in each sentence")
        ("num_label", po::value<size_t>(&(data->num_label))->default_value(12), "Number of arc labels");
    srn.add_program_options(vm, dparser_opts);

    for(size_t i=1; i<=data->num_label;i++)
      if(i!=data->root_label)
        data->valid_labels.push_back(i);

    data->ex = alloc_examples(sizeof(polylabel), 1);
    data->ex->indices.push_back(val_namespace);
    for(size_t i=1; i<14; i++)
      data->ex->indices.push_back((unsigned char)i+'A');
    data->ex->indices.push_back(constant_namespace);

    vw& all = srn.get_vw_pointer_unsafe();
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
    
    srn.set_options(AUTO_CONDITION_FEATURES | NO_CACHING);
    srn.set_label_parser( COST_SENSITIVE::cs_label, [](polylabel&l) -> bool { return l.cs.costs.size() == 0; });
  }

  void finish(Search::search& srn) {
    task_data *data = srn.get_task_data<task_data>();
    data->valid_actions.delete_v();
    data->valid_labels.delete_v();
    data->gold_heads.delete_v();
    data->gold_tags.delete_v();
    data->stack.delete_v();
    data->heads.delete_v();
    data->tags.delete_v();
    data->temp.delete_v();
    data->action_loss.delete_v();
    dealloc_example(COST_SENSITIVE::cs_label.delete_label, *data->ex);
    free(data->ex);
    for (size_t i=0; i<6; i++) data->children[i].delete_v();
    delete data;
  } 

  void inline add_feature(example *ex,  uint32_t idx, unsigned  char ns, size_t mask, uint32_t multiplier){
    feature f = {1.0f, (idx * multiplier) & (uint32_t)mask};
    ex->atomics[(int)ns].push_back(f);
  }

  void inline reset_ex(example *ex){
    ex->num_features = 0;
    ex->total_sum_feat_sq = 0;
    for(unsigned char *ns = ex->indices.begin; ns!=ex->indices.end; ns++){
      ex->sum_feat_sq[(int)*ns] = 0;
      ex->atomics[(int)*ns].erase();
    }
  }

  // arc-hybrid System.
  uint32_t transition_hybrid(Search::search& srn, uint32_t a_id, uint32_t idx, uint32_t t_id) {
    task_data *data = srn.get_task_data<task_data>();
    v_array<uint32_t> &heads=data->heads, &stack=data->stack, &gold_heads=data->gold_heads, &gold_tags=data->gold_tags, &tags = data->tags;
    v_array<uint32_t> *children = data->children;
    switch(a_id) {
      case 1:  //SHIFT
        stack.push_back(idx);
        return idx+1;
      case 2:  //RIGHT
        heads[stack.last()] = stack[stack.size()-2];
        children[5][stack[stack.size()-2]]=children[4][stack[stack.size()-2]];
        children[4][stack[stack.size()-2]]=stack.last();
        children[1][stack[stack.size()-2]]++;
        tags[stack.last()] = t_id;
        srn.loss(gold_heads[stack.last()] != heads[stack.last()]?2:(gold_tags[stack.last()] != t_id)? 1.f : 0.f);
        assert(! stack.empty());
        stack.pop();
        return idx;
      case 3:  //LEFT
        heads[stack.last()] = idx;
        children[3][idx]=children[2][idx];
        children[2][idx]=stack.last();
        children[0][idx]++;
        tags[stack.last()] = t_id;
        srn.loss(gold_heads[stack.last()] != heads[stack.last()]?2:(gold_tags[stack.last()] != t_id)? 1.f : 0.f);
        assert(! stack.empty());
        stack.pop();
        return idx;
    }
    return idx;
  }
  
  void extract_features(Search::search& srn, uint32_t idx,  vector<example*> &ec) {
    vw& all = srn.get_vw_pointer_unsafe();
    task_data *data = srn.get_task_data<task_data>();
    reset_ex(data->ex);
    size_t mask = srn.get_mask();
    uint32_t multiplier = all.wpp << all.reg.stride_shift;
    v_array<uint32_t> &stack = data->stack, &tags = data->tags, *children = data->children, &temp=data->temp;
    example **ec_buf = data->ec_buf;
    example &ex = *(data->ex);

    add_feature(&ex, (uint32_t) constant, constant_namespace, mask, multiplier);
    size_t n = ec.size();

    for(size_t i=0; i<13; i++)
      ec_buf[i] = nullptr;

    // feature based on the top three examples in stack ec_buf[0]: s1, ec_buf[1]: s2, ec_buf[2]: s3
    for(size_t i=0; i<3; i++)
      ec_buf[i] = (stack.size()>i && *(stack.end-(i+1))!=0) ? ec[*(stack.end-(i+1))-1] : 0;

    // features based on examples in string buffer ec_buf[3]: b1, ec_buf[4]: b2, ec_buf[5]: b3
    for(size_t i=3; i<6; i++)
      ec_buf[i] = (idx+(i-3)-1 < n) ? ec[idx+i-3-1] : 0;

    // features based on the leftmost and the rightmost children of the top element stack ec_buf[6]: sl1, ec_buf[7]: sl2, ec_buf[8]: sr1, ec_buf[9]: sr2;
    for(size_t i=6; i<10; i++) {
      /*      if (stack.empty()) continue;
              cerr << "stack.last() = " << stack.last() << endl;
              cerr << "children[i-4] = " << &children[i-4] << endl;
              cerr << "children[i-4][s.l] = " << children[i-4][stack.last()] << endl; */
      if (!stack.empty() && stack.last() != 0&& children[i-4][stack.last()]!=0)
        ec_buf[i] = ec[children[i-4][stack.last()]-1];
    }

    // features based on leftmost children of the top element in bufer ec_buf[10]: bl1, ec_buf[11]: bl2
    for(size_t i=10; i<12; i++)
      ec_buf[i] = (idx <=n && children[i-8][idx]!=0)? ec[children[i-8][idx]-1] : 0;
    ec_buf[12] = (stack.size()>1 && *(stack.end-2)!=0 && children[2][*(stack.end-2)]!=0)? ec[children[2][*(stack.end-2)]-1]:0;

    // unigram features
    uint64_t v0;
    for(size_t i=0; i<13; i++) {
      for (unsigned char* fs = ec[0]->indices.begin; fs != ec[0]->indices.end; fs++) {
        if(*fs == constant_namespace) // ignore constant_namespace
          continue;

        uint32_t additional_offset = (uint32_t)(i*offset_const);
        if(!ec_buf[i]){
          for(size_t k=0; k<ec[0]->atomics[*fs].size(); k++) {
            v0 = affix_constant*((*fs+1)*quadratic_constant + k);
            add_feature(&ex, (uint32_t) v0 + additional_offset, (unsigned char)((i+1)+'A'), mask, multiplier);
          }
        }
        else {
          for(size_t k=0; k<ec_buf[i]->atomics[*fs].size(); k++) {
            v0 = (ec_buf[i]->atomics[*fs][k].weight_index / multiplier);
            add_feature(&ex, (uint32_t) v0 + additional_offset, (unsigned char)((i+1)+'A'), mask, multiplier);
          }
        }
      }
    }

    // Other features
    temp.resize(10,true);
    temp[0] = stack.empty()? 0: (idx >n? 1: 2+min(5, idx - stack.last()));
    temp[1] = stack.empty()? 1: 1+min(5, children[0][stack.last()]);
    temp[2] = stack.empty()? 1: 1+min(5, children[1][stack.last()]);
    temp[3] = idx>n? 1: 1+min(5 , children[0][idx]);
    for(size_t i=4; i<8; i++)
      temp[i] = (!stack.empty() && children[i-2][stack.last()]!=0)?tags[children[i-2][stack.last()]]:15;
    for(size_t i=8; i<10; i++)
      temp[i] = (idx <=n && children[i-6][idx]!=0)? tags[children[i-6][idx]] : 15;	

    size_t additional_offset = val_namespace*offset_const; 
    for(int j=0; j< 10;j++) {
      additional_offset += j* 1023;
      add_feature(&ex, temp[j]+ additional_offset , val_namespace, mask, multiplier);
    }

    size_t count=0;
    for (unsigned char* ns = data->ex->indices.begin; ns != data->ex->indices.end; ns++) {
      data->ex->sum_feat_sq[(int)*ns] = (float) data->ex->atomics[(int)*ns].size();
      count+= data->ex->atomics[(int)*ns].size();
    }
    for (vector<string>::iterator i = all.pairs.begin(); i != all.pairs.end();i++)
      count += data->ex->atomics[(int)(*i)[0]].size()* data->ex->atomics[(int)(*i)[1]].size();	
    for (vector<string>::iterator i = all.triples.begin(); i != all.triples.end();i++)
      count += data->ex->atomics[(int)(*i)[0]].size()*data->ex->atomics[(int)(*i)[1]].size()*data->ex->atomics[(int)(*i)[2]].size();	
    data->ex->num_features = count;
    data->ex->total_sum_feat_sq = (float) count;
  }

  void get_valid_actions(v_array<uint32_t> & valid_action, uint32_t idx, uint32_t n, uint32_t stack_depth, uint32_t state) {
    valid_action.erase();
    if(idx<=n) // SHIFT
      valid_action.push_back(1);
    if(stack_depth >=2) // RIGHT
      valid_action.push_back(2);	
    if(stack_depth >=1 && state!=0 && idx<=n) // LEFT
      valid_action.push_back(3);
  }

  bool is_valid(uint32_t action, v_array<uint32_t> valid_actions) {
    for(size_t i=0; i< valid_actions.size(); i++)
      if(valid_actions[i] == action)
        return true;
    return false;
  }

  size_t get_gold_actions(Search::search &srn, uint32_t idx, uint32_t n){
    task_data *data = srn.get_task_data<task_data>();
    v_array<uint32_t> &action_loss = data->action_loss, &stack = data->stack, &gold_heads=data->gold_heads, &valid_actions=data->valid_actions;

    if (is_valid(1,valid_actions) &&( stack.empty() || gold_heads[idx] == stack.last()))
      return 1;
    
    if (is_valid(3,valid_actions) && gold_heads[stack.last()] == idx)
      return 3;

    for(size_t i = 1; i<= 3; i++)
      action_loss[i] = (is_valid(i,valid_actions))?0:100;

    for(uint32_t i = 0; i<stack.size()-1; i++)
      if(idx <=n && (gold_heads[stack[i]] == idx || gold_heads[idx] == stack[i]))
        action_loss[1] += 1;
    if(stack.size()>0 && gold_heads[stack.last()] == idx)
      action_loss[1] += 1;

    for(uint32_t i = idx+1; i<=n; i++)
      if(gold_heads[i] == stack.last()|| gold_heads[stack.last()] == i)
        action_loss[3] +=1;
    if(stack.size()>0  && idx <=n && gold_heads[idx] == stack.last())
      action_loss[3] +=1;
    if(stack.size()>=2 && gold_heads[stack.last()] == stack[stack.size()-2])
      action_loss[3] += 1;

    if(gold_heads[stack.last()] >=idx)
      action_loss[2] +=1;
    for(uint32_t i = idx; i<=n; i++)
      if(gold_heads[i] == stack.last())
        action_loss[2] +=1;

    // return the best action
    size_t best_action = 1;
    for(size_t i=1; i<=3; i++)
      if(action_loss[i] <= action_loss[best_action])
        best_action= i;
    return best_action;
  }

  void setup(Search::search& srn, vector<example*>& ec) {
    task_data *data = srn.get_task_data<task_data>();
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
      uint32_t head = (costs.size() == 0) ? 0 : costs[0].class_index;
      uint32_t tag  = (costs.size() <= 1) ? data->root_label : costs[1].class_index;
      if (tag > data->num_label) {
        cerr << "invalid label " << tag << " which is > num actions=" << data->num_label << endl;
        throw exception();
      }
      gold_heads.push_back(head);
      gold_tags.push_back(tag);
      heads[i+1] = 0;
      tags[i+1] = -1;
    }
    /*
    for(size_t i=0; i<n; i++) {
      uint32_t label = ec[i]->l.multi.label;
      gold_heads.push_back((label & 255) -1);
      if (label >> 8 > data->num_label) {
        cerr << "invalid y=" << label << " implies a label of " << (label >> 8) << " which is > num actions=" << data->num_label << endl;
        throw exception();
      }
      gold_tags.push_back(label >>8);
      heads[i+1] = 0;
      tags[i+1] = -1;
    }
    */
    for(size_t i=0; i<6; i++)
      data->children[i].resize(n+1, true);
  }    
  
  void run(Search::search& srn, vector<example*>& ec) {
    task_data *data = srn.get_task_data<task_data>();
    v_array<uint32_t> &stack=data->stack, &gold_heads=data->gold_heads, &valid_actions=data->valid_actions, &heads=data->heads, &gold_tags=data->gold_tags, &tags=data->tags, &valid_labels=data->valid_labels;
    uint32_t n = (uint32_t) ec.size();

    stack.erase();
    stack.push_back((data->root_label==0)?0:1);
    for(size_t i=0; i<6; i++)
      for(size_t j=0; j<n+1; j++)
        data->children[i][j] = 0;
    
    int count=1;
    uint32_t idx = ((data->root_label==0)?1:2);
    while(stack.size()>1 || idx <= n){
      if(srn.predictNeedsExample())
        extract_features(srn, idx, ec);
      get_valid_actions(valid_actions, idx, n, (uint32_t) stack.size(), stack.size()>0?stack.last():0);
      uint32_t gold_action = get_gold_actions(srn, idx, n);

      // Predict the next action {SHIFT, REDUCE_LEFT, REDUCE_RIGHT}
      //cerr << "----------------------" << endl << "valid = ["; for (size_t ii=0; ii<valid_actions.size(); ii++) cerr << ' ' << valid_actions[ii]; cerr << "], gold=" << gold_action << endl;
      count = 2*idx + 1;
      uint32_t a_id= Search::predictor(srn, (ptag) count).set_input(*(data->ex)).set_oracle(gold_action).set_allowed(valid_actions).set_condition_range(count-1, srn.get_history_length(), 'p').set_learner_id(0).predict();
      count++;
      //cerr << "a_id=" << a_id << endl;
      
      uint32_t t_id = 0;
      if(a_id ==2 || a_id == 3){
        uint32_t gold_label = gold_tags[stack.last()];
        t_id= Search::predictor(srn, (ptag) count).set_input(*(data->ex)).set_oracle(gold_label).set_allowed(valid_labels).set_condition_range(count-1, srn.get_history_length(), 'p').set_learner_id(a_id-1).predict();
      }
      count++;
      idx = transition_hybrid(srn, a_id, idx, t_id);
    }

    heads[stack.last()] = 0;
    tags[stack.last()] = data->root_label;
    srn.loss((gold_heads[stack.last()] != heads[stack.last()]));
    if (srn.output().good())
      for(size_t i=1; i<=n; i++)
        srn.output() << (heads[i])<<":"<<tags[i] << endl;
  }
}
