// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/cb/cb_explore_adf_rnd.h"

#include "vw/config/options.h"
#include "vw/core/debug_print.h"
#include "vw/core/gd_predict.h"
#include "vw/core/gen_cs_example.h"
#include "vw/core/label_parser.h"
#include "vw/core/rand48.h"
#include "vw/core/reductions/bs.h"
#include "vw/core/reductions/cb/cb_adf.h"
#include "vw/core/reductions/cb/cb_explore.h"
#include "vw/core/scope_exit.h"
#include "vw/core/setup_base.h"
#include "vw/explore/explore.h"

#include <algorithm>
#include <cfloat>
#include <cmath>

// Random Network Distillation style exploration.  Basically predicts
// something whose true expectation is zero and uses the MSE(prediction
// error) as a confidence interval.
//
// Predicting a synthetic Gaussian target is equivalent to a randomized range
// finder on the pseudoinverse and the prediction error concentrates around
// the Fisher information matrix quadratic form of linucb.  So essentially
// this is a randomized algorithm for approximating the linucb bonus.
// Hopefully it works well when the expected reward is realizable.  YMMV.

using namespace VW::LEARNER;
using namespace VW::cb_explore_adf;

namespace
{
struct cb_explore_adf_rnd
{
private:
  float epsilon;
  float alpha;
  float sqrtinvlambda;
  uint32_t numrnd;

  size_t increment;
  VW::workspace* all;

  std::vector<float> bonuses;
  std::vector<float> initials;

  CB::cb_class save_class;

  template <bool is_learn>
  void predict_or_learn_impl(multi_learner& base, VW::multi_ex& examples);

  float get_initial_prediction(VW::example*);
  void get_initial_predictions(VW::multi_ex&, uint32_t);
  void zero_bonuses(VW::multi_ex&);
  void accumulate_bonuses(VW::multi_ex&);
  void finish_bonuses();
  void compute_ci(v_array<ACTION_SCORE::action_score>&, float);

  template <bool>
  void save_labels(VW::multi_ex&);
  template <bool>
  void make_fake_rnd_labels(VW::multi_ex&);
  template <bool>
  void restore_labels(VW::multi_ex&);
  template <bool>
  void base_learn_or_predict(multi_learner&, VW::multi_ex&, uint32_t);

public:
  cb_explore_adf_rnd(
      float _epsilon, float _alpha, float _invlambda, uint32_t _numrnd, size_t _increment, VW::workspace* _all)
      : epsilon(_epsilon)
      , alpha(_alpha)
      , sqrtinvlambda(std::sqrt(_invlambda))
      , numrnd(_numrnd)
      , increment(_increment)
      , all(_all)
  {
  }
  ~cb_explore_adf_rnd() = default;

  // Should be called through cb_explore_adf_base for pre/post-processing
  void predict(multi_learner& base, VW::multi_ex& examples) { predict_or_learn_impl<false>(base, examples); }
  void learn(multi_learner& base, VW::multi_ex& examples) { predict_or_learn_impl<true>(base, examples); }
};

void cb_explore_adf_rnd::zero_bonuses(VW::multi_ex& examples) { bonuses.assign(examples.size(), 0.f); }

void cb_explore_adf_rnd::accumulate_bonuses(VW::multi_ex& examples)
{
  const auto& preds = examples[0]->pred.a_s;
  for (const auto& p : preds)
  {
    float score = p.score - initials[p.action];
    bonuses[p.action] += score * score;
  }
}

void cb_explore_adf_rnd::finish_bonuses()
{
  for (auto& b : bonuses) { b = std::sqrt(b / numrnd); }
}

void cb_explore_adf_rnd::compute_ci(v_array<ACTION_SCORE::action_score>& preds, float max_bonus)
{
  constexpr float eulergamma = 0.57721566490153286f;
  for (auto& p : preds) { p.score -= eulergamma * (bonuses[p.action] - max_bonus); }
}

namespace
{
bool is_the_labeled_example(const VW::example* ec)
{
  return (ec->l.cb.costs.size() == 1 && ec->l.cb.costs[0].cost != FLT_MAX && ec->l.cb.costs[0].probability > 0);
}
}  // namespace

template <bool is_learn>
void cb_explore_adf_rnd::save_labels(VW::multi_ex& examples)
{
  if (is_learn)
  {
    for (const auto* ec : examples)
    {
      if (is_the_labeled_example(ec))
      {
        save_class.cost = ec->l.cb.costs[0].cost;
        save_class.probability = ec->l.cb.costs[0].probability;
        break;
      }
    }
  }
}

namespace
{
struct LazyGaussian
{
  inline float operator[](uint64_t index) const { return merand48_boxmuller(index); }
};

inline void vec_add_with_norm(std::pair<float, float>& p, float fx, float fw)
{
  p.first += fx * fx;
  p.second += fx * fw;
}

}  // namespace

float cb_explore_adf_rnd::get_initial_prediction(VW::example* ec)
{
  LazyGaussian w;

  std::pair<float, float> dotwithnorm(0.f, 0.f);
  GD::foreach_feature<std::pair<float, float>, float, vec_add_with_norm, LazyGaussian>(w, all->ignore_some_linear,
      all->ignore_linear, all->interactions, all->extent_interactions, all->permutations, *ec, dotwithnorm,
      all->_generate_interactions_object_cache);

  return sqrtinvlambda * dotwithnorm.second / std::sqrt(2.0f * std::max(1e-12f, dotwithnorm.first));
}

void cb_explore_adf_rnd::get_initial_predictions(VW::multi_ex& examples, uint32_t id)
{
  initials.clear();
  initials.reserve(examples.size());
  for (size_t i = 0; i < examples.size(); ++i)
  {
    auto* ec = examples[i];

    LEARNER::details::increment_offset(*ec, increment, id);
    initials.push_back(get_initial_prediction(ec));
    LEARNER::details::decrement_offset(*ec, increment, id);
  }
}

template <bool is_learn>
void cb_explore_adf_rnd::make_fake_rnd_labels(VW::multi_ex& examples)
{
  if (is_learn)
  {
    for (size_t i = 0; i < examples.size(); ++i)
    {
      auto* ec = examples[i];
      if (is_the_labeled_example(ec))
      {
        ec->l.cb.costs[0].cost = alpha * all->get_random_state()->get_and_update_gaussian() + initials[i];
        ec->l.cb.costs[0].probability = 1.0f;
        break;
      }
    }
  }
}

template <bool is_learn>
void cb_explore_adf_rnd::restore_labels(VW::multi_ex& examples)
{
  if (is_learn)
  {
    for (auto* ec : examples)
    {
      if (is_the_labeled_example(ec))
      {
        ec->l.cb.costs[0].cost = save_class.cost;
        ec->l.cb.costs[0].probability = save_class.probability;
        break;
      }
    }
  }
}

template <bool is_learn>
void cb_explore_adf_rnd::base_learn_or_predict(multi_learner& base, VW::multi_ex& examples, uint32_t id)
{
  if (is_learn) { base.learn(examples, id); }
  else
  {
    base.predict(examples, id);
  }
}

template <bool is_learn>
void cb_explore_adf_rnd::predict_or_learn_impl(multi_learner& base, VW::multi_ex& examples)
{
  save_labels<is_learn>(examples);

  // Guard example state restore against throws
  auto restore_guard = VW::scope_exit([this, &examples] { this->restore_labels<is_learn>(examples); });

  zero_bonuses(examples);
  for (uint32_t id = 0; id < numrnd; ++id)
  {
    get_initial_predictions(examples, 1 + id);
    make_fake_rnd_labels<is_learn>(examples);
    base_learn_or_predict<is_learn>(base, examples, 1 + id);
    accumulate_bonuses(examples);
  }
  finish_bonuses();

  // Labels need to be restored before calling base_learn_or_predict
  restore_guard.call();
  base_learn_or_predict<is_learn>(base, examples, 0);

  auto& preds = examples[0]->pred.a_s;
  float max_bonus = std::max(1e-3f, *std::max_element(bonuses.begin(), bonuses.end()));
  compute_ci(preds, max_bonus);
  exploration::generate_softmax(
      -1.0f / max_bonus, begin_scores(preds), end_scores(preds), begin_scores(preds), end_scores(preds));
  exploration::enforce_minimum_probability(epsilon, true, begin_scores(preds), end_scores(preds));
}
}  // namespace

