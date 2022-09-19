// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/eigen_memory_tree.h"

#include "vw/common/future_compat.h"
#include "vw/config/options.h"
#include "vw/core/array_parameters.h"
#include "vw/core/example.h"
#include "vw/core/numeric_casts.h"
#include "vw/core/rand48.h"
#include "vw/core/rand_state.h"
#include "vw/core/setup_base.h"
#include "vw/core/v_array.h"
#include "vw/core/vw.h"
#include "vw/io/logger.h"

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <ctime>
#include <memory>
#include <sstream>

using namespace VW::LEARNER;
using namespace VW::config;

// TODO: This file has several cout print statements. It looks like
//       they should be using trace_message, but its difficult to tell
namespace
{
///////////////////////Helper//////////////////////////////
//////////////////////////////////////////////////////////
float median(std::vector<float>& array)
{
  int size = array.size();
  std::sort(array.begin(), array.end());

  if (size % 2 == 0) { return (array[size / 2 - 1] + array[size / 2]) / 2; }
  else { return array[size / 2]; }
}

float projection(VW::flat_example& ec, sparse_parameters& projector)
{
  float projection = 0;
  
  for (size_t idx1 = 0; idx1 < ec.fs.size(); idx1++)
  {
    projection += projector[ec.fs.indices[idx1]] * ec.fs.values[idx1];
  }

  return projection;
}
////////////////////////////end of helper/////////////////////////
//////////////////////////////////////////////////////////////////

////////////////////////eigen_memory_tree///////////////////
////////////////////////////////////////////////////////////

struct tree_example
{
  VW::flat_example* base;
  VW::flat_example* full;

  uint32_t label;

  tree_example() {
    base = new flat_example();
    full = new flat_example();
    label = 0;
  }

  tree_example(VW::workspace& all, example* ex)
  {
    label = ex->l.multi.label;

    auto full_interactions = ex->interactions;
    auto base_interactions = new std::vector<std::vector<namespace_index>>();

    ex->interactions = base_interactions;
    base = VW::flatten_sort_example(all, ex);

    ex->interactions = full_interactions;
    full = VW::flatten_sort_example(all, ex);
  }
};

struct LRU
{
  typedef tree_example* K;
  typedef std::list<K>::iterator V;

  std::list<K> list;
  std::unordered_map<K, V> map;

  int max_size;

public:
  LRU(int max_size) { (*this).max_size = max_size; }

  K bound(K item)
  {
    if (max_size == -1) { return nullptr; }

    auto item_map_reference = map.find(item);

    // item is not in map so we add it to our list
    if (item_map_reference == map.end())
    {
      list.push_front(item);
      map.emplace(std::pair<K, V>(item, list.begin()));
    }
    // item is in map already so we move it to the front of the line
    else
    {
      auto item_list_reference = (*item_map_reference).second;
      if (item_list_reference != list.begin())
      {
        list.splice(list.begin(), list, item_list_reference, std::next(item_list_reference));
      }
    }

    if (list.size() > max_size)
    {
      K last_value = list.back();
      list.pop_back();
      map.erase(last_value);
      return last_value;
    }
    else { return nullptr; }
  }
};

struct node
{
  node* parent;  // parent node
  bool internal; // (internal or leaf)
  uint32_t depth;// depth.
  node* left;    // left child.
  node* right;   // right child.

  std::vector<tree_example*> examples;

  double router_decision;
  sparse_parameters* router_weights = nullptr;

  node()  // construct:
  {
    parent = nullptr;
    left = nullptr;
    right = nullptr;

    internal = false;
    depth = 0;

    router_decision = 0;
    router_weights = nullptr;
  }
};

struct tree
{
  VW::workspace* all = nullptr;
  std::shared_ptr<VW::rand_state> _random_state;

  int iter; // how many times we've 'learned'
  int depth;// how deep the tree is
  int pass; // what pass we are on for the data
  bool test_only; //indicates that learning should not occur

  int32_t tree_bound;  // how many memories before bounding the tree
  int32_t leaf_split;  // how many memories before splitting a leaf node
  int32_t scorer_type; // 1: random, 2: distance, 3: rank

  example* scorer_ex = nullptr; //we create one of these which we re-use so we don't have to reallocate examples

  clock_t begin; // for timing performance
  float time;  // for timing performance

  node* root;
  LRU* bounder;

  tree()
  {
    iter = 0;
    pass = 0;
    depth = 0;
    test_only = false;

    tree_bound = -1;
    leaf_split = 100;
    scorer_type = 3;

    begin = clock();
    time = 0;

    root = nullptr;
    bounder = nullptr;
  }

