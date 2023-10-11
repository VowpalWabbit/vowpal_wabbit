// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/eigen_memory_tree.h"

#include "vw/common/future_compat.h"
#include "vw/common/random.h"
#include "vw/common/string_view.h"
#include "vw/common/vw_throw.h"
#include "vw/config/options.h"
#include "vw/core/array_parameters.h"
#include "vw/core/example.h"
#include "vw/core/feature_group.h"
#include "vw/core/learner.h"
#include "vw/core/memory.h"
#include "vw/core/model_utils.h"
#include "vw/core/multiclass.h"
#include "vw/core/numeric_casts.h"
#include "vw/core/setup_base.h"
#include "vw/core/v_array.h"
#include "vw/core/vw.h"
#include "vw/core/vw_fwd.h"
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
#include <type_traits>

using namespace VW::LEARNER;
using namespace VW::config;

namespace VW
{
namespace reductions
{
namespace eigen_memory_tree
{
////////////////////////////definitions/////////////////////
////////////////////////////////////////////////////////////

emt_scorer_type emt_scorer_type_from_string(VW::string_view val)
{
  if (val == "random") { return emt_scorer_type::RANDOM; }

  if (val == "distance") { return emt_scorer_type::DISTANCE; }

  if (val == "self_consistent_rank") { return emt_scorer_type::SELF_CONSISTENT_RANK; }

  if (val == "not_self_consistent_rank") { return emt_scorer_type::NOT_SELF_CONSISTENT_RANK; }

  THROW(fmt::format("{} is not valid emt_scorer_type", val));
}

emt_router_type emt_router_type_from_string(VW::string_view val)
{
  if (val == "random") { return emt_router_type::RANDOM; }

  if (val == "eigen") { return emt_router_type::EIGEN; }

  THROW(fmt::format("{} is not valid emt_router_type", val));
}

emt_initial_type emt_initial_type_from_string(VW::string_view val)
{
  if (val == "gaussian") { return emt_initial_type::GAUSSIAN; }

  if (val == "cosine") { return emt_initial_type::COSINE; }

  if (val == "euclidean") { return emt_initial_type::EUCLIDEAN; }

  if (val == "none") { return emt_initial_type::NONE; }

  THROW(fmt::format("{} is not valid emt_initial_type", val));
}

float emt_initial(emt_initial_type initial_type, emt_feats f1, emt_feats f2)
{
  if (initial_type == emt_initial_type::GAUSSIAN) { return 1 - std::exp(-emt_norm(emt_scale_add(1, f1, -1, f2))); }

  if (initial_type == emt_initial_type::COSINE)
  {
    auto den = (emt_norm(f1) * emt_norm(f2));
    if (den != 0) { return 1 - emt_inner(f1, f2) / den; }
    else
    {
      // cosine distance isn't defined for vectors of size 0 so
      // we default to a gaussian loss as a fallback distance
      return 1 - std::exp(-emt_norm(emt_scale_add(1, f1, -1, f2)));
    }
  }

  if (initial_type == emt_initial_type::EUCLIDEAN) { return emt_norm(emt_scale_add(1, f1, -1, f2)); }

  return 0;
}

emt_example::emt_example(VW::workspace& all, VW::example* ex)
{
  label = ex->l.multi.label;

  std::vector<std::vector<VW::namespace_index>>* full_interactions = ex->interactions;
  std::vector<std::vector<VW::namespace_index>> base_interactions;

  ex->interactions = &base_interactions;
  VW::features fs;
  VW::flatten_features(all, *ex, fs);
  for (auto& f : fs) { base.emplace_back(f.index(), f.value()); }

  fs.clear();
  ex->interactions = full_interactions;
  VW::flatten_features(all, *ex, fs);
  for (auto& f : fs) { full.emplace_back(f.index(), f.value()); }
}

emt_lru::emt_lru(uint64_t max_size) : max_size(max_size) {}

emt_lru::K emt_lru::bound(emt_lru::K item)
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
  return nullptr;
}

emt_tree::emt_tree(VW::workspace* all, std::shared_ptr<VW::rand_state> random_state, uint32_t leaf_split,
    emt_scorer_type scorer_type, emt_router_type router_type, emt_initial_type initial_type, uint64_t tree_bound)
    : all(all)
    , random_state(std::move(random_state))
    , leaf_split(leaf_split)
    , scorer_type(scorer_type)
    , router_type(router_type)
    , initial_type(initial_type)
{
  bounder = VW::make_unique<VW::reductions::eigen_memory_tree::emt_lru>(tree_bound);
  root = VW::make_unique<emt_node>();

  // we set this up for repeated use later in the scorer.
  // we will populate this examples features over and over.
  ex = VW::make_unique<VW::example>();
  empty_interactions_for_ex = VW::make_unique<std::vector<std::vector<VW::namespace_index>>>();
  empty_extent_interactions_for_ex = VW::make_unique<std::vector<std::vector<extent_term>>>();
  ex->interactions = empty_interactions_for_ex.get();
  ex->extent_interactions = empty_extent_interactions_for_ex.get();
  ex->indices.push_back(0);
}
////////////////////////end of definitions/////////////////
///////////////////////////////////////////////////////////

///////////////////////Helper/////////////////////////////
//////////////////////////////////////////////////////////
float emt_median(std::vector<float>& array)
{
  const auto size = array.size();
  const auto nth = array.begin() + size / 2;

  if (size % 2 == 0)
  {
    std::nth_element(array.begin(), nth, array.end());
    const auto v1 = (*nth);
    std::nth_element(array.begin(), nth - 1, array.end());
    const auto v2 = *(nth - 1);
    return (v1 + v2) / 2.;
  }

  std::nth_element(array.begin(), nth, array.end());
  return *nth;
}

float emt_inner(const emt_feats& f1, const emt_feats& f2)
{
  float sum = 0;
  auto iter1 = f1.begin();
  auto iter2 = f2.begin();

  while (iter1 != f1.end() && iter2 != f2.end())
  {
    if (iter1->first < iter2->first) { iter1++; }
    else if (iter2->first < iter1->first) { iter2++; }
    else
    {
      sum += iter1->second * iter2->second;
      iter1++;
      iter2++;
    }
  }

  return sum;
}

float emt_norm(const emt_feats& xs)
{
  float sum_weights_sq = 0;

  for (const auto& x : xs) { sum_weights_sq += x.second * x.second; }

  return std::sqrt(sum_weights_sq);
}

void emt_scale(emt_feats& xs, float scalar)
{
  for (auto& x : xs) { x.second *= scalar; }
}

void emt_normalize(emt_feats& xs) { emt_scale(xs, 1 / emt_norm(xs)); }

emt_feats emt_sub(const emt_feats& f1, const emt_feats& f2)
{
  emt_feats out;
  auto iter1 = f1.begin();
  auto iter2 = f2.begin();

  while (iter1 != f1.end() && iter2 != f2.end())
  {
    if (iter1->first < iter2->first)
    {
      out.emplace_back(iter1->first, iter1->second);
      iter1++;
    }
    else if (iter2->first < iter1->first)
    {
      out.emplace_back(iter2->first, -iter2->second);
      iter2++;
    }
    else
    {
      out.emplace_back(iter1->first, iter1->second - iter2->second);
      iter1++;
      iter2++;
    }
  }

  while (iter1 != f1.end())
  {
    out.emplace_back(iter1->first, iter1->second);
    iter1++;
  }

  while (iter2 != f2.end())
  {
    out.emplace_back(iter2->first, -iter2->second);
    iter2++;
  }

  return out;
}

emt_feats emt_scale_add(const emt_feats& f1, float s2, const emt_feats& f2)
{
  if (s2 == -1) { return emt_sub(f1, f2); }

  emt_feats out;
  auto iter1 = f1.begin();
  auto iter2 = f2.begin();

  while (iter1 != f1.end() && iter2 != f2.end())
  {
    if (iter1->first < iter2->first)
    {
      out.emplace_back(iter1->first, iter1->second);
      iter1++;
    }
    else if (iter2->first < iter1->first)
    {
      out.emplace_back(iter2->first, s2 * iter2->second);
      iter2++;
    }
    else
    {
      out.emplace_back(iter1->first, iter1->second + s2 * iter2->second);
      iter1++;
      iter2++;
    }
  }

  while (iter1 != f1.end())
  {
    out.emplace_back(iter1->first, iter1->second);
    iter1++;
  }

  while (iter2 != f2.end())
  {
    out.emplace_back(iter2->first, s2 * iter2->second);
    iter2++;
  }

  return out;
}

void emt_scale_add2(emt_feats& f1, float s2, const emt_feats& f2)
{
  auto iter1 = f1.begin();
  auto iter2 = f2.begin();

  while (iter1 != f1.end() && iter2 != f2.end())
  {
    if (iter1->first < iter2->first) { iter1++; }
    else
    {
      iter1->second = iter1->second + s2 * iter2->second;
      iter1++;
      iter2++;
    }
  }
}

emt_feats emt_scale_add(float s1, const emt_feats& f1, float s2, const emt_feats& f2)
{
  if (s1 == 1) { return emt_scale_add(f1, s2, f2); }

  emt_feats out;
  auto iter1 = f1.begin();
  auto iter2 = f2.begin();

  while (iter1 != f1.end() && iter2 != f2.end())
  {
    if (iter1->first < iter2->first)
    {
      out.emplace_back(iter1->first, s1 * iter1->second);
      iter1++;
    }
    else if (iter2->first < iter1->first)
    {
      out.emplace_back(iter2->first, s2 * iter2->second);
      iter2++;
    }
    else
    {
      out.emplace_back(iter1->first, s1 * iter1->second + s2 * iter2->second);
      iter1++;
      iter2++;
    }
  }

  while (iter1 != f1.end())
  {
    out.emplace_back(iter1->first, s1 * iter1->second);
    iter1++;
  }

  while (iter2 != f2.end())
  {
    out.emplace_back(iter2->first, s2 * iter2->second);
    iter2++;
  }

  return out;
}

emt_feats emt_router_random(std::vector<emt_feats>& exs, VW::rand_state& rng)
{
  std::set<int> is;
  emt_feats weights;

  for (auto& e : exs)
  {
    for (auto& f : e) { is.insert(f.first); }
  }
  for (const auto& i : is) { weights.emplace_back(i, rng.get_and_update_random()); }

  emt_normalize(weights);

  return weights;
}

emt_feats emt_router_eigen(std::vector<emt_feats>& exs, VW::rand_state& rng)
{
  // for more on the method below please see:
  //    Balsubramani, Akshay, Sanjoy Dasgupta, and Yoav Freund. "The fast convergence of
  //    incremental PCA." Advances in neural information processing systems 26 (2013).

  auto weights = emt_router_random(exs, rng);

  std::map<int, float> sums;
  emt_feats means;

  for (auto& ex : exs)
  {
    for (auto& p : ex) { sums[p.first] += p.second; }
  }
  for (auto& p : sums) { means.emplace_back(p.first, p.second / exs.size()); }

  std::vector<emt_feats> centered_exs;
  for (auto& e : exs) { centered_exs.push_back(emt_sub(e, means)); }

  int n_epochs = 40;  // the bigger the better eigen approximation
  int j = 0;

  for (int i = 0; i < n_epochs; i++)
  {
    // reseting n on each epoch gives
    // projectors that are substantially
    // closer to the true top eigen vector
    // in experiments
    int n = 1;
    emt_shuffle(centered_exs.begin(), centered_exs.end(), rng);
    for (const emt_feats& fs : centered_exs)
    {
      // in matrix multiplication notation this looks like:
      // weights = weights + (1/n) * inner(outer(fs,fs), weights)
      //         = weights + (1/n) * fs * inner(fs,weights)
      //         =          weights+(1/n)*inner(fs,weights)*fs
      j++;
      emt_scale_add2(weights, emt_inner(fs, weights) / n, fs);
      if (j % 4 == 0) { emt_normalize(weights); }
      n++;
    }
  }
  emt_normalize(weights);
  return weights;
}
}  // namespace eigen_memory_tree
}  // namespace reductions
}  // namespace VW
namespace
{
using namespace VW::reductions::eigen_memory_tree;

emt_feats emt_router(std::vector<emt_feats>& exs, emt_router_type router_type, VW::rand_state& rng)
{
  if (router_type == emt_router_type::RANDOM) { return emt_router_random(exs, rng); }

  return emt_router_eigen(exs, rng);
}
////////////////////////////end of helper/////////////////////////
//////////////////////////////////////////////////////////////////

////////////////////////eigen_memory_tree///////////////////
////////////////////////////////////////////////////////////
emt_node* node_route(emt_node& cn, emt_example& ec)
{
  return emt_inner(ec.base, cn.router_weights) < cn.router_decision ? cn.left.get() : cn.right.get();
}

emt_node* tree_route(emt_tree& b, emt_example& ec)
{
  emt_node* cn = b.root.get();
  while (cn->left != nullptr) { cn = node_route(*cn, ec); }
  return cn;
}

void tree_bound(emt_tree& b, emt_example* ec)
{
  emt_example* to_delete = b.bounder->bound(ec);

  if (to_delete == nullptr) { return; }

  emt_node& cn = *tree_route(b, *to_delete);

  for (auto iter = cn.examples.begin(); iter != cn.examples.end(); iter++)
  {
    if (iter->get() == to_delete)
    {
      cn.examples.erase(iter);
      return;
    }
  }
}

void scorer_features_sub(const emt_feats& f1, const emt_feats& f2, VW::features& out)
{
  auto iter1 = f1.begin();
  auto iter2 = f2.begin();

  while (iter1 != f1.end() && iter2 != f2.end())
  {
    if (iter1->first < iter2->first)
    {
      if (iter1->second != 0) { out.push_back(iter1->second, iter1->first); }
      iter1++;
    }
    else if (iter2->first < iter1->first)
    {
      if (iter2->second != 0) { out.push_back(iter2->second, iter2->first); }
      iter2++;
    }
    else
    {
      if (iter1->second != iter2->second) { out.push_back(std::abs(iter1->second - iter2->second), iter1->first); }
      iter1++;
      iter2++;
    }
  }

  while (iter1 != f1.end())
  {
    if (iter1->second != 0) { out.push_back(std::abs(iter1->second), iter1->first); }
    iter1++;
  }

  while (iter2 != f2.end())
  {
    if (iter2->second != 0) { out.push_back(std::abs(iter2->second), iter2->first); }
    iter2++;
  }
}

void scorer_features_mul(const emt_feats& f1, const emt_feats& f2, VW::features& out)
{
  auto iter1 = f1.begin();
  auto iter2 = f2.begin();

  while (iter1 != f1.end() && iter2 != f2.end())
  {
    if (iter1->first < iter2->first)
    {
      iter1++;
    }
    else if (iter2->first < iter1->first)
    {
      iter2++;
    }
    else
    {
      out.push_back(iter1->second*iter2->second, iter1->first);
      iter1++;
      iter2++;
    }
  }
}

void scorer_example(emt_tree& b, const emt_example& ex1, const emt_example& ex2)
{
  VW::example& out = *b.ex;

  static constexpr VW::namespace_index X_NS = 'x';

  out.feature_space[X_NS].clear();

  if (b.scorer_type == emt_scorer_type::SELF_CONSISTENT_RANK)
  {
    out.indices.clear();
    out.indices.push_back(X_NS);

    out.interactions->clear();

    scorer_features_sub(ex1.full, ex2.full, out.feature_space[X_NS]);

    out.total_sum_feat_sq = out.feature_space[X_NS].sum_feat_sq;
    out.num_features = out.feature_space[X_NS].size();

    auto initial = emt_initial(b.initial_type, ex1.full, ex2.full);
    out.ex_reduction_features.get<VW::simple_label_reduction_features>().initial = initial;
  }

  if (b.scorer_type == emt_scorer_type::NOT_SELF_CONSISTENT_RANK)
  {
    out.indices.clear();
    out.indices.push_back(X_NS);

    out.interactions->clear();

    scorer_features_mul(ex1.full, ex2.full, out.feature_space[X_NS]);

    out.total_sum_feat_sq = out.feature_space[X_NS].sum_feat_sq;
    out.num_features = out.feature_space[X_NS].size();

    auto initial = emt_initial(b.initial_type, ex1.full, ex2.full);
    out.ex_reduction_features.get<VW::simple_label_reduction_features>().initial = initial;
  }

  // We cache metadata about model weights adjacent to them. For example if we have
  // a model weight w[i] then we may also store information about our confidence in
  // w[i] at w[i+1] and information about the scale of feature f[i] at w[i+2] and so on.
  // This variable indicates how many such meta-data places we need to save in between actual weights.
  uint64_t floats_per_feature_index = static_cast<uint64_t>(b.all->reduction_state.total_feature_width)
      << b.all->weights.stride_shift();

  // In both of the example_types above we construct our scorer_example from flat_examples. The VW routine
  // which creates flat_examples removes the floats_per_feature_index from the when flattening. Therefore,
  // we need to manually add it back to make sure our base learner doesn't overwrite our features/weights
  // with metadata.
  if (floats_per_feature_index != 1)
  {
    for (VW::features& fs : out)
    {
      for (auto& j : fs.indices) { j *= floats_per_feature_index; }
    }
  }
}

float scorer_predict(emt_tree& b, learner& base, const emt_example& pred_ex, const emt_example& leaf_ex)
{
  if (b.scorer_type == emt_scorer_type::RANDOM)  // random scorer
  {
    return b.random_state->get_and_update_random();
  }

  if (b.scorer_type == emt_scorer_type::DISTANCE)  // dist scorer
  {
    return emt_initial(b.initial_type, pred_ex.full, leaf_ex.full);
  }

  // The features matched exactly. Return max negative to make sure it is picked.
  // Do I want this here? It doesn't seem to matter on experimental datasets.
  if (pred_ex.full == leaf_ex.full) { return -FLT_MAX; }

  scorer_example(b, pred_ex, leaf_ex);
  b.ex->l.simple = {FLT_MAX};
  base.predict(*b.ex);

  return b.ex->pred.scalar;
}

void scorer_learn(learner& base, VW::example& ex, float label, float weight)
{
  if (ex.total_sum_feat_sq != 0)
  {
    ex.pred.scalar = 0;
    ex.l.simple = {label};
    ex.weight = weight;
    base.learn(ex);
  }
}

void scorer_learn(emt_tree& b, learner& base, emt_node& cn, const emt_example& ex, float weight)
{
  // random and dist scorer has nothing to learn
  if (b.scorer_type == emt_scorer_type::RANDOM || b.scorer_type == emt_scorer_type::DISTANCE) { return; }

  if (weight == 0) { return; }
  if (cn.examples.size() < 2) { return; }

  // shuffle the examples to break ties randomly
  emt_shuffle(cn.examples.begin(), cn.examples.end(), *b.random_state);

  float preferred_score = FLT_MAX;
  float preferred_error = FLT_MAX;
  emt_example* preferred_ex = nullptr;

  float alternative_score = FLT_MAX;
  float alternative_error = FLT_MAX;
  emt_example* alternative_ex = nullptr;

  std::vector<float> scores;
  for (auto& example : cn.examples) { scores.push_back(scorer_predict(b, base, ex, *example)); }

  // double loop has time complexity of 2n which is almost always faster than a sort with n*log(n)
  for (size_t i = 0; i < cn.examples.size(); i++)
  {
    if (scores[i] < preferred_score)
    {
      preferred_score = scores[i];
      preferred_ex = cn.examples[i].get();
      preferred_error = (preferred_ex->label == ex.label) ? 0.f : 1.f;
    }
  }

  for (size_t i = 0; i < cn.examples.size(); i++)
  {
    if (cn.examples[i].get() == preferred_ex) { continue; }
    float error = (cn.examples[i].get()->label == ex.label) ? 0.f : 1.f;

    if ((error < alternative_error) || (error == alternative_error && scores[i] < alternative_score))
    {
      alternative_score = scores[i];
      alternative_ex = cn.examples[i].get();
      alternative_error = error;
    }
  }

  // the better of the two options moves towards 0 while the other moves towards 1
  weight *= std::abs(preferred_error - alternative_error);

  if (alternative_ex == nullptr || preferred_ex == nullptr)
  {
    *(b.all->output_runtime.trace_message) << "ERROR" << std::endl;
    return;
  }

  if (weight != 0)
  {
    if (b.random_state->get_and_update_random() < .5)
    {
      scorer_example(b, ex, *preferred_ex);
      scorer_learn(base, *b.ex, int(preferred_error > alternative_error), weight);

      scorer_example(b, ex, *alternative_ex);
      scorer_learn(base, *b.ex, int(alternative_error > preferred_error), weight);
    }
    else
    {
      scorer_example(b, ex, *alternative_ex);
      scorer_learn(base, *b.ex, int(alternative_error > preferred_error), weight);

      scorer_example(b, ex, *preferred_ex);
      scorer_learn(base, *b.ex, int(preferred_error > alternative_error), weight);
    }
  }
}

void node_split(emt_tree& b, emt_node& cn)
{
  if (cn.examples.size() <= b.leaf_split) { return; }

  std::vector<emt_feats> exs;
  for (auto& ex : cn.examples) { exs.push_back(ex->base); }

  cn.left = VW::make_unique<emt_node>();
  cn.right = VW::make_unique<emt_node>();
  cn.router_weights = emt_router(exs, b.router_type, *b.random_state);

  std::vector<float> projs;
  projs.reserve(exs.size());
  for (auto& ex : exs) { projs.push_back(emt_inner(ex, cn.router_weights)); }

  cn.router_decision = emt_median(projs);

  for (auto& ex : cn.examples) { node_route(cn, *ex)->examples.push_back(std::move(ex)); }
  cn.examples.clear();
}

void node_insert(emt_node& cn, std::unique_ptr<emt_example> ex)
{
  for (auto& cn_ex : cn.examples)
  {
    if (cn_ex->full == ex->full) { return; }
  }
  cn.examples.push_back(std::move(ex));
}

emt_example* node_pick(emt_tree& b, learner& base, emt_node& cn, const emt_example& ex)
{
  if (cn.examples.empty()) { return nullptr; }

  float best_score = FLT_MAX;
  emt_example* best_example = cn.examples[0].get();

  // shuffle the examples to break ties randomly
  emt_shuffle(cn.examples.begin(), cn.examples.end(), *b.random_state);

  for (auto const& example : cn.examples)
  {
    float score = scorer_predict(b, base, ex, *example);

    if (score < best_score)
    {
      best_score = score;
      best_example = example.get();
    }
  }

  return best_example;
}

void node_predict(emt_tree& b, learner& base, emt_node& cn, emt_example& ex, VW::example& ec)
{
  auto* closest_ex = node_pick(b, base, cn, ex);
  ec.pred.multiclass = (closest_ex != nullptr) ? closest_ex->label : 0;
  ec.loss = (ec.l.multi.label != ec.pred.multiclass) ? ec.weight : 0;
}

void emt_predict(emt_tree& b, learner& base, VW::example& ec)
{
  b.all->feature_tweaks_config.ignore_some_linear = false;
  emt_example ex(*b.all, &ec);

  emt_node& cn = *tree_route(b, ex);
  node_predict(b, base, cn, ex, ec);
  tree_bound(b, &ex);
}

void emt_learn(emt_tree& b, learner& base, VW::example& ec)
{
  b.all->feature_tweaks_config.ignore_some_linear = false;
  auto ex = VW::make_unique<emt_example>(*b.all, &ec);

  emt_node& cn = *tree_route(b, *ex);
  scorer_learn(b, base, cn, *ex, ec.weight);
  node_predict(b, base, cn, *ex, ec);  // vw learners predict and emt_learn
  tree_bound(b, ex.get());
  node_insert(cn, std::move(ex));
  node_split(b, cn);
}

#ifdef VW_ENABLE_EMT_DEBUG_TIMER
void emt_end_pass_timer(emt_tree& b)
{
  *(b.all->output_runtime.trace_message) << "##### pass_time: "
                                         << static_cast<float>(clock() - b.begin) / CLOCKS_PER_SEC << std::endl;

  b.begin = clock();
}
#endif

}  // namespace
/////////////////end of eigen_memory_tree///////////////////
////////////////////////////////////////////////////////////

