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

float project(flat_example& ec, sparse_parameters& projector)
{
  float projection = 0;

  for (size_t idx1 = 0; idx1 < ec.fs.size(); idx1++)
  {
    projection += projector[ec.fs.indices[idx1]] * ec.fs.values[idx1];
  }

  return projection;
}

int compare(const void* a, const void* b) { return *(char*)a - *(char*)b; }

void scorer_features(features& f1, features& f2, features& out_f)
{
  out_f.clear();
  out_f.sum_feat_sq = 0;

  size_t idx1 = 0;
  size_t idx2 = 0;

  while (idx1 < f1.size() || idx2 < f2.size())
  {
    float out_value = 0;
    float f1_value = 0;
    float f2_value = 0;

    uint64_t out_index = 0;
    uint64_t f1_index = f1.indices[idx1];
    uint64_t f2_index = f2.indices[idx2];

    if (f1_index <= f2_index || idx2 == f2.size())
    {
      out_index = f1_index;
      f1_value = f1.values[idx1];
      idx1++;
    }

    if (f2_index <= f1_index || idx1 == f1.size())
    {
      out_index = f2_index;
      f2_value = f2.values[idx2];
      idx2++;
    }

    // out_value = f1_value * f2_value;
    out_value = abs(f1_value - f2_value);

    if (out_value != 0)
    {
      out_f.push_back(out_value, out_index);
      // out_f.sum_feat_sq += out_value * out_value; push_back on the line above already does this
    }
  }
}

void scorer_example(VW::flat_example& ec1, VW::flat_example& ec2, VW::example& out_example)
{
  out_example.total_sum_feat_sq = 0.0;
  scorer_features(ec1.fs, ec2.fs, out_example.feature_space[0]);
  out_example.total_sum_feat_sq = out_example.feature_space[0].sum_feat_sq;
}
////////////////////////////end of helper/////////////////////////
//////////////////////////////////////////////////////////////////

////////////////////////eigen_memory_tree///////////////////
////////////////////////////////////////////////////////////
struct node
{
  int64_t parent;  // parent index
  bool internal;   // (internal or leaf)
  uint32_t depth;  // depth.
  int64_t left;    // left child.
  int64_t right;   // right child.

  std::vector<uint32_t> example_indexes;

  double router_decision;
  sparse_parameters* router_weights = nullptr;

  node()  // construct:
  {
    parent = -1;
    internal = false;

    depth = 0;
    left = -1;
    right = -1;

    router_decision = 0;
    router_weights = nullptr;
  }

  float route(flat_example& ec)
  {
    return router_weights != nullptr ? project(ec, (*router_weights)) - router_decision : 0;
  }
};

struct eigen_memory_tree
{
  VW::workspace* all = nullptr;
  std::shared_ptr<VW::rand_state> _random_state;

  std::vector<node> nodes;              // array of nodes.
  std::vector<flat_example*> examples;  // array of example points

  size_t max_nodes = 0;
  size_t max_leaf_examples = 0;
  size_t leaf_example_multiplier = 0;

  int iter;
  bool test_only;
  int scorer_type;   // 1: random, 2: distance, 3: rank

  uint32_t total_num_queries = 0;

  size_t max_depth;
  size_t max_ex_in_leaf;

  size_t current_pass = 0;

  VW::example* scorer_ex = nullptr; //we create one of these which we re-use so we don't have to reallocate examples

  eigen_memory_tree()
  {
    iter = 0;
    max_depth = 0;
    max_ex_in_leaf = 0;
    test_only = false;
    scorer_type = 3;
  }

  ~eigen_memory_tree()
  {
    for (auto* ex : examples) { ::VW::free_flatten_example(ex); }
    if (scorer_ex) { ::VW::dealloc_examples(scorer_ex, 1); }
    for (auto& node : nodes) { delete node.router_weights; }
  }
};