  void deallocate_node(node* n) {
    if (!n) { return; }
    for (auto e : n->examples)
    {
      VW::free_flatten_example(e->base);
      VW::free_flatten_example(e->full);
    }
    deallocate_node(n->left);
    deallocate_node(n->right);
  }

  ~tree()
  {
    deallocate_node(root);
    if (scorer_ex) { VW::dealloc_examples(scorer_ex, 1); }
  }
};

node* node_route(tree& b, single_learner& base, node& cn, tree_example& ec)
{
  return projection(*ec.base, *cn.router_weights) < cn.router_decision ? cn.left : cn.right;
}

void tree_init(tree& b)
{
  b.iter = 0;
  b.depth= 0;
  b.pass = 0;

  b.root = new node();

  b.bounder = new LRU(b.tree_bound);

  //we set this up for repeated use later in the scorer.
  //we will populate this examples features over and over.
  b.scorer_ex = ::VW::alloc_examples(1);
  b.scorer_ex->interactions = new std::vector<std::vector<VW::namespace_index>>();
  b.scorer_ex->extent_interactions = new std::vector<std::vector<extent_term>>();
  b.scorer_ex->indices.push_back(0);
  b.scorer_ex->num_features++;
}

node& tree_route(tree& b, single_learner& base, tree_example& ec)
{
  node* cn = b.root;
  while (cn->internal) { cn = node_route(b, base, *cn, ec); }
  return *cn;
}

void tree_bound(tree& b, single_learner& base, tree_example* ec)
{
  auto to_delete = b.bounder->bound(ec);

  if (to_delete == nullptr) { return; }

  node& cn = tree_route(b, base, *to_delete);

  for (auto iter = cn.examples.begin(); iter != cn.examples.end(); iter++)
  {
    if (*iter == to_delete)
    {
      cn.examples.erase(iter);
      return;
    }
  }
}

float scorer_initial(example& ex) { return 1 - exp(-sqrt(ex.total_sum_feat_sq)); }

void scorer_features(features& f1, features& f2, features& out_f)
{
  out_f.values.clear_noshrink();
  out_f.indices.clear_noshrink();
  out_f.sum_feat_sq = 0;

  size_t idx1 = 0;
  size_t idx2 = 0;

  uint64_t index = 0;
  uint64_t f1_idx = 0;
  uint64_t f2_idx = 0;

  float f1_val = 0;
  float f2_val = 0;

  while (idx1 < f1.size() || idx2 < f2.size())
  {
    f1_idx = (idx1 == f1.size()) ? INT_MAX : f1.indices[idx1];
    f2_idx = (idx2 == f2.size()) ? INT_MAX : f2.indices[idx2];

    f1_val = 0;
    f2_val = 0;

    if (f1_idx <= f2_idx)
    {
      index = f1_idx;
      f1_val = f1.values[idx1];
      idx1++;
    }

    if (f2_idx <= f1_idx)
    {
      index = f2_idx;
      f2_val = f2.values[idx2];
      idx2++;
    }

    if (f1_val != f2_val)
    {
      float value = abs(f1_val - f2_val);
      out_f.values.push_back(value);
      out_f.indices.push_back(index);
      out_f.sum_feat_sq += value * value;
    }
  }
}

void scorer_example(tree_example& ec1, tree_example& ec2, example& out_example)
{
  scorer_features(ec1.full->fs, ec2.full->fs, out_example.feature_space[0]);
  out_example.total_sum_feat_sq = out_example.feature_space[0].sum_feat_sq;
}

float scorer_predict(tree& b, single_learner& base, tree_example& pred_ec, tree_example& leaf_ec)
{
  if (b.scorer_type == 1)  // random scorer
  {
    return -b._random_state->get_and_update_random();
  }

  if (b.scorer_type == 2)  // dist scorer
  {
    scorer_example(pred_ec, leaf_ec, *b.scorer_ex);
    return -b.scorer_ex->total_sum_feat_sq;
  }

  if (b.scorer_type == 3)  // rank scorer
  {
    scorer_example(pred_ec, leaf_ec, *b.scorer_ex);

    if (b.scorer_ex->total_sum_feat_sq == 0) { return 0; }

    b.scorer_ex->l.simple.label = FLT_MAX;
    b.scorer_ex->_reduction_features.template get<simple_label_reduction_features>().initial = scorer_initial(*b.scorer_ex);

    base.predict(*b.scorer_ex);

    return -b.scorer_ex->pred.scalar;
  }
}

void scorer_learn(tree& b, single_learner& base, node& cn, tree_example& ec, float weight)
{
  // random and dist scorer has nothing to learsn
  if (b.scorer_type == 1 || b.scorer_type == 2) { return; }

  if (b.scorer_type == 3)  // rank scorer should learn
  {
    if (weight == 0) { return; }

    if (cn.examples.size() < 2) { return; }

    // shuffle the examples to break ties randomly
    std::random_shuffle(cn.examples.begin(), cn.examples.end());

    float score_value_1 = -FLT_MAX;
    float score_reward_1 = -FLT_MAX;
    tree_example* score_ptr_1 = nullptr;

    float reward_value_1 = -FLT_MAX;
    tree_example* reward_ptr_1 = nullptr;

    float reward_value_2 = -FLT_MAX;
    tree_example* reward_ptr_2 = nullptr;

    for (auto example: cn.examples)
    {
      auto score = scorer_predict(b, base, ec, *example);
      auto reward = (example->label == ec.label) ? 1.f : 0.f;

      if (score > score_value_1)
      {
        score_value_1 = score;
        score_ptr_1 = example;
        score_reward_1 = reward;
      }
      if (reward > reward_value_1)
      {
        reward_value_2 = reward_value_1;
        reward_ptr_2 = reward_ptr_1;

        reward_value_1 = reward;
        reward_ptr_1 = example;
      }
      else if (reward > reward_value_2)
      {
        reward_value_2 = reward;
        reward_ptr_2 = example;
      }
    }

    // get the index/reward for the example with the best score
    auto preferred_index = score_ptr_1;
    auto preferred_reward = score_reward_1;

    // get the index/reward for the best example without the best score
    auto alternative_index = (reward_ptr_1 == score_ptr_1) ? reward_ptr_2 : reward_ptr_1;
    auto alternative_reward = (reward_ptr_1 == score_ptr_1) ? reward_value_2 : reward_value_1;

    // the better of the two options moves towards 0 while the other moves towards -1
    weight *= abs(preferred_reward - alternative_reward);

    if (weight == 0) { return; }

    auto preferred_label = preferred_reward > alternative_reward ? 0.f : 1.f;
    auto alternative_label = alternative_reward > preferred_reward ? 0.f : 1.f;

    if (b._random_state->get_and_update_random() > .5)
    {
      scorer_example(ec, *preferred_index, *b.scorer_ex);
      b.scorer_ex->pred.scalar = 0;
      b.scorer_ex->l.simple = {preferred_label};
      b.scorer_ex->weight = weight;
      b.scorer_ex->_reduction_features.template get<simple_label_reduction_features>().initial = scorer_initial(*b.scorer_ex);
      base.learn(*b.scorer_ex);
      
      scorer_example(ec, *alternative_index, *b.scorer_ex);
      b.scorer_ex->pred.scalar = 0;
      b.scorer_ex->l.simple = {alternative_label};
      b.scorer_ex->weight = weight;
      b.scorer_ex->_reduction_features.template get<simple_label_reduction_features>().initial = scorer_initial(*b.scorer_ex);
      base.learn(*b.scorer_ex);
    }
    else
    {
      scorer_example(ec, *alternative_index, *b.scorer_ex);
      b.scorer_ex->l.simple = {alternative_label};
      b.scorer_ex->weight = weight;
      b.scorer_ex->_reduction_features.template get<simple_label_reduction_features>().initial = scorer_initial(*b.scorer_ex);
      base.learn(*b.scorer_ex);

      scorer_example(ec, *preferred_index, *b.scorer_ex);
      b.scorer_ex->l.simple = {preferred_label};
      b.scorer_ex->weight = weight;
      b.scorer_ex->_reduction_features.template get<simple_label_reduction_features>().initial = scorer_initial(*b.scorer_ex);
      base.learn(*b.scorer_ex);
    }
  }
}

void node_split(tree& b, single_learner& base, node& cn)
{
  VW::rand_state rand_state = *b._random_state;
  std::vector<float> projs;

  uint64_t bits = static_cast<uint64_t>(1) << (b.all->num_bits);

  float best_variance = 0;
  float best_decision = 0;

  sparse_parameters* best_weights = new sparse_parameters(bits);
  sparse_parameters* new_weights = new sparse_parameters(bits);

  (*new_weights).set_default([&rand_state](weight* weights, uint64_t) { weights[0] = rand_state.get_and_update_gaussian(); });
  (*best_weights).set_default([&rand_state](weight* weights, uint64_t) { weights[0] = rand_state.get_and_update_gaussian(); });

  for (size_t perms = 0; perms < 90; perms++)
  {
    (*new_weights).set_zero(0);

    projs.clear();

    for (auto example: cn.examples)
    {
      projs.push_back(projection(*example->base, *new_weights));
    }

    float E = std::accumulate(projs.begin(), projs.end(), 0.0) / projs.size();
    float E_2 = std::inner_product(projs.begin(), projs.end(), projs.begin(), 0.0) / projs.size();
    float new_variance = E_2 - E * E;

    if (new_variance > best_variance)
    {
      best_variance = new_variance;
      std::swap(new_weights, best_weights);
      best_decision = median(projs);
    }
  }

  if (best_variance == 0)
  {
    std::cout << "warning: all examples in a leaf have identical features" << std::endl;
    delete new_weights;
    delete best_weights;
  }
  else
  {
    delete new_weights;

    auto left  = new node();
    auto right = new node();
    auto depth = cn.depth+1; 

    cn.internal = true;
    cn.left = left;
    cn.right = right;

    left->depth = depth;
    right->depth = depth;
    left->parent = &cn;
    right->parent = &cn;

    if (depth > b.depth)
    {
      b.depth = depth;
    }

    (*best_weights).set_default(nullptr);
    cn.router_weights = best_weights;
    cn.router_decision = best_decision;

    for (auto example : cn.examples)
    {
      node_route(b, base, cn, *example)->examples.push_back(example);
    }
    cn.examples.clear();
  }
}

void node_insert(tree& b, single_learner& base, node& cn, tree_example& ec)
{
  for (auto example : cn.examples)
  {
    scorer_example(ec, *example, *b.scorer_ex);
    if (b.scorer_ex->total_sum_feat_sq == 0) { return; }
  }

  cn.examples.push_back(&ec);

  if (cn.examples.size() >= b.leaf_split) { node_split(b, base, cn); }
}

tree_example* node_pick(tree& b, single_learner& base, node& cn, tree_example& ec)
{
  if (cn.examples.size() == 0) { return nullptr; }

  float best_score = -FLT_MAX;
  std::vector<tree_example*> best_examples;

  for (auto example: cn.examples)
  {
    float score = scorer_predict(b, base, ec, *example);

    if (score == best_score) {
      best_examples.push_back(example);
    }
    else if (score > best_score)
    {
      best_score = score;
      best_examples.clear();
      best_examples.push_back(example);
    }
  }

  std::random_shuffle(best_examples.begin(), best_examples.end());
  return best_examples.front();
}

void predict(tree& b, single_learner& base, example& ec)
{
  tree_example ex(*b.all, &ec);

  node& cn = tree_route(b, base, ex);
  auto closest_ec = node_pick(b, base, cn, ex);

  ec.pred.multiclass = (closest_ec != nullptr) ? closest_ec->label : 0;
  ec.loss = (ec.l.multi.label != ec.pred.multiclass) ? ec.weight : 0;

  tree_bound(b, base, closest_ec);
}

void learn(tree& b, single_learner& base, example& ec)
{
  tree_example& ex = *new tree_example(*b.all, &ec);

  b.iter++;

  if (b.test_only) { return; }

  node& cn = tree_route(b, base, ex);
  scorer_learn(b, base, cn, ex, ec.weight);

  if (b.pass == 0) { node_insert(b, base, cn, ex); }

  tree_bound(b, base, &ex);
}

void end_pass(tree& b)
{
  std::cout << "##### pass_time: " << static_cast<float>(clock() - b.begin) / CLOCKS_PER_SEC << std::endl;

  b.time = 0;
  b.begin = clock();

  b.pass++;
}
/////////////////end of eigen_memory_tree///////////////////
////////////////////////////////////////////////////////////

///////////////////Save & Load//////////////////////////////////////
////////////////////////////////////////////////////////////////////
void save_load_examples(tree& b, node& n, io_buf& model_file, bool& read, bool& text, std::stringstream& msg)
{
  writeitvar(n.examples.size(), "n_examples", n_examples);

  auto parser = b.all->example_parser->lbl_parser;
  auto mask = b.all->parse_mask;

  if (read) { for (uint32_t i = 0; i < n_examples; i++) { n.examples.push_back(new tree_example()); } }

  for (auto e : n.examples) {
    if (read) {
      writeit(e->label, "example_label")
      VW::model_utils::read_model_field(model_file, *e->base, parser);
      VW::model_utils::read_model_field(model_file, *e->full, parser);
    }
    else {
      writeit(e->label, "example_label");
      VW::model_utils::write_model_field(model_file, *e->base, "_memory_base", false, parser, mask);
      VW::model_utils::write_model_field(model_file, *e->full, "_memory_full", false, parser, mask);
    }
  }
}

void save_load_weights(tree& b, node& n, io_buf& model_file, bool& read, bool& text, std::stringstream& msg)
{
  if (!n.internal) { return; }

  uint32_t router_dims = 0;
  size_t router_length = 0;
  uint32_t router_shift = 0;

  if (n.router_weights != nullptr)
  {
    for (sparse_parameters::iterator i = n.router_weights->begin(); i != n.router_weights->end(); ++i)
    {
      if (*i != 0) { router_dims++; }
    }
    router_shift = n.router_weights->stride_shift();
    router_length = (n.router_weights->mask() + 1) >> router_shift;
  }

  writeit(n.router_decision, "router_decision");
  writeit(router_dims, "router_dims");
  writeit(router_length, "router_length");
  writeit(router_shift, "router_shift");

  if (read)
  {
    n.router_weights = new sparse_parameters(router_length, router_shift);
    for (int i = 0; i < router_dims; i++)
    {
      uint64_t index = 0;
      float value = 0;
      writeit(index, "router_index");
      writeit(value, "router_value");

      (*n.router_weights)[index] = value;
    }
  }
  else
  {
    for (sparse_parameters::iterator i = n.router_weights->begin(); i != n.router_weights->end(); ++i)
    {
      uint64_t index = i.index();
      float value = (*i);
      if (value != 0)
      {
        writeit(index, "router_index");
        writeit(value, "router_value");
      }
    }
  }
}

node* save_load_node(tree& b, node* n, io_buf& model_file, bool& read, bool& text, std::stringstream& msg) {

  writeitvar(!read && !n, "is_null", is_null);
  if (is_null) { return nullptr; }

  if (!n) { n = new node(); }

  writeit(n->depth, "depth");
  writeit(n->internal, "internal");
  writeit(n->router_decision, "decision");

  save_load_examples(b, *n, model_file, read, text, msg);
  save_load_weights(b, *n, model_file, read, text, msg);

  n->left  = save_load_node(b, n->left, model_file, read, text, msg);
  n->right = save_load_node(b, n->right, model_file, read, text, msg);

  if (n->left) { n->left->parent = n; }
  if (n->right) { n->right->parent = n; }

  return n;
}

void save_load_tree(tree& b, io_buf& model_file, bool read, bool text)
{
  std::stringstream msg;
  if (model_file.num_files() > 0)
  {
    if (read) { b.test_only = true; }

    uint32_t ss = b.all->weights.stride_shift();
    writeit(ss, "stride_shift");

    //this could likely be faster with a stack, if it is every a problem
    b.all->weights.stride_shift(ss);
 
    writeit(b.tree_bound, "tree_bound");
    writeit(b.leaf_split, "leaf_split");
    writeit(b.scorer_type, "scorer_type");

    b.root = save_load_node(b, b.root, model_file, read, text, msg);
    if (!b.all->quiet) { std::cout << "done loading...." << std::endl; }
  }
}
/////////////////End of Save & Load/////////////////////////////////
////////////////////////////////////////////////////////////////////
}

