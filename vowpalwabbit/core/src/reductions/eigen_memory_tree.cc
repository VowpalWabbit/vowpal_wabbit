// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/eigen_memory_tree.h"

#include "vw/common/future_compat.h"
#include "vw/config/options.h"
#include "vw/core/array_parameters.h"
#include "vw/core/example.h"
#include "vw/core/learner.h"
#include "vw/core/numeric_casts.h"
#include "vw/core/rand48.h"
#include "vw/core/rand_state.h"
#include "vw/core/setup_base.h"
#include "vw/core/v_array.h"
#include "vw/core/vw.h"
#include "vw/io/logger.h"

#include <algorithm>
#include <cfloat>
#include <climits>
#include <cmath>
#include <cstdio>
#include <ctime>
#include <list>
#include <memory>
#include <sstream>

using namespace VW::LEARNER;
using namespace VW::config;

// TODO: This file has several cout print statements. It looks like
//       they should be using trace_message, but its difficult to tell
namespace VW
{
namespace reductions
{
namespace eigen_memory_tree
{
///////////////////////Helper/////////////////////////////
//////////////////////////////////////////////////////////
float median(std::vector<float>& array)
{
  int size = array.size();
  std::sort(array.begin(), array.end());

  if (size % 2 == 0) { return (array[size / 2 - 1] + array[size / 2]) / 2; }
  else { return array[size / 2]; }
}

float inner(features& fs, sparse_parameters& weights)
{
  float inner = 0;
  for (auto& f : fs) { inner += weights[f.index()] * f.value(); }
  return inner;
}

float inner(VW::flat_example& example, sparse_parameters& weights) { return inner(example.fs, weights); }

void scale(sparse_parameters& weights, float scalar)
{
  for (sparse_parameters::iterator i = weights.begin(); i != weights.end(); ++i)
  {
    weights[i.index()] = weights[i.index()] * scalar;
  }
}

float norm(sparse_parameters& weights)
{
  float sum_weights_sq = 0;

  for (auto& w : weights) { sum_weights_sq += w * w; }

  return std::sqrt(sum_weights_sq);
}
////////////////////////////end of helper/////////////////////////
//////////////////////////////////////////////////////////////////

////////////////////////////definitions/////////////////////
////////////////////////////////////////////////////////////

emt_example::emt_example() {
  base = nullptr;
  full = nullptr;
  score = 0;
  label = 0;
  tag = -1;
}

emt_example::emt_example(VW::workspace& all, VW::example* ex)
{
  label = ex->l.multi.label;
  tag = -1;
  score = 0;

  auto full_interactions = ex->interactions;
  auto base_interactions = new std::vector<std::vector<VW::namespace_index>>();

  ex->interactions = base_interactions;
  base = VW::flatten_sort_example(all, ex);

  ex->interactions = full_interactions;
  full = VW::flatten_sort_example(all, ex);
}

emt_LRU::emt_LRU(unsigned long max_size) { (*this).max_size = max_size; }

emt_LRU::K emt_LRU::bound(emt_LRU::K item)
{
  if (max_size == 0) { return nullptr; }

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
    V item_list_reference = (*item_map_reference).second;
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

emt_node::emt_node() {
  parent = nullptr;
  left = nullptr;
  right = nullptr;

  internal = false;
  depth = 0;

  router_decision = 0;
  router_weights = nullptr;
}

emt_tree::emt_tree() {
  iter = 0;
  pass = 0;
  depth = 0;
  test_only = false;

  tree_bound = -1;
  leaf_split = 100;

  scorer_type = emt_scorer_type::self_consistent_rank;
  router_type = emt_router_type::eigen;

  begin = clock();
  time = 0;

  all = nullptr;
  ex = nullptr;
  root = nullptr;
  bounder = nullptr;
}

void emt_tree::deallocate_node(emt_node* n)
{
  if (!n) { return; }
  for (emt_example* e : n->examples)
  {
    VW::free_flatten_example(e->base);
    VW::free_flatten_example(e->full);
  }
  deallocate_node(n->left);
  deallocate_node(n->right);
}

emt_tree::~emt_tree()
{
  deallocate_node(root);
  if (ex) { VW::dealloc_examples(ex, 1); }
}

/// <summary>
/// This has been added so that we can shuffle and have repeatable results based on VW's internal random number
/// generator and its seed
/// </summary>
struct emt_rng
{
  std::shared_ptr<VW::rand_state> state;

public:
  emt_rng(std::shared_ptr<VW::rand_state> state) { this->state = std::move(state); }

  using result_type = size_t;
  static constexpr result_type min() { return 0; }
  static constexpr result_type max() { return 1000; }
  result_type operator()() { return static_cast<size_t>(state->get_and_update_random() * .999 * emt_rng::max()); }
};
////////////////////////end of definitions/////////////////
///////////////////////////////////////////////////////////

////////////////////////eigen_memory_tree///////////////////
////////////////////////////////////////////////////////////
emt_node* node_route(emt_node& cn, emt_example& ec)
{
  return inner(*ec.base, *cn.router_weights) < cn.router_decision ? cn.left : cn.right;
}

void tree_init(emt_tree& b)
{
  b.iter = 0;
  b.depth = 0;
  b.pass = 0;

  b.root = new emt_node();

  b.bounder = new emt_LRU(b.tree_bound);

  // we set this up for repeated use later in the scorer.
  // we will populate this examples features over and over.
  b.ex = ::VW::alloc_examples(1);
  for (int i = 0; i != 1; i++)
  {
    b.ex[i].interactions = new std::vector<std::vector<VW::namespace_index>>();
    b.ex[i].extent_interactions = new std::vector<std::vector<extent_term>>();
    b.ex[i].indices.push_back(0);
  }
}

emt_node& tree_route(emt_tree& b, emt_example& ec)
{
  emt_node* cn = b.root;
  while (cn->internal) { cn = node_route(*cn, ec); }
  return *cn;
}

void tree_bound(emt_tree& b, emt_example* ec)
{
  emt_example* to_delete = b.bounder->bound(ec);

  if (to_delete == nullptr) { return; }

  emt_node& cn = tree_route(b, *to_delete);

  for (auto iter = cn.examples.begin(); iter != cn.examples.end(); iter++)
  {
    if (*iter == to_delete)
    {
      cn.examples.erase(iter);
      return;
    }
  }
}

float scorer_initial(VW::example& ex) { return 1 - std::exp(-std::sqrt(ex.total_sum_feat_sq)); }

/// <summary>
/// Calculate pairwise difference features for a scorer example
/// </summary>
/// <param name="f1">features from scorer example 1</param>
/// <param name="f2">features from scorer eample 2</param>
/// <param name="out">the features object calcuated features will be written to</param>
/// <param name="feature_type"> difference or absolute difference</param>
void scorer_features(features& f1, features& f2, features& out, emt_scorer_feature_type feature_type)
{
  out.values.clear();
  out.indices.clear();
  out.sum_feat_sq = 0;

  size_t idx1 = 0;
  size_t idx2 = 0;

  uint64_t index = 0;
  uint64_t f1_idx = 0;
  uint64_t f2_idx = 0;

  float f1_val = 0.f;
  float f2_val = 0.f;

  while (idx1 < f1.size() || idx2 < f2.size())
  {
    f1_idx = (idx1 == f1.size()) ? INT_MAX : f1.indices[idx1];
    f2_idx = (idx2 == f2.size()) ? INT_MAX : f2.indices[idx2];

    f1_val = 0.f;
    f2_val = 0.f;

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
      float value = 0;
      if (feature_type == emt_scorer_feature_type::abs_diff) { value = std::abs(f1_val - f2_val); }
      else if (feature_type == emt_scorer_feature_type::diff) { value = f1_val - f2_val; }
      else { THROW("An unrecognized feature type was provided.") }

      out.values.push_back(value);
      out.indices.push_back(index);
      out.sum_feat_sq += value * value;
    }
  }
}

/// <summary>
/// Initialize an example to be used either for learning or predicting with the scorer
/// </summary>
/// <param name="ex1">memory 1 to use for constructing the scorer example</param>
/// <param name="ex2">memory 2 to use for constructing the scorer example</param>
/// <param name="out">the examples object which we are initializing</param>
/// <param name="example_type">The type of example to create. 1--self-consistent 2--polynomial</param>
void scorer_example(
    emt_tree& b, emt_example& ex1, emt_example& ex2, VW::example& out, emt_scorer_example_type example_type)
{
  if (example_type == emt_scorer_example_type::self_consistent ||
      example_type == emt_scorer_example_type::not_self_consistent)
  {
    out.indices.clear();
    out.indices.push_back('x');

    out.interactions->clear();

    out.feature_space['x'].clear();
    out.feature_space['z'].clear();

    scorer_features(ex1.full->fs, ex2.full->fs, out.feature_space['x'], emt_scorer_feature_type::abs_diff);

    out.total_sum_feat_sq = out.feature_space['x'].sum_feat_sq;
    out.num_features = out.feature_space['x'].size();

    out.ex_reduction_features.template get<VW::simple_label_reduction_features>().initial = scorer_initial(out);
  }

  if (example_type == emt_scorer_example_type::not_self_consistent)
  {
    out.indices.clear();
    out.indices.push_back('x');
    out.indices.push_back('z');

    out.interactions->clear();
    out.interactions->push_back({'x', 'z'});

    b.all->ignore_some_linear = true;
    b.all->ignore_linear['x'] = true;
    b.all->ignore_linear['z'] = true;

    // creates a copy
    out.feature_space['x'] = ex1.full->fs;
    out.feature_space['z'] = ex2.full->fs;

    // when we receive ex1 and ex2 their features are indexed on top of eachother. In order
    // to make sure VW recognizes the features from the two examples as separate features
    // we apply a map of multiplying by 2 and then offseting by 1 on the second example.
    for (auto& j : out.feature_space['x'].indices) { j = j * 2; }
    for (auto& j : out.feature_space['z'].indices) { j = j * 2 + 1; }

    out.total_sum_feat_sq = out.feature_space['x'].sum_feat_sq + out.feature_space['z'].sum_feat_sq;
    out.num_features = out.feature_space['x'].size() + out.feature_space['z'].size();
  }

  // We cache metadata about model weights adjacent to them. For example if we have
  // a model weight w[i] then we may also store information about our confidence in
  // w[i] at w[i+1] and information about the scale of feature f[i] at w[i+2] and so on.
  // This variable indicates how many such meta-data places we need to save in between actual weights.
  uint64_t floats_per_feature_index = static_cast<uint64_t>(b.all->wpp) << b.all->weights.stride_shift();

  // In both of the example_types above we construct our scorer_example from flat_examples. The VW routine
  // which creates flat_examples removes the floats_per_feature_index from the when flattening. Therefore,
  // we need to manually add it back to make sure our base learner doesn't overwrite our features/weights
  // with metadata.
  if (floats_per_feature_index != 1)
  {
    for (features& fs : out)
    {
      for (auto& j : fs.indices) { j *= floats_per_feature_index; }
    }
  }
}

float scorer_predict(emt_tree& b, single_learner& base, emt_example& pred_ex, emt_example& leaf_ex)
{
  if (b.scorer_type == emt_scorer_type::random)  // random scorer
  {
    return b._random_state->get_and_update_random();
  }

  if (b.scorer_type == emt_scorer_type::distance)  // dist scorer
  {
    scorer_example(b, pred_ex, leaf_ex, *b.ex, emt_scorer_example_type::self_consistent);
    return b.ex->total_sum_feat_sq;
  }

  if (b.scorer_type == emt_scorer_type::self_consistent_rank ||
      b.scorer_type == emt_scorer_type::not_self_consistent_rank)  // rank scorer
  {
    auto example_type = (b.scorer_type == emt_scorer_type::self_consistent_rank)
        ? emt_scorer_example_type::self_consistent
        : emt_scorer_example_type::not_self_consistent;

    scorer_example(b, pred_ex, leaf_ex, *b.ex, example_type);

    // The features matched exactly. Return max negative to make sure it is picked.
    if (b.ex->ex_reduction_features.template get<VW::simple_label_reduction_features>().initial == 0)
    {
      return -FLT_MAX;
    }

    b.ex->l.simple = {FLT_MAX};
    base.predict(*b.ex);

    return b.ex->pred.scalar;
  }

  THROW("An unrecognized scorer type was provided.")
}

void scorer_learn(single_learner& base, VW::example& ex, float label, float weight)
{
  if (ex.total_sum_feat_sq != 0)
  {
    ex.pred.scalar = 0;
    ex.l.simple = {label};
    ex.weight = weight;
    base.learn(ex);
  }
}

void scorer_learn(emt_tree& b, single_learner& base, emt_node& cn, emt_example& ex, float weight)
{
  // random and dist scorer has nothing to learsn
  if (b.scorer_type == emt_scorer_type::random || b.scorer_type == emt_scorer_type::distance) { return; }

  if (b.scorer_type == emt_scorer_type::self_consistent_rank ||
      b.scorer_type == emt_scorer_type::not_self_consistent_rank)
  {
    auto example_type = (b.scorer_type == emt_scorer_type::self_consistent_rank)
        ? emt_scorer_example_type::self_consistent
        : emt_scorer_example_type::not_self_consistent;

    if (weight == 0) { return; }

    if (cn.examples.size() < 2) { return; }

    // shuffle the examples to break ties randomly
    std::shuffle(cn.examples.begin(), cn.examples.end(), emt_rng(b._random_state));

    float preferred_score = FLT_MAX;
    float preferred_error = FLT_MAX;
    emt_example* preferred_ex = nullptr;

    float alternative_error = FLT_MAX;
    emt_example* alternative_ex = nullptr;

    for (emt_example* example : cn.examples)
    {
      float score = scorer_predict(b, base, ex, *example);

      if (score < preferred_score)
      {
        preferred_score = score;
        preferred_ex = example;
        preferred_error = (example->label == ex.label) ? 0.f : 1.f;
      }
    }

    for (emt_example* example : cn.examples)
    {
      if (example == preferred_ex) { continue; }

      float error = (example->label == ex.label) ? 0.f : 1.f;
      if (error < alternative_error)
      {
        alternative_error = error;
        alternative_ex = example;
      }
    }

    // the better of the two options moves towards 0 while the other moves towards -1
    weight *= std::abs(preferred_error - alternative_error);

    if (alternative_ex == nullptr || preferred_ex == nullptr)
    {
      std::cout << "ERROR" << std::endl;
      return;
    }

    if (weight != 0)
    {
      if (b._random_state->get_and_update_random() < .5)
      {
        scorer_example(b, ex, *preferred_ex, b.ex[0], example_type);
        scorer_learn(base, b.ex[0], int(preferred_error > alternative_error), weight);

        scorer_example(b, ex, *alternative_ex, b.ex[0], example_type);
        scorer_learn(base, b.ex[0], int(alternative_error > preferred_error), weight);
      }
      else
      {
        scorer_example(b, ex, *alternative_ex, b.ex[0], example_type);
        scorer_learn(base, b.ex[0], int(alternative_error > preferred_error), weight);

        scorer_example(b, ex, *preferred_ex, b.ex[0], example_type);
        scorer_learn(base, b.ex[0], int(preferred_error > alternative_error), weight);
      }

      // doing the trick below doesn't work as well as the two separate updates. Why? It does seem to be faster.
      // scorer_example(ex, *preferred_ex, b.ex[0]);
      // scorer_example(ex, *alternative_ex, b.ex[1]);
      // scorer_features(b.ex[0].feature_space[0], b.ex[1].feature_space[0], b.ex[2].feature_space[0], 2);

      // b.ex[2].total_sum_feat_sq = b.ex[2].feature_space[0].sum_feat_sq;
      // b.ex[2].num_features = b.ex[2].feature_space[0].size();

      // float label = (preferred_reward < alternative_reward) ? 1.f : -1.f;
      // float initial = scorer_initial(b.ex[0]) - scorer_initial(b.ex[1]);
      // scorer_learn(b, base, b.ex[2], label, 1.5*weight, initial);
    }
  }
}

void node_split(emt_tree& b, emt_node& cn)
{
  if (cn.examples.size() <= b.leaf_split) { return; }

  uint64_t bits = static_cast<uint64_t>(1) << (b.all->num_bits);

  auto& rand_state = *b._random_state;
  auto random_positive = [&rand_state](VW::weight* weights, uint64_t)
  { weights[0] = rand_state.get_and_update_random(); };
  sparse_parameters* weights = new sparse_parameters(bits);
  weights->set_default(random_positive);

  std::vector<VW::flat_example*> examples;
  for (emt_example* example : cn.examples) { examples.push_back(example->base); }

  for (VW::flat_example* ex : examples)
  {
    for (auto& f : ex->fs) { weights[f.index()]; }
  }
  scale(*weights, 1 / norm(*weights));

  if (b.router_type == emt_router_type::eigen)
  {
    float n_examples = examples.size();

    std::unordered_map<int, float> mean_map;
    for (VW::flat_example* ex : examples)
    {
      for (auto& f : ex->fs) { mean_map[f.index()] += (1 / n_examples) * f.value(); }
    }

    features feature_means;
    for (auto& map : mean_map) { feature_means.push_back(map.second, map.first); }

    std::vector<features> centered_features;
    for (int i = 0; i < n_examples; i++)
    {
      features fs;
      centered_features.push_back(fs);
      scorer_features(examples[i]->fs, centered_features[i], feature_means, emt_scorer_feature_type::diff);
    }

    int n_epochs = 20;  // the bigger the better the top eigen approximation wil

    for (int i = 0; i < n_epochs; i++)
    {
      // reseting n on each epoch gives
      // projectors that are substantially
      // closer to the true top eigen vector
      // in experiments
      float n = 1;
      std::shuffle(centered_features.begin(), centered_features.end(), emt_rng(b._random_state));

      for (features fs : centered_features)
      {
        // in matrix multiplication notation this looks like:
        // weights = weights + (1/n) * inner(outer(example->fs,example->fs), weights)
        float scalar = (1 / n) * inner(fs, *weights);
        for (auto& f : fs) { (*weights)[f.index()] += f.value() * scalar; }
        scale(*weights, 1 / norm(*weights));
        n += 1;
      }
    }
  }

  auto* left = new emt_node();
  auto* right = new emt_node();
  auto depth = cn.depth + 1;

  cn.internal = true;
  cn.left = left;
  cn.right = right;

  left->depth = depth;
  right->depth = depth;
  left->parent = &cn;
  right->parent = &cn;

  if (depth > b.depth) { b.depth = depth; }

  std::vector<float> projs;
  projs.reserve(examples.size());
  for (VW::flat_example* example : examples) { projs.push_back(inner(*example, *weights)); }

  cn.router_weights = weights;
  cn.router_decision = median(projs);

  for (emt_example* example : cn.examples) { node_route(cn, *example)->examples.push_back(example); }
  cn.examples.clear();
}

void node_insert(emt_tree& b, emt_node& cn, emt_example& ex)
{
  for (emt_example* example : cn.examples)
  {
    scorer_example(b, ex, *example, b.ex[0], emt_scorer_example_type::self_consistent);
    if (b.ex[0].total_sum_feat_sq == 0) { return; }
  }

  cn.examples.push_back(&ex);
}

emt_example* node_pick(emt_tree& b, single_learner& base, emt_node& cn, emt_example& ex)
{
  if (cn.examples.size() == 0) { return nullptr; }

  float best_score = FLT_MAX;
  std::vector<emt_example*> best_examples;

  for (emt_example* example : cn.examples)
  {
    example->score = scorer_predict(b, base, ex, *example);

    if (example->score == best_score) { best_examples.push_back(example); }
    else if (example->score < best_score)
    {
      best_score = example->score;
      best_examples.clear();
      best_examples.push_back(example);
    }
  }

  std::shuffle(best_examples.begin(), best_examples.end(), emt_rng(b._random_state));
  return best_examples[0];
}

void node_predict(emt_tree& b, single_learner& base, emt_node& cn, emt_example& ex, VW::example& ec)
{
  auto closest_ex = node_pick(b, base, cn, ex);

  ec.confidence = (closest_ex != nullptr) ? (1 - std::exp(-closest_ex->score)) : 0;
  ec.pred.multiclass = (closest_ex != nullptr) ? closest_ex->label : 0;
  ec.loss = (ec.l.multi.label != ec.pred.multiclass) ? ec.weight : 0;
}

void predict(emt_tree& b, single_learner& base, VW::example& ec)
{
  b.all->ignore_some_linear = false;
  emt_example ex(*b.all, &ec);

  emt_node& cn = tree_route(b, ex);
  node_predict(b, base, cn, ex, ec);
  tree_bound(b, &ex);
}

void learn(emt_tree& b, single_learner& base, VW::example& ec)
{
  b.all->ignore_some_linear = false;
  emt_example& ex = *new emt_example(*b.all, &ec);

  ex.tag = b.iter;

  b.iter++;

  if (b.test_only) { return; }

  emt_node& cn = tree_route(b, ex);
  scorer_learn(b, base, cn, ex, ec.weight);
  node_predict(b, base, cn, ex, ec);  // vw learners predict and learn

  if (b.pass == 0)
  {
    node_insert(b, cn, ex);
    tree_bound(b, &ex);
    node_split(b, cn);
  }
}

void end_pass(emt_tree& b)
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
void save_load_examples(emt_tree& b, emt_node& n, io_buf& model_file, bool& read, bool& text, std::stringstream& msg)
{
  WRITEITVAR(n.examples.size(), "n_examples", n_examples);

  auto parser = b.all->example_parser->lbl_parser;
  auto mask = b.all->parse_mask;

  if (read)
  {
    for (uint32_t i = 0; i < n_examples; i++) { n.examples.push_back(new emt_example()); }
  }

  for (emt_example* e : n.examples)
  {
    if (read)
    {
      WRITEIT(e->label, "example_label")
      VW::model_utils::read_model_field(model_file, *e->base, parser);
      VW::model_utils::read_model_field(model_file, *e->full, parser);
    }
    else
    {
      WRITEIT(e->label, "example_label");
      VW::model_utils::write_model_field(model_file, *e->base, "_memory_base", false, parser, mask);
      VW::model_utils::write_model_field(model_file, *e->full, "_memory_full", false, parser, mask);
    }
  }
}

void save_load_weights(emt_node& n, io_buf& model_file, bool& read, bool& text, std::stringstream& msg)
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