void init_tree(eigen_memory_tree& b)
{
  b.iter = 0;
  b.max_depth = 0;
  b.scorer_type = 3;

  b.nodes.push_back(node());

  b.scorer_ex = ::VW::alloc_examples(1);
  b.scorer_ex->interactions = new std::vector<std::vector<VW::namespace_index>>();
  b.scorer_ex->extent_interactions = new std::vector<std::vector<extent_term>>();

  //we don't have features yet but we will later
  b.scorer_ex->indices.push_back(0);
  b.scorer_ex->num_features++;

  if (!b.all->quiet)
  {
    std::cout << "tree initiazliation is done...." << std::endl
              << "max nodes " << b.max_nodes << std::endl
              << "tree size: " << b.nodes.size() << std::endl
              << "current_pass: " << b.current_pass << std::endl;
  }
}

float initial(VW::example& ex) { return 1 - exp(-sqrt(ex.total_sum_feat_sq)); }

float scorer_predict(eigen_memory_tree& b, single_learner& base, VW::flat_example& pred_ec, VW::flat_example& leaf_ec)
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
    b.scorer_ex->_reduction_features.template get<simple_label_reduction_features>().initial = initial(*b.scorer_ex);

    base.predict(*b.scorer_ex);
    //std::cout << initial(*b.scorer_ex) << " " << b.scorer_ex->partial_prediction << " " << b.scorer_ex->pred.scalar << std::endl;
    return -b.scorer_ex->pred.scalar;
  }
}

void scorer_learn(eigen_memory_tree& b, single_learner& base, VW::flat_example& ec, float weight)
{
  // random and dist scorer has nothing to learsn
  if (b.scorer_type == 1 || b.scorer_type == 2) { return; }

  if (b.scorer_type == 3)  // rank scorer should learn
  {
    if (weight == 0) { return; }

    uint64_t cn = 0;
    while (b.nodes[cn].internal) { cn = b.nodes[cn].route(ec) < 0 ? b.nodes[cn].left : b.nodes[cn].right; }

    if (b.nodes[cn].example_indexes.size() < 2) { return; }

    // shuffle the examples to break ties randomly
    std::random_shuffle(b.nodes[cn].example_indexes.begin(), b.nodes[cn].example_indexes.end());

    float score_value_1 = -FLT_MAX;
    float score_reward_1 = -FLT_MAX;
    int score_index_1 = 0;

    float reward_value_1 = -FLT_MAX;
    int reward_index_1 = 0;

    float reward_value_2 = -FLT_MAX;
    int reward_index_2 = 0;

    for (auto index : b.nodes[cn].example_indexes)
    {
      auto score = scorer_predict(b, base, ec, *b.examples[index]);
      auto reward = (b.examples[index]->l.multi.label == ec.l.multi.label) ? 1.f : 0.f;

      if (score > score_value_1)
      {
        score_value_1 = score;
        score_index_1 = index;
        score_reward_1 = reward;
      }
      if (reward > reward_value_1)
      {
        reward_value_2 = reward_value_1;
        reward_index_2 = reward_index_1;

        reward_value_1 = reward;
        reward_index_1 = index;
      }
      else if (reward > reward_value_2)
      {
        reward_value_2 = reward;
        reward_index_2 = index;
      }
    }

    // get the index/reward for the example with the best score
    auto preferred_index = score_index_1;
    auto preferred_reward = score_reward_1;

    // get the index/reward for the best example without the best score
    auto alternative_index = (reward_index_1 == score_index_1) ? reward_index_2 : reward_index_1;
    auto alternative_reward = (reward_index_1 == score_index_1) ? reward_value_2 : reward_value_1;

    // the better of the two options moves towards 0 while the other moves towards -1
    weight *= abs(preferred_reward - alternative_reward);

    if (weight == 0) { return; }

    auto preferred_label = preferred_reward > alternative_reward ? 0.f : 1.f;
    auto alternative_label = alternative_reward > preferred_reward ? 0.f : 1.f;

    if (b._random_state->get_and_update_random() > .5)
    {
      scorer_example(ec, *b.examples[preferred_index], *b.scorer_ex);
      b.scorer_ex->pred.scalar = 0;
      b.scorer_ex->l.simple = {preferred_label};
      b.scorer_ex->weight = weight;
      b.scorer_ex->_reduction_features.template get<simple_label_reduction_features>().initial = initial(*b.scorer_ex);
      base.learn(*b.scorer_ex);
      
      scorer_example(ec, *b.examples[alternative_index], *b.scorer_ex);
      b.scorer_ex->pred.scalar = 0;
      b.scorer_ex->l.simple = {alternative_label};
      b.scorer_ex->weight = weight;
      b.scorer_ex->_reduction_features.template get<simple_label_reduction_features>().initial = initial(*b.scorer_ex);
      base.learn(*b.scorer_ex);
    }
    else
    {
      b.scorer_ex->pred.scalar = 0;
      scorer_example(ec, *b.examples[alternative_index], *b.scorer_ex);
      b.scorer_ex->l.simple = {alternative_label};
      b.scorer_ex->weight = weight;
      b.scorer_ex->_reduction_features.template get<simple_label_reduction_features>().initial = initial(*b.scorer_ex);
      base.learn(*b.scorer_ex);

      b.scorer_ex->pred.scalar = 0;
      scorer_example(ec, *b.examples[preferred_index], *b.scorer_ex);
      b.scorer_ex->l.simple = {preferred_label};
      b.scorer_ex->weight = weight;
      b.scorer_ex->_reduction_features.template get<simple_label_reduction_features>().initial = initial(*b.scorer_ex);
      base.learn(*b.scorer_ex);
    }
  }
}

