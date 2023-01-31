// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/cb/cb_explore_adf_rnd.h"

#include "vw/config/options.h"
#include "vw/core/debug_print.h"
#include "vw/core/gd_predict.h"
#include "vw/core/gen_cs_example.h"
#include "vw/core/global_data.h"
#include "vw/core/label_parser.h"
#include "vw/core/parser.h"
#include "vw/core/reductions/bs.h"
#include "vw/core/reductions/cb/cb_adf.h"
#include "vw/core/reductions/cb/cb_explore.h"
#include "vw/core/reductions/cb/cb_explore_adf_common.h"
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
class cb_explore_adf_rnd
{
public:
  cb_explore_adf_rnd(
      float _epsilon, float _alpha, float _invlambda, uint32_t _numrnd, size_t _increment, VW::workspace* _all)
      : _epsilon(_epsilon)
      , _alpha(_alpha)
      , _sqrtinvlambda(std::sqrt(_invlambda))
      , _numrnd(_numrnd)
      , _increment(_increment)
      , _all(_all)
  {
  }
  ~cb_explore_adf_rnd() = default;

  // Should be called through cb_explore_adf_base for pre/post-processing
  void predict(learner& base, VW::multi_ex& examples) { predict_or_learn_impl<false>(base, examples); }
  void learn(learner& base, VW::multi_ex& examples) { predict_or_learn_impl<true>(base, examples); }

private:
  float _epsilon;
  float _alpha;
  float _sqrtinvlambda;
  uint32_t _numrnd;

  size_t _increment;
  VW::workspace* _all;

  std::vector<float> _bonuses;
  std::vector<float> _initials;

  VW::cb_class _save_class;

  template <bool is_learn>
  void predict_or_learn_impl(learner& base, VW::multi_ex& examples);

  float get_initial_prediction(VW::example*);
  void get_initial_predictions(VW::multi_ex&, uint32_t);
  void zero_bonuses(VW::multi_ex&);
  void accumulate_bonuses(VW::multi_ex&);
  void finish_bonuses();
  void compute_ci(VW::v_array<VW::action_score>&, float);

  template <bool>
  void save_labels(VW::multi_ex&);
  template <bool>
  void make_fake_rnd_labels(VW::multi_ex&);
  template <bool>
  void restore_labels(VW::multi_ex&);
  template <bool>
  void base_learn_or_predict(learner&, VW::multi_ex&, uint32_t);
};

void cb_explore_adf_rnd::zero_bonuses(VW::multi_ex& examples) { _bonuses.assign(examples.size(), 0.f); }

void cb_explore_adf_rnd::accumulate_bonuses(VW::multi_ex& examples)
{
  const auto& preds = examples[0]->pred.a_s;
  for (const auto& p : preds)
  {
    float score = p.score - _initials[p.action];
    _bonuses[p.action] += score * score;
  }
}

void cb_explore_adf_rnd::finish_bonuses()
{
  for (auto& b : _bonuses) { b = std::sqrt(b / _numrnd); }
}

void cb_explore_adf_rnd::compute_ci(VW::v_array<VW::action_score>& preds, float max_bonus)
{
  constexpr float eulergamma = 0.57721566490153286f;
  for (auto& p : preds) { p.score -= eulergamma * (_bonuses[p.action] - max_bonus); }
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
        _save_class.cost = ec->l.cb.costs[0].cost;
        _save_class.probability = ec->l.cb.costs[0].probability;
        break;
      }
    }
  }
}

namespace
{
class lazy_gaussian
{
public:
  inline float operator[](uint64_t index) const { return VW::details::merand48_boxmuller(index); }
};

inline void vec_add_with_norm(std::pair<float, float>& p, float fx, float fw)
{
  p.first += fx * fx;
  p.second += fx * fw;
}

}  // namespace

