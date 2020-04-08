// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "cb_explore_adf_rnd.h"
#include "reductions.h"
#include "cb_adf.h"
#include "rand48.h"
#include "bs.h"
#include "gen_cs_example.h"
#include "cb_explore.h"
#include "explore.h"
#include <vector>
#include <algorithm>
#include <cmath>

namespace VW
{
namespace cb_explore_adf
{
namespace rnd
{
struct cb_explore_adf_rnd
{
 private:
  float epsilon;
  float alpha;
  float invlambda;
  int numrnd;
  vw* all;

 public:
  cb_explore_adf_rnd(float _epsilon, float _alpha, float _invlambda, int _numrnd, vw* _all) : epsilon(_epsilon), alpha(_alpha), invlambda(_invlambda), numrnd(_numrnd), all(_all) { }
  ~cb_explore_adf_rnd() = default;

  // Should be called through cb_explore_adf_base for pre/post-processing
  void predict(LEARNER::multi_learner& base, multi_ex& examples) { predict_or_learn_impl<false>(base, examples); }
  void learn(LEARNER::multi_learner& base, multi_ex& examples) { predict_or_learn_impl<true>(base, examples); }

 private:
  template <bool is_learn>
  void predict_or_learn_impl(LEARNER::multi_learner& base, multi_ex& examples);

  std::vector<float> bonuses;
  void zero_bonuses(multi_ex& examples);
  void accumulate_bonuses(multi_ex& examples);
  void finish_bonuses();
  void compute_ci(v_array<ACTION_SCORE::action_score>& preds);