void leaf_split(eigen_memory_tree& b, single_learner& base, const uint64_t cn)
{
  VW::rand_state rand_state = *b._random_state;
  std::vector<float> projs;

  uint64_t bits = static_cast<uint64_t>(1) << (b.all->num_bits);

  float best_variance = 0;
  float best_decision = 0;

  sparse_parameters* best_weights = new sparse_parameters(bits);
  sparse_parameters* new_weights = new sparse_parameters(bits);

  for (size_t perms = 0; perms < 90; perms++)
  {
    (*new_weights).set_zero(0);
    (*new_weights)
        .set_default([&rand_state](weight* weights, uint64_t) { weights[0] = rand_state.get_and_update_gaussian(); });

    projs.clear();

    for (size_t ec_id = 0; ec_id < b.nodes[cn].example_indexes.size(); ec_id++)
    {
      projs.push_back(project(*b.examples[b.nodes[cn].example_indexes[ec_id]], (*new_weights)));
    }

    float E = std::accumulate(projs.begin(), projs.end(), 0.0) / projs.size();
    float E_2 = std::inner_product(projs.begin(), projs.end(), projs.begin(), 0.0) / projs.size();
    float new_variance = E_2 - E * E;

    if (new_variance > best_variance)
    {
      best_variance = new_variance;
      std::swap(best_weights, new_weights);
      best_decision = median(projs);
    }
  }

  if (best_variance == 0)
  {
    // std::cout << "warning: all examples in a leaf have identical features" << std::endl;
    delete new_weights;
    delete best_weights;
  }
  else
  {
    delete new_weights;

    uint32_t left_i = static_cast<uint32_t>(b.nodes.size());
    uint32_t right_i = left_i + 1;

    b.nodes[cn].internal = true;
    b.nodes[cn].left = left_i;
    b.nodes[cn].right = right_i;

    b.nodes.push_back(node());
    b.nodes.push_back(node());

    b.nodes[left_i].depth = b.nodes[cn].depth + 1;
    b.nodes[right_i].depth = b.nodes[cn].depth + 1;
    b.nodes[left_i].parent = cn;
    b.nodes[right_i].parent = cn;

    if (b.nodes[cn].depth + 1 > b.max_depth)
    {
      b.max_depth = b.nodes[cn].depth + 1;
      std::cout << "depth " << b.max_depth << std::endl;
    }

    (*best_weights).set_default([](weight* weights, uint64_t) { weights[0] = 0; });
    b.nodes[cn].router_weights = best_weights;
    b.nodes[cn].router_decision = best_decision;

    for (uint32_t ec_i : b.nodes[cn].example_indexes)
    {
      b.nodes[b.nodes[cn].route(*b.examples[ec_i]) < 0 ? left_i : right_i].example_indexes.push_back(ec_i);
    }
    b.nodes[cn].example_indexes.clear();
  }
}