float cb_explore_adf_rnd::get_initial_prediction(VW::example* ec)
{
  lazy_gaussian w;

  std::pair<float, float> dotwithnorm(0.f, 0.f);
  VW::foreach_feature<std::pair<float, float>, float, vec_add_with_norm, lazy_gaussian>(w, _all->ignore_some_linear,
      _all->ignore_linear, _all->interactions, _all->extent_interactions, _all->permutations, *ec, dotwithnorm,
      _all->generate_interactions_object_cache_state);

  return _sqrtinvlambda * dotwithnorm.second / std::sqrt(2.0f * std::max(1e-12f, dotwithnorm.first));
}

void cb_explore_adf_rnd::get_initial_predictions(VW::multi_ex& examples, uint32_t id)
{
  _initials.clear();
  _initials.reserve(examples.size());
  for (size_t i = 0; i < examples.size(); ++i)
  {
    auto* ec = examples[i];

    VW::LEARNER::details::increment_offset(*ec, _increment, id);
    _initials.push_back(get_initial_prediction(ec));
    VW::LEARNER::details::decrement_offset(*ec, _increment, id);
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
        ec->l.cb.costs[0].cost = _alpha * _all->get_random_state()->get_and_update_gaussian() + _initials[i];
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
        ec->l.cb.costs[0].cost = _save_class.cost;
        ec->l.cb.costs[0].probability = _save_class.probability;
        break;
      }
    }
  }
}

template <bool is_learn>
void cb_explore_adf_rnd::base_learn_or_predict(learner& base, VW::multi_ex& examples, uint32_t id)
{
  if (is_learn) { base.learn(examples, id); }
  else { base.predict(examples, id); }
}

template <bool is_learn>
void cb_explore_adf_rnd::predict_or_learn_impl(learner& base, VW::multi_ex& examples)
{
  save_labels<is_learn>(examples);

  // Guard example state restore against throws
  auto restore_guard = VW::scope_exit([this, &examples] { this->restore_labels<is_learn>(examples); });

  zero_bonuses(examples);
  for (uint32_t id = 0; id < _numrnd; ++id)
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
  float max_bonus = std::max(1e-3f, *std::max_element(_bonuses.begin(), _bonuses.end()));
  compute_ci(preds, max_bonus);
  VW::explore::generate_softmax(
      -1.0f / max_bonus, begin_scores(preds), end_scores(preds), begin_scores(preds), end_scores(preds));
  VW::explore::enforce_minimum_probability(_epsilon, true, begin_scores(preds), end_scores(preds));
}
}  // namespace

std::shared_ptr<VW::LEARNER::learner> VW::reductions::cb_explore_adf_rnd_setup(VW::setup_base_i& stack_builder)
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

  auto base = require_multiline(stack_builder.setup_base_learner());
  all.example_parser->lbl_parser = VW::cb_label_parser_global;

  using explore_type = cb_explore_adf_base<cb_explore_adf_rnd>;
  auto data = VW::make_unique<explore_type>(all.global_metrics.are_metrics_enabled(), epsilon, alpha, invlambda, numrnd,
      base->increment * problem_multiplier, &all);

  if (epsilon < 0.0 || epsilon > 1.0) { THROW("The value of epsilon must be in [0,1]"); }
  auto l = make_reduction_learner(std::move(data), base, explore_type::learn, explore_type::predict,
      stack_builder.get_setupfn_name(cb_explore_adf_rnd_setup))
               .set_input_label_type(VW::label_type_t::CB)
               .set_output_label_type(VW::label_type_t::CB)
               .set_input_prediction_type(VW::prediction_type_t::ACTION_SCORES)
               .set_output_prediction_type(VW::prediction_type_t::ACTION_PROBS)
               .set_params_per_weight(problem_multiplier)
               .set_output_example_prediction(explore_type::output_example_prediction)
               .set_update_stats(explore_type::update_stats)
               .set_print_update(explore_type::print_update)
               .set_persist_metrics(explore_type::persist_metrics)
               .build();
  return l;
}