  WRITEIT(n.router_decision, "router_decision");
  WRITEIT(router_dims, "router_dims");
  WRITEIT(router_length, "router_length");
  WRITEIT(router_shift, "router_shift");

  if (read)
  {
    n.router_weights = new sparse_parameters(router_length, router_shift);
    for (uint32_t i = 0; i < router_dims; i++)
    {
      uint64_t index = 0;
      float value = 0;
      WRITEIT(index, "router_index");
      WRITEIT(value, "router_value");

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
        WRITEIT(index, "router_index");
        WRITEIT(value, "router_value");
      }
    }
  }
}

emt_node* save_load_node(emt_tree& b, emt_node* n, io_buf& model_file, bool& read, bool& text, std::stringstream& msg)
{
  WRITEITVAR(!read && !n, "is_null", is_null);
  if (is_null) { return nullptr; }

  if (!n) { n = new emt_node(); }

  WRITEIT(n->depth, "depth");
  WRITEIT(n->internal, "internal");
  WRITEIT(n->router_decision, "decision");

  save_load_examples(b, *n, model_file, read, text, msg);
  save_load_weights(*n, model_file, read, text, msg);

  n->left = save_load_node(b, n->left, model_file, read, text, msg);
  n->right = save_load_node(b, n->right, model_file, read, text, msg);

  if (n->left) { n->left->parent = n; }
  if (n->right) { n->right->parent = n; }

  return n;
}

void save_load_tree(emt_tree& b, io_buf& model_file, bool read, bool text)
{
  std::stringstream msg;
  if (model_file.num_files() > 0)
  {
    if (read) { b.test_only = true; }

    uint32_t ss = b.all->weights.stride_shift();
    WRITEIT(ss, "stride_shift");

    // this could likely be faster with a stack, if it is every a problem
    b.all->weights.stride_shift(ss);

    int scorer_type = static_cast<int>(b.scorer_type);
    int router_type = static_cast<int>(b.router_type);

    WRITEIT(b.tree_bound, "tree_bound");
    WRITEIT(b.leaf_split, "leaf_split");
    WRITEIT(scorer_type, "scorer_type");
    WRITEIT(router_type, "router_type");

    b.scorer_type = static_cast<emt_scorer_type>(scorer_type);
    b.router_type = static_cast<emt_router_type>(router_type);

    b.root = save_load_node(b, b.root, model_file, read, text, msg);
    if (!b.all->quiet) { std::cout << "done loading...." << std::endl; }
  }
}
/////////////////End of Save & Load/////////////////////////////////
////////////////////////////////////////////////////////////////////
}  // namespace eigen_memory_tree
}  // namespace reductions
}  // namespace VW