///////////////////Save & Load//////////////////////////////////////
////////////////////////////////////////////////////////////////////
namespace VW
{
namespace model_utils
{

size_t read_model_field(io_buf& io, reductions::eigen_memory_tree::emt_example& ex)
{
  size_t bytes = 0;
  bytes += read_model_field(io, ex.base);
  bytes += read_model_field(io, ex.full);
  bytes += read_model_field(io, ex.label);
  return bytes;
}
size_t write_model_field(
    io_buf& io, const reductions::eigen_memory_tree::emt_example& ex, const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  bytes += write_model_field(io, ex.base, upstream_name + ".base", text);
  bytes += write_model_field(io, ex.full, upstream_name + ".full", text);
  bytes += write_model_field(io, ex.label, upstream_name + ".label", text);
  return bytes;
}

size_t read_model_field(io_buf& io, reductions::eigen_memory_tree::emt_node& node)
{
  size_t bytes = 0;
  bytes += read_model_field(io, node.router_decision);
  bytes += read_model_field(io, node.left);
  bytes += read_model_field(io, node.right);
  bytes += read_model_field(io, node.router_weights);
  bytes += read_model_field(io, node.examples);
  return bytes;
}

size_t write_model_field(
    io_buf& io, const reductions::eigen_memory_tree::emt_node& node, const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  bytes += write_model_field(io, node.router_decision, upstream_name + ".router_decision", text);
  bytes += write_model_field(io, node.left, upstream_name + ".left", text);
  bytes += write_model_field(io, node.right, upstream_name + ".right", text);
  bytes += write_model_field(io, node.router_weights, upstream_name + ".router_weights", text);
  bytes += write_model_field(io, node.examples, upstream_name + ".examples", text);
  return bytes;
}

size_t read_model_field(io_buf& io, reductions::eigen_memory_tree::emt_tree& tree)
{
  size_t bytes = 0;

  bytes += read_model_field(io, tree.leaf_split);

  uint32_t scorer_type{};
  bytes += read_model_field(io, scorer_type);
  tree.scorer_type = static_cast<reductions::eigen_memory_tree::emt_scorer_type>(scorer_type);

  uint32_t router_type{};
  bytes += read_model_field(io, router_type);
  tree.router_type = static_cast<reductions::eigen_memory_tree::emt_router_type>(router_type);

  uint64_t tree_bound{};
  bytes += read_model_field(io, tree_bound);
  tree.bounder = VW::make_unique<reductions::eigen_memory_tree::emt_lru>(tree_bound);

  bytes += read_model_field(io, tree.root);

  return bytes;
}

size_t write_model_field(
    io_buf& io, const reductions::eigen_memory_tree::emt_tree& tree, const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  bytes += write_model_field(io, tree.leaf_split, upstream_name + ".leaf_split", text);
  bytes += write_model_field(io, static_cast<uint32_t>(tree.scorer_type), upstream_name + ".scorer_type", text);
  bytes += write_model_field(io, static_cast<uint32_t>(tree.router_type), upstream_name + ".router_type", text);
  bytes += write_model_field(io, tree.bounder->max_size, upstream_name + ".tree_bound", text);
  bytes += write_model_field(io, tree.root, upstream_name + ".root", text);
  return bytes;
}

}  // namespace model_utils
}  // namespace VW