base_learner* VW::reductions::eigen_memory_tree_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();

  auto t = VW::make_unique<tree>();  
  bool _;

  option_group_definition new_options("[Reduction] Eigen Memory Tree");
  new_options
      .add(make_option("eigen_memory_tree", _)
               .keep()
               .necessary()
               .help("Make an eigen memory tree with at most <n> memories"))
      .add(make_option("tree", t->tree_bound)
               .keep()
               .default_value(-1)
               .help("Indicates the maximum number of memories the tree can have."))
    .add(make_option("leaf", t->leaf_split)
               .keep()
               .default_value(100)
               .help("Indicates the maximum number of memories a leaf can have."))
    .add(make_option("scorer", t->scorer_type)
               .keep()
               .default_value(3)
               .help("Indicates the type of scorer to use (1=random,2=distance,3=rank)"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  t->all = &all;
  t->_random_state = all.get_random_state();

  tree_init(*t);

  VW::prediction_type_t pred_type;
  VW::label_type_t label_type;

  // multi-class classification
  all.example_parser->lbl_parser = MULTICLASS::mc_label;
  pred_type = VW::prediction_type_t::multiclass;
  label_type = VW::label_type_t::multiclass;

  auto l = make_reduction_learner(std::move(t), as_singleline(stack_builder.setup_base_learner()), learn, predict,
      stack_builder.get_setupfn_name(eigen_memory_tree_setup))
               .set_end_pass(end_pass)
               .set_save_load(save_load_tree)
               .set_output_prediction_type(pred_type)
               .set_input_label_type(label_type);

  l.set_finish_example(MULTICLASS::finish_example<tree&>);

  return make_base(*l.build());
}
