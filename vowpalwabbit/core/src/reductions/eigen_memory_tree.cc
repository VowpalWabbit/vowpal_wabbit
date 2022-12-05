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
emt_example::emt_example() { label = 0; }

emt_example::emt_example(VW::workspace& all, VW::example* ex)
{
  label = ex->l.multi.label;

  std::vector<std::vector<VW::namespace_index>>* full_interactions = ex->interactions;
  std::vector<std::vector<VW::namespace_index>> base_interactions;

  ex->interactions = &base_interactions;
  auto* ex1 = VW::flatten_sort_example(all, ex);
  for (auto& f : ex1->fs) { base.emplace_back(f.index(), f.value()); }
  VW::free_flatten_example(ex1);

  ex->interactions = full_interactions;
  auto* ex2 = VW::flatten_sort_example(all, ex);
  for (auto& f : ex2->fs) { full.emplace_back(f.index(), f.value()); }
  VW::free_flatten_example(ex2);
}

emt_lru::emt_lru(unsigned long max_size) { (*this).max_size = max_size; }

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

emt_node::emt_node()
{
  left = nullptr;
  right = nullptr;
  router_decision = 0;
}

emt_tree::emt_tree()
{
  leaf_split = 100;

  scorer_type = emt_scorer_type::self_consistent_rank;
  router_type = emt_router_type::eigen;

  begin = 0;

  all = nullptr;
  ex = nullptr;
  bounder = nullptr;
  root = VW::make_unique<emt_node>();

  // we set this up for repeated use later in the scorer.
  // we will populate this examples features over and over.
  ex = VW::make_unique<VW::example>();
  ex->interactions = new std::vector<std::vector<VW::namespace_index>>();
  ex->extent_interactions = new std::vector<std::vector<extent_term>>();
  ex->indices.push_back(0);
}

emt_tree::~emt_tree()
{
  delete ex->interactions;
  delete ex->extent_interactions;
}

////////////////////////end of definitions/////////////////
///////////////////////////////////////////////////////////

///////////////////////Helper/////////////////////////////
//////////////////////////////////////////////////////////
float emt_median(std::vector<float>& array)
{
  int size = array.size();
  auto nth = array.begin() + size / 2;

  if (size % 2 == 0)
  {
    std::nth_element(array.begin(), nth, array.end());
    auto v1 = (*nth);
    std::nth_element(array.begin(), nth - 1, array.end());
    auto v2 = *(nth - 1);
    return (v1 + v2) / 2.;
  }
  else
  {
    std::nth_element(array.begin(), nth, array.end());
    return *nth;
  }
}

float emt_inner(const emt_feats& xs, const emt_feats& ys)
{
  float sum = 0;
  unsigned long xi = 0;
  unsigned long yi = 0;

  while (xi < xs.size() && yi < ys.size())
  {
    auto x = xs[xi];
    auto y = ys[yi];

    if (x.first == y.first) { sum += x.second * y.second; }

    if (x.first <= y.first) { xi += 1; }
    if (y.first <= x.first) { yi += 1; }
  }

  return sum;
}

float emt_norm(const emt_feats& xs)
{
  float sum_weights_sq = 0;

  for (auto& x : xs) { sum_weights_sq += x.second * x.second; }

  return std::sqrt(sum_weights_sq);
}

void emt_scale(emt_feats& xs, float scalar)
{
  for (auto& x : xs) { x.second *= scalar; }
}

void emt_normalize(emt_feats& xs) { emt_scale(xs, 1 / emt_norm(xs)); }

void emt_abs(emt_feats& fs)
{
  for (auto& f : fs) { f.second = std::abs(f.second); }
}