namespace
{
void emt_save_load_tree(VW::reductions::eigen_memory_tree::emt_tree& tree, VW::io_buf& io, bool read, bool text)
{
  if (io.num_files() == 0) { return; }
  if (read) { VW::model_utils::read_model_field(io, tree); }
  else { VW::model_utils::write_model_field(io, tree, "emt", text); }
}
}  // namespace
/////////////////End of Save & Load/////////////////////////////////
////////////////////////////////////////////////////////////////////

std::shared_ptr<VW::LEARNER::learner> VW::reductions::eigen_memory_tree_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();

  bool enabled{};
  std::string scorer_type;
  std::string router_type;
  std::string initial_type;
  uint32_t tree_bound = 0;
  uint32_t leaf_split = 0;

  option_group_definition new_options("[Reduction] Eigen Memory Tree");
  new_options.add(make_option("emt", enabled).keep().necessary().help("Make an eigen memory tree"))
      .add(make_option("emt_tree", tree_bound)
               .keep()
               .default_value(0)
               .help("Indicates the maximum number of memories the tree can have"))
      .add(make_option("emt_leaf", leaf_split)
               .keep()
               .default_value(100)
               .help("Indicates the maximum number of memories a leaf can have"))
      .add(make_option("emt_scorer", scorer_type)
               .keep()
               .one_of({"random", "distance", "self_consistent_rank", "not_self_consistent_rank"})
               .default_value("self_consistent_rank")
               .help("Indicates the type of scorer to use"))
      .add(make_option("emt_initial", initial_type)
               .keep()
               .one_of({"gaussian", "cosine", "euclidean", "none"})
               .default_value("cosine")
               .help("Indicates how to initialize scorer"))
      .add(make_option("emt_router", router_type)
               .keep()
               .one_of({"random", "eigen"})
               .default_value("eigen")
               .help("Indicates the type of router to use"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  // The EMT scorer, when it is learning, will only ever see 0's and 1's.
  // Therefore, VW's native min/max will be 0/1 unless we explicitly make it larger.
  if (all.sd->min_label == 0 && all.sd->max_label == 0)
  {
    all.sd->min_label = 0;
    all.sd->max_label = 3;
  }

  auto t = VW::make_unique<VW::reductions::eigen_memory_tree::emt_tree>(&all, all.get_random_state(), leaf_split,
      emt_scorer_type_from_string(scorer_type), emt_router_type_from_string(router_type),
      emt_initial_type_from_string(initial_type), tree_bound);

  auto l =
      make_reduction_learner(std::move(t), require_singleline(stack_builder.setup_base_learner()), emt_learn,
          emt_predict,
          stack_builder.get_setupfn_name(eigen_memory_tree_setup))
          .set_learn_returns_prediction(true)  // we set this to true otherwise bounding doesn't work as well
#ifdef VW_ENABLE_EMT_DEBUG_TIMER
          .set_end_pass(emt_end_pass_timer)
#endif
          .set_save_load(emt_save_load_tree)
          .set_input_prediction_type(VW::prediction_type_t::SCALAR)
          .set_output_prediction_type(VW::prediction_type_t::MULTICLASS)
          .set_input_label_type(VW::label_type_t::MULTICLASS)
          .set_output_label_type(VW::label_type_t::SIMPLE)
          .set_update_stats(VW::details::update_stats_multiclass_label<VW::reductions::eigen_memory_tree::emt_tree>)
          .set_output_example_prediction(
              VW::details::output_example_prediction_multiclass_label<VW::reductions::eigen_memory_tree::emt_tree>)
          .set_print_update(VW::details::print_update_multiclass_label<VW::reductions::eigen_memory_tree::emt_tree>);
  return l.build();
}
