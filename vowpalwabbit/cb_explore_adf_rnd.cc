// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "cb_explore_adf_rnd.h"
#include "reductions.h"
#include "cb_adf.h"
#include "rand48.h"
#include "bs.h"
#include "gd_predict.h"
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

  size_t increment;
  vw* all;

 public:
  cb_explore_adf_rnd(float _epsilon, float _alpha, float _invlambda, int _numrnd, size_t _increment, vw* _all) : epsilon(_epsilon), alpha(_alpha), invlambda(_invlambda), numrnd(_numrnd), increment(_increment), all(_all) { }
  ~cb_explore_adf_rnd() = default;

  // Should be called through cb_explore_adf_base for pre/post-processing
  void predict(LEARNER::multi_learner& base, multi_ex& examples) { predict_or_learn_impl<false>(base, examples); }
  void learn(LEARNER::multi_learner& base, multi_ex& examples) { predict_or_learn_impl<true>(base, examples); }

 private:
  template <bool is_learn>
  void predict_or_learn_impl(LEARNER::multi_learner& base, multi_ex& examples);

  std::vector<float> bonuses;
  void zero_bonuses(multi_ex&);
  void accumulate_bonuses(multi_ex&);
  void finish_bonuses();
  void compute_ci(v_array<ACTION_SCORE::action_score>&, float);

  CB::cb_class save_class;
  template<bool> void save_labels(multi_ex&);
  float get_prediction_offset(example*);
  template<bool> void make_fake_rnd_labels(multi_ex&);
  template<bool> void restore_labels(multi_ex&);
  template<bool> void base_learn_or_predict(LEARNER::multi_learner&, multi_ex&, uint32_t);
};

void cb_explore_adf_rnd::zero_bonuses(multi_ex& examples)
{
  bonuses.clear();
  bonuses.reserve(examples.size());
  for (auto* ec: examples) { (void) ec; bonuses.push_back(0.f); }
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

void cb_explore_adf_rnd::compute_ci(v_array<ACTION_SCORE::action_score>& preds,
                                    float maxbonus)
{
  const float eulergamma = 0.57721566490153286;
  for (auto& p: preds) { p.score -= eulergamma * (bonuses[p.action] - maxbonus); }
}

namespace
{
  bool is_the_labeled_example(const example* ec)
    {
      return (   ec->l.cb.costs.size() == 1
              && ec->l.cb.costs[0].cost != FLT_MAX
              && ec->l.cb.costs[0].probability > 0);
    }
}

template <bool is_learn>
void cb_explore_adf_rnd::save_labels(multi_ex& examples)
{
  if (is_learn)
    {
      for (const auto* ec: examples)
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
  class LazyGaussianWeight
    {
      public:
        float operator[](uint64_t index) const
          {
            return merand48_boxmuller(index);
          }
    };
}

float cb_explore_adf_rnd::get_prediction_offset(example* ec)
{
  LazyGaussianWeight w;
  return GD::inline_predict(w,
                            all->ignore_some_linear,
                            all->ignore_linear,
                            all->interactions,
                            all->permutations,
                            *ec);
}

template <bool is_learn>
void cb_explore_adf_rnd::make_fake_rnd_labels(multi_ex& examples)
{
  if (is_learn)
    {
      for (size_t i = 0; i < examples.size(); ++i)
        {
          auto* ec = examples[i];
          if (is_the_labeled_example(ec))
            {
              LEARNER::increment_offset(*ec, increment, 1 + i);
              float initial = get_prediction_offset(ec);
              LEARNER::decrement_offset(*ec, increment, 1 + i);

              ec->l.cb.costs[0].cost = alpha * (all->get_random_state()->get_and_update_gaussian() + initial);
              ec->l.cb.costs[0].probability = 1.0f;
              break;
            }
        }
    }
}

template <bool is_learn>
void cb_explore_adf_rnd::restore_labels(multi_ex& examples)
{
  if (is_learn)
    {
      for (auto* ec: examples)
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
  float maxbonus = std::max(1e-3f, *std::max_element(bonuses.begin(), bonuses.end()));
  compute_ci(preds, maxbonus);
  exploration::generate_softmax(
    -1.0/maxbonus, begin_scores(preds), end_scores(preds), begin_scores(preds), end_scores(preds));
  exploration::enforce_minimum_probability(epsilon, true, begin_scores(preds), end_scores(preds));
}

LEARNER::base_learner* setup(VW::config::options_i& options, vw& all)
{
  using config::make_option;
  bool cb_explore_adf_option = false;
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
      .add(make_option("rnd", numrnd).keep().help("rnd based exploration"))
      .add(make_option("alpha", alpha).keep().allow_override().default_value(0.0125f).help("ci width for rnd (bigger => more exploration on repeating features)"))
      .add(make_option("invlambda", invlambda).keep().allow_override().default_value(0.125f).help("covariance regularization strength rnd (bigger => more exploration on new features)"));
  options.add_and_parse(new_options);

  if (!cb_explore_adf_option || !options.was_supplied("rnd"))
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

  size_t problem_multiplier = 1 + numrnd;

  LEARNER::multi_learner* base = as_multiline(setup_base(options, all));
  all.p->lp = CB::cb_label;
  all.label_type = label_type_t::cb;

  using explore_type = cb_explore_adf_base<cb_explore_adf_rnd>;
  auto data = scoped_calloc_or_throw<explore_type>(epsilon, alpha, invlambda, numrnd, base->increment * problem_multiplier, &all);

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