base_learner* VW::reductions::eigen_memory_tree_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();

  auto t = VW::make_unique<VW::reductions::eigen_memory_tree::emt_tree>();
  bool _;
  uint32_t scorer_type = 0, router_type = 0;

  option_group_definition new_options("[Reduction] Eigen Memory Tree");
  new_options
      .add(make_option("eigen_memory_tree", _)
               .keep()
               .necessary()
               .help("Make an eigen memory tree with at most <n> memories"))
      .add(make_option("tree", t->tree_bound)
               .keep()
               .default_value(0)
               .help("Indicates the maximum number of memories the tree can have."))
      .add(make_option("leaf", t->leaf_split)
               .keep()
               .default_value(100)
               .help("Indicates the maximum number of memories a leaf can have."))
      .add(make_option("scorer", scorer_type)
               .keep()
               .one_of({1,2,3})
               .default_value(3)
               .help("Indicates the type of scorer to use (1=random,2=distance,3=self consistent rank,4=not self consistent rank)"))
      .add(make_option("router", router_type)
               .keep()
               .one_of({1, 2})
               .default_value(2)
               .help("Indicates the type of router to use (1=random;2=eigen)"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  t->scorer_type = static_cast<VW::reductions::eigen_memory_tree::emt_scorer_type>(scorer_type);
  t->router_type = static_cast<VW::reductions::eigen_memory_tree::emt_router_type>(router_type);

  t->all = &all;
  t->_random_state = all.get_random_state();

  tree_init(*t);

  VW::prediction_type_t pred_type;
  VW::label_type_t label_type;

  // multi-class classification
  all.example_parser->lbl_parser = VW::multiclass_label_parser_global;
  pred_type = VW::prediction_type_t::MULTICLASS;
  label_type = VW::label_type_t::MULTICLASS;

  auto learn = VW::reductions::eigen_memory_tree::learn;
  auto predict = VW::reductions::eigen_memory_tree::predict;
  auto end_pass = VW::reductions::eigen_memory_tree::end_pass;
  auto save_load = VW::reductions::eigen_memory_tree::save_load_tree;

  auto& l = make_reduction_learner(std::move(t), as_singleline(stack_builder.setup_base_learner()), learn, predict,
      stack_builder.get_setupfn_name(eigen_memory_tree_setup))
                .set_learn_returns_prediction(true)  // we set this to true otherwise bounding doesn't work as well
                .set_end_pass(end_pass)
                .set_save_load(save_load)
                .set_output_prediction_type(pred_type)
                .set_input_label_type(label_type);

  l.set_finish_example(VW::details::finish_multiclass_example<VW::reductions::eigen_memory_tree::emt_tree&>);

  return make_base(*l.build());
}