int64_t leaf_pick(eigen_memory_tree& b, single_learner& base, const uint64_t cn, VW::flat_example& ec)
{
  if (b.nodes[cn].example_indexes.size() == 0) { return -1; }

  float max_score = -FLT_MAX;
  std::vector<int> max_pos;

  for (uint32_t i : b.nodes[cn].example_indexes)
  {
    float score = scorer_predict(b, base, ec, *b.examples[i]);

    if (score > max_score)
    {
      max_score = score;
      max_pos.clear();
    }

    if (score == max_score) { max_pos.push_back(i); }
  }

  // we multiply by *.9999 so that get_and_update_random is in [0,.9999]
  return max_pos[static_cast<uint32_t>(b._random_state->get_and_update_random() * max_pos.size() * .9999)];
}

void insert(eigen_memory_tree& b, single_learner& base, VW::flat_example& ec)
{
  uint64_t cn = 0;
  while (b.nodes[cn].internal) { cn = b.nodes[cn].route(ec) < 0 ? b.nodes[cn].left : b.nodes[cn].right; }

  b.nodes[cn].example_indexes.push_back(static_cast<uint32_t>(b.examples.size()));
  b.examples.push_back(&ec);

  if ((b.nodes[cn].example_indexes.size() >= b.max_leaf_examples) && (b.nodes.size() + 2 <= b.max_nodes))
  {
    leaf_split(b, base, cn);
  }
}

void predict(eigen_memory_tree& b, single_learner& base, VW::example& ec)
{
  // auto true_label = ec.l.multi;
  // auto pred_label = ec.pred.multiclass;

  VW::flat_example& flat_ec = *VW::flatten_sort_example(*b.all, &ec);

  uint64_t cn = 0;
  while (b.nodes[cn].internal) { cn = b.nodes[cn].route(flat_ec) < 0 ? b.nodes[cn].left : b.nodes[cn].right; }

  // ec.l.multi = true_label;
  // ec.pred.multiclass = pred_label;

  int64_t closest_ec = leaf_pick(b, base, cn, flat_ec);

  ec.pred.multiclass = closest_ec != -1 ? b.examples[closest_ec]->l.multi.label : 0;
  ec.loss = (ec.l.multi.label != ec.pred.multiclass) ? ec.weight : 0;
}

void learn(eigen_memory_tree& b, single_learner& base, VW::example& ec)
{
  VW::flat_example& flat_ec = *VW::flatten_sort_example(*b.all, &ec);

  b.iter++;

  if (b.test_only) { return; }

  scorer_learn(b, base, flat_ec, ec.weight);

  if (b.current_pass == 0) { insert(b, base, flat_ec); }
}

void end_pass(eigen_memory_tree& b)
{
  b.current_pass++;
  std::cout << "##### Ending pass " << b.current_pass << " with " << b.examples.size() << " memories stored. "
            << std::endl;
}
/////////////////end of eigen_memory_tree///////////////////
////////////////////////////////////////////////////////////