VW::LEARNER::base_learner* VW::reductions::cb_explore_adf_rnd_setup(VW::setup_base_i& stack_builder)
{
  VW::config::options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();
  using config::make_option;
  bool cb_explore_adf_option = false;
  float epsilon = 0.;
  float alpha = 0.;
  float invlambda = 0.;
  uint32_t numrnd = 1;
  config::option_group_definition new_options("[Reduction] Contextual Bandit Exploration with ADF (rnd)");
  new_options
      .add(make_option("cb_explore_adf", cb_explore_adf_option)
               .keep()
               .necessary()
               .help("Online explore-exploit for a contextual bandit problem with multiline action dependent features"))
      .add(make_option("epsilon", epsilon)
               .keep()
               .default_value(0.f)
               .allow_override()
               .help("Minimum exploration probability"))
      .add(make_option("rnd", numrnd).keep().default_value(1).necessary().help("Rnd based exploration"))
      .add(make_option("rnd_alpha", alpha)
               .keep()
               .allow_override()
               .default_value(0.1f)
               .help("CI width for rnd (bigger => more exploration on repeating features)"))
      .add(make_option("rnd_invlambda", invlambda)
               .keep()
               .allow_override()
               .default_value(0.1f)
               .help("Covariance regularization strength rnd (bigger => more exploration on new features)"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  if (alpha <= 0) { THROW("The value of rnd_alpha must be positive.") }

  if (invlambda <= 0) { THROW("The value of rnd_invlambda must be positive.") }

  if (numrnd < 1) { THROW("The value of numrnd must be at least 1.") }

  // Ensure serialization of cb_adf in all cases.
  if (!options.was_supplied("cb_adf")) { options.insert("cb_adf", ""); }

  size_t problem_multiplier = 1 + numrnd;

  multi_learner* base = as_multiline(stack_builder.setup_base_learner());
  all.example_parser->lbl_parser = CB::cb_label;

  bool with_metrics = options.was_supplied("extra_metrics");

  using explore_type = cb_explore_adf_base<cb_explore_adf_rnd>;
  auto data = VW::make_unique<explore_type>(
      with_metrics, epsilon, alpha, invlambda, numrnd, base->increment * problem_multiplier, &all);

  if (epsilon < 0.0 || epsilon > 1.0) { THROW("The value of epsilon must be in [0,1]"); }
  auto* l = make_reduction_learner(std::move(data), base, explore_type::learn, explore_type::predict,
      stack_builder.get_setupfn_name(cb_explore_adf_rnd_setup))
                .set_input_label_type(VW::label_type_t::cb)
                .set_output_label_type(VW::label_type_t::cb)
                .set_input_prediction_type(VW::prediction_type_t::action_scores)
                .set_output_prediction_type(VW::prediction_type_t::action_probs)
                .set_params_per_weight(problem_multiplier)
                .set_finish_example(explore_type::finish_multiline_example)
                .set_print_example(explore_type::print_multiline_example)
                .set_persist_metrics(explore_type::persist_metrics)
                .build(&all.logger);
  return make_base(*l);
}