emt_feats emt_scale_add(float s1, const emt_feats& f1, float s2, const emt_feats& f2)
{
  size_t idx1 = 0;
  size_t idx2 = 0;

  uint64_t index = 0;
  uint64_t f1_idx = 0;
  uint64_t f2_idx = 0;

  float f1_val = 0.f;
  float f2_val = 0.f;

  emt_feats out;

  while (idx1 < f1.size() || idx2 < f2.size())
  {
    f1_idx = (idx1 == f1.size()) ? INT_MAX : f1[idx1].first;
    f2_idx = (idx2 == f2.size()) ? INT_MAX : f2[idx2].first;

    f1_val = 0.f;
    f2_val = 0.f;

    if (f1_idx <= f2_idx)
    {
      index = f1_idx;
      f1_val = f1[idx1].second;
      idx1++;
    }

    if (f2_idx <= f1_idx)
    {
      index = f2_idx;
      f2_val = f2[idx2].second;
      idx2++;
    }

    out.emplace_back(index, f1_val * s1 + f2_val * s2);
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
  for (auto& i : is) { weights.emplace_back(i, rng.get_and_update_random()); }

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
  for (auto& e : exs) { centered_exs.push_back(emt_scale_add(1, e, -1, means)); }

  int n_epochs = 40;  // the bigger the better eigen approximation

  for (int i = 0; i < n_epochs; i++)
  {
    // reseting n on each epoch gives
    // projectors that are substantially
    // closer to the true top eigen vector
    // in experiments
    float n = 1;
    emt_shuffle(centered_exs.begin(), centered_exs.end(), &rng);

    for (emt_feats fs : centered_exs)
    {
      // in matrix multiplication notation this looks like:
      // weights = weights + (1/n) * inner(outer(fs,fs), weights)
      //         = weights + (1/n) * fs * inner(fs,weights)
      //         =          weights+(1/n)*inner(fs,weights)*fs
      weights = emt_scale_add(1, weights, (1 / n) * emt_inner(fs, weights), fs);
      emt_normalize(weights);
      n += 1;
    }
  }

  return weights;
}

emt_feats emt_router(std::vector<emt_feats> exs, emt_router_type router_type, VW::rand_state& rng)
{
  if (router_type == emt_router_type::random) { return emt_router_random(exs, rng); }
  else { return emt_router_eigen(exs, rng); }
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

float scorer_initial(const VW::example& ex) { return 1 - std::exp(-std::sqrt(ex.total_sum_feat_sq)); }

void scorer_features(const emt_feats& f1, features& out)
{
  out.clear();
  for (auto p : f1)
  {
    if (p.second != 0) { out.push_back(p.second, p.first); }
  }
}

void scorer_example(emt_tree& b, const emt_example& ex1, const emt_example& ex2)
{
  VW::example& out = *b.ex;

  if (b.scorer_type == emt_scorer_type::self_consistent_rank)
  {
    out.indices.clear();
    out.indices.push_back('x');

    out.interactions->clear();

    out.feature_space['x'].clear();
    out.feature_space['z'].clear();

    emt_feats feat_diff = emt_scale_add(1, ex1.full, -1, ex2.full);
    emt_abs(feat_diff);
    scorer_features(feat_diff, out.feature_space['x']);

    out.total_sum_feat_sq = out.feature_space['x'].sum_feat_sq;
    out.num_features = out.feature_space['x'].size();

    out.ex_reduction_features.get<VW::simple_label_reduction_features>().initial = scorer_initial(out);
  }

  if (b.scorer_type == emt_scorer_type::not_self_consistent_rank)
  {
    out.indices.clear();
    out.indices.push_back('x');
    out.indices.push_back('z');

    out.interactions->clear();
    out.interactions->push_back({'x', 'z'});

    b.all->ignore_some_linear = true;
    b.all->ignore_linear['x'] = true;
    b.all->ignore_linear['z'] = true;

    scorer_features(ex1.full, out.feature_space['x']);
    scorer_features(ex2.full, out.feature_space['z']);

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

float scorer_predict(emt_tree& b, single_learner& base, const emt_example& pred_ex, const emt_example& leaf_ex)
{
  if (b.scorer_type == emt_scorer_type::random)  // random scorer
  {
    return b._random_state->get_and_update_random();
  }

  else if (b.scorer_type == emt_scorer_type::distance)  // dist scorer
  {
    return emt_norm(emt_scale_add(1, pred_ex.full, -1, leaf_ex.full));
  }

  else
  {
    // The features matched exactly. Return max negative to make sure it is picked.
    if (pred_ex.full == leaf_ex.full) { return -FLT_MAX; }

    scorer_example(b, pred_ex, leaf_ex);
    b.ex->l.simple = {FLT_MAX};
    base.predict(*b.ex);

    return b.ex->pred.scalar;
  }
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

void scorer_learn(emt_tree& b, single_learner& base, emt_node& cn, const emt_example& ex, float weight)
{
  // random and dist scorer has nothing to learsn
  if (b.scorer_type == emt_scorer_type::random || b.scorer_type == emt_scorer_type::distance) { return; }
  else
  {
    if (weight == 0) { return; }
    if (cn.examples.size() < 2) { return; }

    // shuffle the examples to break ties randomly
    emt_shuffle(cn.examples.begin(), cn.examples.end(), b._random_state.get());

    float preferred_score = FLT_MAX;
    float preferred_error = FLT_MAX;
    emt_example* preferred_ex = nullptr;

    float alternative_error = FLT_MAX;
    emt_example* alternative_ex = nullptr;

    for (auto& example : cn.examples)
    {
      float score = scorer_predict(b, base, ex, *example);

      if (score < preferred_score)
      {
        preferred_score = score;
        preferred_ex = example.get();
        preferred_error = (example->label == ex.label) ? 0.f : 1.f;
      }
    }

    for (auto& example : cn.examples)
    {
      if (example.get() == preferred_ex) { continue; }

      float error = (example->label == ex.label) ? 0.f : 1.f;
      if (error < alternative_error)
      {
        alternative_error = error;
        alternative_ex = example.get();
      }
    }

    // the better of the two options moves towards 0 while the other moves towards -1
    weight *= std::abs(preferred_error - alternative_error);

    if (alternative_ex == nullptr || preferred_ex == nullptr)
    {
      *(b.all->trace_message) << "ERROR" << std::endl;
      return;
    }

    if (weight != 0)
    {
      if (b._random_state->get_and_update_random() < .5)
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

      // this trick gives worse outcomes. Why? It is faster so it'd be nice if it didn't.
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

  std::vector<emt_feats> exs;
  for (auto& ex : cn.examples) { exs.push_back(ex->base); }

  cn.left = VW::make_unique<emt_node>();
  cn.right = VW::make_unique<emt_node>();
  cn.router_weights = emt_router(exs, b.router_type, *b._random_state);

  std::vector<float> projs;
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

emt_example* node_pick(emt_tree& b, single_learner& base, emt_node& cn, const emt_example& ex)
{
  if (cn.examples.size() == 0) { return nullptr; }

  float best_score = FLT_MAX;
  emt_example* best_example = cn.examples[0].get();

  // shuffle the examples to break ties randomly
  emt_shuffle(cn.examples.begin(), cn.examples.end(), b._random_state.get());

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

void node_predict(emt_tree& b, single_learner& base, emt_node& cn, emt_example& ex, VW::example& ec)
{
  auto closest_ex = node_pick(b, base, cn, ex);
  ec.pred.multiclass = (closest_ex != nullptr) ? closest_ex->label : 0;
  ec.loss = (ec.l.multi.label != ec.pred.multiclass) ? ec.weight : 0;
}

void predict(emt_tree& b, single_learner& base, VW::example& ec)
{
  b.all->ignore_some_linear = false;
  emt_example ex(*b.all, &ec);

  emt_node& cn = *tree_route(b, ex);
  node_predict(b, base, cn, ex, ec);
  tree_bound(b, &ex);
}

void learn(emt_tree& b, single_learner& base, VW::example& ec)
{
  b.all->ignore_some_linear = false;
  auto ex = VW::make_unique<emt_example>(*b.all, &ec);

  emt_node& cn = *tree_route(b, *ex);
  scorer_learn(b, base, cn, *ex, ec.weight);
  node_predict(b, base, cn, *ex, ec);  // vw learners predict and learn

  tree_bound(b, ex.get());
  node_insert(cn, std::move(ex));
  node_split(b, cn);
}

void end_pass(emt_tree& b)
{
  *(b.all->trace_message) << "##### pass_time: " << static_cast<float>(clock() - b.begin) / CLOCKS_PER_SEC << std::endl;

  b.begin = clock();
}
/////////////////end of eigen_memory_tree///////////////////
////////////////////////////////////////////////////////////

///////////////////Save & Load//////////////////////////////////////
////////////////////////////////////////////////////////////////////
void save_load_examples(emt_node& n, io_buf& model_file, bool& read, bool& text, std::stringstream& msg)
{
  WRITEITVAR(n.examples.size(), "n_examples", n_examples);

  if (read)
  {
    for (uint32_t i = 0; i < n_examples; i++) { n.examples.push_back(VW::make_unique<emt_example>()); }
  }

  for (auto& e : n.examples)
  {
    for (auto& p : e->base)
    {
      WRITEIT(p.first, "base_index");
      WRITEIT(p.second, "base_value");
    }
    for (auto& p : e->full)
    {
      WRITEIT(p.first, "full_index");
      WRITEIT(p.second, "full_value");
    }
  }
}

void save_load_weights(emt_node& n, io_buf& model_file, bool& read, bool& text, std::stringstream& msg)
{
  for (auto& p : n.router_weights)
  {
    WRITEIT(p.first, "router_index");
    WRITEIT(p.second, "router_value");
  }
}

std::unique_ptr<emt_node> save_load_node(
    emt_tree& b, std::unique_ptr<emt_node> n, io_buf& model_file, bool& read, bool& text, std::stringstream& msg)
{
  WRITEITVAR(!read && !n, "is_null", is_null);

  if (is_null) { return nullptr; }
  if (!n) { n = VW::make_unique<emt_node>(); }

  WRITEIT(n->router_decision, "decision");

  save_load_examples(*n, model_file, read, text, msg);
  save_load_weights(*n, model_file, read, text, msg);

  n->left = save_load_node(b, std::move(n->left), model_file, read, text, msg);
  n->right = save_load_node(b, std::move(n->right), model_file, read, text, msg);

  return n;
}

void save_load_tree(emt_tree& b, io_buf& model_file, bool read, bool text)
{
  std::stringstream msg;
  if (model_file.num_files() > 0)
  {
    uint32_t ss = b.all->weights.stride_shift();
    WRITEIT(ss, "stride_shift");

    b.all->weights.stride_shift(ss);

    int scorer_type = static_cast<int>(b.scorer_type);
    int router_type = static_cast<int>(b.router_type);
    uint32_t tree_bound = b.bounder->max_size;

    WRITEIT(b.leaf_split, "leaf_split");
    WRITEIT(tree_bound, "tree_bound");
    WRITEIT(scorer_type, "scorer_type");
    WRITEIT(router_type, "router_type");

    b.scorer_type = static_cast<emt_scorer_type>(scorer_type);
    b.router_type = static_cast<emt_router_type>(router_type);

    if (read) { b.bounder = VW::make_unique<emt_lru>(tree_bound); }

    b.root = save_load_node(b, std::move(b.root), model_file, read, text, msg);
    *(b.all->trace_message) << "done loading...." << std::endl;
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
  uint32_t scorer_type = 0, router_type = 0, tree_bound = 0;

  option_group_definition new_options("[Reduction] Eigen Memory Tree");
  new_options.add(make_option("eigen_memory_tree", _).keep().necessary().help("Make an eigen memory tree"))
      .add(make_option("emt_tree", tree_bound)
               .keep()
               .default_value(0)
               .help("Indicates the maximum number of memories the tree can have"))
      .add(make_option("emt_leaf", t->leaf_split)
               .keep()
               .default_value(100)
               .help("Indicates the maximum number of memories a leaf can have"))
      .add(make_option("emt_scorer", scorer_type)
               .keep()
               .one_of({1, 2, 3, 4})
               .default_value(3)
               .help("Indicates the type of scorer to use (1=random;2=distance;3=self consistent rank;4=not self "
                     "consistent rank)"))
      .add(make_option("emt_router", router_type)
               .keep()
               .one_of({1, 2})
               .default_value(2)
               .help("Indicates the type of router to use (1=random;2=eigen)"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  t->scorer_type = static_cast<VW::reductions::eigen_memory_tree::emt_scorer_type>(scorer_type);
  t->router_type = static_cast<VW::reductions::eigen_memory_tree::emt_router_type>(router_type);
  t->bounder = VW::make_unique<VW::reductions::eigen_memory_tree::emt_lru>(tree_bound);

  t->all = &all;
  t->_random_state = all.get_random_state();

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

  auto l = make_reduction_learner(std::move(t), as_singleline(stack_builder.setup_base_learner()), learn, predict,
      stack_builder.get_setupfn_name(eigen_memory_tree_setup))
               .set_learn_returns_prediction(true)  // we set this to true otherwise bounding doesn't work as well
               .set_end_pass(end_pass)
               .set_save_load(save_load)
               .set_output_prediction_type(pred_type)
               .set_input_label_type(label_type);

  l.set_finish_example(VW::details::finish_multiclass_example<VW::reductions::eigen_memory_tree::emt_tree>);

  return make_base(*l.build());
}