///////////////////Save & Load//////////////////////////////////////
////////////////////////////////////////////////////////////////////
void save_load_node(node& cn, io_buf& model_file, bool& read, bool& text, std::stringstream& msg)
{
  writeit(cn.parent, "parent");
  writeit(cn.internal, "internal");
  writeit(cn.depth, "depth");
  writeit(cn.left, "left");
  writeit(cn.right, "right");
  writeitvar(cn.example_indexes.size(), "leaf_n_examples", leaf_n_examples);

  if (read)
  {
    cn.example_indexes.clear();
    for (uint32_t k = 0; k < leaf_n_examples; k++) { cn.example_indexes.push_back(0); }
  }
  for (uint32_t k = 0; k < leaf_n_examples; k++) writeit(cn.example_indexes[k], "example_location");

  if (cn.internal)
  {
    uint32_t router_dims = 0;
    size_t router_length = 0;
    uint32_t router_shift = 0;

    if (cn.router_weights != nullptr)
    {
      for (sparse_parameters::iterator i = cn.router_weights->begin(); i != cn.router_weights->end(); ++i)
      {
        if (*i != 0) { router_dims++; }
      }
      router_shift = cn.router_weights->stride_shift();
      router_length = (cn.router_weights->mask() + 1) >> router_shift;
    }

    writeit(cn.router_decision, "router_decision");
    writeit(router_dims, "router_dims");
    writeit(router_length, "router_length");
    writeit(router_shift, "router_shift");

    if (read)
    {
      cn.router_weights = new sparse_parameters(router_length, router_shift);
      for (int i = 0; i < router_dims; i++)
      {
        uint64_t index = 0;
        float value = 0;
        writeit(index, "router_index");
        writeit(value, "router_value");

        (*cn.router_weights)[index] = value;
      }
    }
    else
    {
      for (sparse_parameters::iterator i = cn.router_weights->begin(); i != cn.router_weights->end(); ++i)
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
}

void save_load_tree(eigen_memory_tree& b, io_buf& model_file, bool read, bool text)
{
  std::stringstream msg;
  if (model_file.num_files() > 0)
  {
    if (read) { b.test_only = true; }

    if (read)
    {
      uint32_t ss = 0;
      writeit(ss, "stride_shift");
      b.all->weights.stride_shift(ss);
    }
    else
    {
      uint32_t ss = b.all->weights.stride_shift();
      writeit(ss, "stride_shift");
    }

    writeit(b.max_nodes, "max_nodes");
    writeit(b.leaf_example_multiplier, "leaf_example_multiplier") writeitvar(b.nodes.size(), "nodes", n_nodes);

    if (read)
    {
      b.nodes.clear();
      for (uint32_t i = 0; i < n_nodes; i++) { b.nodes.push_back(node()); }
    }

    // node
    for (uint32_t i = 0; i < n_nodes; i++) { save_load_node(b.nodes[i], model_file, read, text, msg); }
    // deal with examples:
    writeitvar(b.examples.size(), "examples", n_examples);
    for (uint32_t i = 0; i < n_examples; i++)
    {
      if (read)
      {
        flat_example fec;
        VW::model_utils::read_model_field(model_file, fec, b.all->example_parser->lbl_parser);
        b.examples.push_back(&fec);
      }
      else
      {
        VW::model_utils::write_model_field(
            model_file, *b.examples[i], "_flat_example", false, b.all->example_parser->lbl_parser, b.all->parse_mask);
      }
    }
    // std::cout<<"done loading...."<< std::endl;
  }
}
/////////////////End of Save & Load/////////////////////////////////
////////////////////////////////////////////////////////////////////
}  // namespace

base_learner* VW::reductions::eigen_memory_tree_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();

  auto tree = VW::make_unique<eigen_memory_tree>();
  uint64_t max_nodes;
  uint64_t leaf_example_multiplier;
  option_group_definition new_options("[Reduction] Eigen Memory Tree");

  // leaf_example_multiplier controls how many examples per leaf before splitting
  new_options
      .add(make_option("eigen_memory_tree", max_nodes)
               .keep()
               .necessary()
               .default_value(0)
               .help("Make a memory tree with at most <n> nodes"))
      .add(make_option("leaf_example_multiplier", leaf_example_multiplier)
               .default_value(1)
               .help("Multiplier on examples per leaf (default = log nodes)"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  tree->max_nodes = VW::cast_to_smaller_type<size_t>(max_nodes);
  tree->leaf_example_multiplier = VW::cast_to_smaller_type<size_t>(leaf_example_multiplier);
  tree->all = &all;
  tree->_random_state = all.get_random_state();
  tree->current_pass = 0;

  tree->max_leaf_examples = static_cast<size_t>(tree->leaf_example_multiplier * (log(tree->max_nodes) / log(2)));

  init_tree(*tree);

  if (!all.quiet)
  {
    *(all.trace_message) << "eigen_memory_tree: "
                         << "max_nodes = " << tree->max_nodes << " "
                         << "max_leaf_examples = " << tree->max_leaf_examples << std::endl;
  }

  VW::prediction_type_t pred_type;
  VW::label_type_t label_type;

  // multi-class classification
  all.example_parser->lbl_parser = MULTICLASS::mc_label;
  pred_type = VW::prediction_type_t::multiclass;
  label_type = VW::label_type_t::multiclass;

  auto l = make_reduction_learner(std::move(tree), as_singleline(stack_builder.setup_base_learner()), learn, predict,
      stack_builder.get_setupfn_name(eigen_memory_tree_setup))
               .set_end_pass(end_pass)
               .set_save_load(save_load_tree)
               .set_output_prediction_type(pred_type)
               .set_input_label_type(label_type);

  l.set_finish_example(MULTICLASS::finish_example<eigen_memory_tree&>);

  return make_base(*l.build());
}