  std::vector<CB::label> cblabels;
  template<bool> void save_labels(multi_ex&);
  template<bool> void make_fake_rnd_labels(multi_ex&);
  template<bool> void restore_labels(multi_ex&);
  template<bool> void base_learn_or_predict(LEARNER::multi_learner&, multi_ex&, uint32_t);
};

void cb_explore_adf_rnd::zero_bonuses(multi_ex& examples)
{
  bonuses.reserve(examples.size());
  for (auto& b: bonuses) { b = 0.f; }
}

void cb_explore_adf_rnd::accumulate_bonuses(multi_ex& examples)
{
  v_array<ACTION_SCORE::action_score>& preds = examples[0]->pred.a_s;
  for (const auto& p : preds) { bonuses[p.action] += p.score * p.score; }
}

void cb_explore_adf_rnd::finish_bonuses()
{
  for (auto& b: bonuses) { b = sqrt(b / numrnd) * invlambda; }
}

void cb_explore_adf_rnd::compute_ci(v_array<ACTION_SCORE::action_score>& preds)
{
  for (auto& p: preds) { p.score -= bonuses[p.action]; }
}

template <bool is_learn>
void cb_explore_adf_rnd::save_labels(multi_ex& examples)
{
  if (is_learn)
    {
      cblabels.reserve(examples.size());
      for (size_t i = 0; i < examples.size(); ++i)
        {
          cblabels[i].weight = examples[i]->l.cb.weight;
          copy_array(cblabels[i].costs, examples[i]->l.cb.costs);
        }
    }
}

template <bool is_learn>
void cb_explore_adf_rnd::make_fake_rnd_labels(multi_ex& examples)
{
  if (is_learn)
    {
      for (auto& e: examples) 
        { 
          CB::cb_class v;
          v.cost = alpha * all->get_random_state()->get_and_update_gaussian();
          v.action = e->l.cb.costs[0].action;
          v.probability = 1.0f;

          e->l.cb.costs.clear();
          e->l.cb.costs.push_back(v);
        }
    }
}

template <bool is_learn>
void cb_explore_adf_rnd::restore_labels(multi_ex& examples)
{
  if (is_learn)
    {
      for (size_t i = 0; i < examples.size(); ++i)
        {
          examples[i]->l.cb.weight = cblabels[i].weight;
          copy_array(examples[i]->l.cb.costs, cblabels[i].costs);
        }
    }
}

template <bool is_learn>
void cb_explore_adf_rnd::base_learn_or_predict(LEARNER::multi_learner& base, multi_ex& examples, uint32_t id)
{
  if (is_learn)
    {
      base.learn(examples, id);
    }
  else
    {
      base.predict(examples, id);
    }
}

template <bool is_learn>
void cb_explore_adf_rnd::predict_or_learn_impl(LEARNER::multi_learner& base, multi_ex& examples)
{
  // NB: pretend learn does *not* return a prediction
  //     since this is the future desired contract
  
  save_labels<is_learn>(examples);
  zero_bonuses(examples);
  for (int i = 0; i < numrnd; ++i)
    {
      make_fake_rnd_labels<is_learn>(examples);
      base_learn_or_predict<is_learn>(base, examples, 1 + i);
      accumulate_bonuses(examples);
    }
  finish_bonuses();
  restore_labels<is_learn>(examples);
  base_learn_or_predict<is_learn>(base, examples, 0);
  v_array<ACTION_SCORE::action_score>& preds = examples[0]->pred.a_s;
  compute_ci(preds);
  exploration::generate_softmax(
    -1.f, ACTION_SCORE::begin_scores(preds), ACTION_SCORE::end_scores(preds), ACTION_SCORE::begin_scores(preds), ACTION_SCORE::end_scores(preds));
  exploration::enforce_minimum_probability(epsilon, true, ACTION_SCORE::begin_scores(preds), ACTION_SCORE::end_scores(preds));
}

LEARNER::base_learner* setup(VW::config::options_i& options, vw& all)
{
  using config::make_option;
  bool cb_explore_adf_option = false;
  bool rnd = false;
  float epsilon = 0.;
  float alpha = 0.;
  float invlambda = 0.;
  int numrnd = 1;
  config::option_group_definition new_options("Contextual Bandit Exploration with Action Dependent Features");
  new_options
      .add(make_option("cb_explore_adf", cb_explore_adf_option)
               .keep()
               .help("Online explore-exploit for a contextual bandit problem with multiline action dependent features"))
      .add(make_option("epsilon", epsilon).keep().allow_override().help("minimum exploration probability"))
      .add(make_option("rnd", rnd).keep().help("rnd exploration"))
      .add(make_option("alpha", alpha).keep().allow_override().default_value(0.1f).help("ci width for rnd (bigger => more exploration on repeating features)"))
      .add(make_option("invlambda", invlambda).keep().allow_override().default_value(0.1f).help("covariance regularization strength rnd (bigger => more exploration on new features)"))
      .add(make_option("numrnd", numrnd).keep().allow_override().default_value(1).help("number of rnd predictors (bigger => better but slower)"));
  options.add_and_parse(new_options);

  if (!cb_explore_adf_option || !rnd)
    return nullptr;

  if (alpha < 0)
  {
    THROW("The value of alpha must be positive.")
  }

  if (invlambda < 0)
  {
    THROW("The value of invlambda must be positive.")
  }

  if (numrnd < 1)
  {
    THROW("The value of numrnd must be at least 1.")
  }

  // Ensure serialization of cb_adf in all cases.
  if (!options.was_supplied("cb_adf"))
  {
    options.insert("cb_adf", "");
  }

  all.delete_prediction = ACTION_SCORE::delete_action_scores;

  // ensure truncated_normal_weights
  if (!options.was_supplied("truncated_normal_weights"))
    {
      options.insert("truncated_normal_weights", std::to_string(true));
    }

  size_t problem_multiplier = 1 + numrnd;

  LEARNER::multi_learner* base = as_multiline(setup_base(options, all));
  all.p->lp = CB::cb_label;
  all.label_type = label_type_t::cb;

  using explore_type = cb_explore_adf_base<cb_explore_adf_rnd>;
  auto data = scoped_calloc_or_throw<explore_type>(epsilon, alpha, invlambda, numrnd, &all);

  if (epsilon < 0.0 || epsilon > 1.0)
  {
    THROW("The value of epsilon must be in [0,1]");
  }
  
  LEARNER::learner<explore_type, multi_ex>& l = LEARNER::init_learner(
      data, base, explore_type::learn, explore_type::predict, problem_multiplier, prediction_type_t::action_probs);

  l.set_finish_example(explore_type::finish_multiline_example);
  return make_base(l);
}
}  // namespace rnd
}  // namespace cb_explore_adf
}  // namespace VW
