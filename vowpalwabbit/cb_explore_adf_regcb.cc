// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "cb_explore_adf_regcb.h"
#include "reductions.h"
#include "cb_adf.h"
#include "rand48.h"
#include "bs.h"
#include "gen_cs_example.h"
#include "cb_explore.h"
#include "explore.h"
#include "action_score.h"
#include "cb.h"
#include <vector>
#include <algorithm>
#include <cmath>

// All exploration algorithms return a vector of id, probability tuples, sorted in order of scores. The probabilities
// are the probability with which each action should be replaced to the top of the list.

#define B_SEARCH_MAX_ITER 20

namespace VW
{
namespace cb_explore_adf
{
namespace regcb
{
struct cb_explore_adf_regcb
{
 private:
  size_t _counter;
  bool _regcbopt;  // use optimistic variant of RegCB
  float _c0;       // mellowness parameter for RegCB
  bool _first_only;
  float _min_cb_cost;
  float _max_cb_cost;

  std::vector<float> _min_costs;
  std::vector<float> _max_costs;

  // for backing up cb example data when computing sensitivities
  std::vector<ACTION_SCORE::action_scores> _ex_as;
  std::vector<v_array<CB::cb_class>> _ex_costs;

 public:
  cb_explore_adf_regcb(bool regcbopt, float c0, bool first_only, float min_cb_cost, float max_cb_cost);
  ~cb_explore_adf_regcb() = default;

  // Should be called through cb_explore_adf_base for pre/post-processing
  void predict(LEARNER::multi_learner& base, multi_ex& examples) { predict_or_learn_impl<false>(base, examples); }
  void learn(LEARNER::multi_learner& base, multi_ex& examples) { predict_or_learn_impl<true>(base, examples); }

 private:
  template <bool is_learn>
  void predict_or_learn_impl(LEARNER::multi_learner& base, multi_ex& examples);

  void get_cost_ranges(float delta, LEARNER::multi_learner& base, multi_ex& examples, bool min_only);
  float binary_search(float fhat, float delta, float sens, float tol = 1e-6);
};

cb_explore_adf_regcb::cb_explore_adf_regcb(
    bool regcbopt, float c0, bool first_only, float min_cb_cost, float max_cb_cost)
    : _regcbopt(regcbopt), _c0(c0), _first_only(first_only), _min_cb_cost(min_cb_cost), _max_cb_cost(max_cb_cost)
{
}

// TODO: same as cs_active.cc, move to shared place
float cb_explore_adf_regcb::binary_search(float fhat, float delta, float sens, float tol)
{
  const float maxw = (std::min)(fhat / sens, FLT_MAX);

  if (maxw * fhat * fhat <= delta)
    return maxw;

  float l = 0;
  float u = maxw;
  float w, v;

  for (int iter = 0; iter < B_SEARCH_MAX_ITER; iter++)
  {
    w = (u + l) / 2.f;
    v = w * (fhat * fhat - (fhat - sens * w) * (fhat - sens * w)) - delta;
    if (v > 0)
      u = w;
    else
      l = w;
    if (fabs(v) <= tol || u - l <= tol)
      break;
  }

  return l;
}

void cb_explore_adf_regcb::get_cost_ranges(float delta, LEARNER::multi_learner& base, multi_ex& examples, bool min_only)
{
  const size_t num_actions = examples[0]->pred.a_s.size();
  _min_costs.resize(num_actions);
  _max_costs.resize(num_actions);

  _ex_as.clear();
  _ex_costs.clear();

  // backup cb example data
  for (const auto& ex : examples)
  {
    _ex_as.push_back(ex->pred.a_s);
    _ex_costs.push_back(ex->l.cb.costs);
  }

  // set regressor predictions
  for (const auto& as : _ex_as[0])
  {
    examples[as.action]->pred.scalar = as.score;
  }

  const float cmin = _min_cb_cost;
  const float cmax = _max_cb_cost;

  for (size_t a = 0; a < num_actions; ++a)
  {
    example* ec = examples[a];
    ec->l.simple.label = cmin - 1;
    float sens = base.sensitivity(*ec);
    float w = 0;  // importance weight

    if (ec->pred.scalar < cmin || std::isnan(sens) || std::isinf(sens))
      _min_costs[a] = cmin;
    else
    {
      w = binary_search(ec->pred.scalar - cmin + 1, delta, sens);
      _min_costs[a] = (std::max)(ec->pred.scalar - sens * w, cmin);
      if (_min_costs[a] > cmax)
        _min_costs[a] = cmax;
    }

    if (!min_only)
    {
      ec->l.simple.label = cmax + 1;
      sens = base.sensitivity(*ec);
      if (ec->pred.scalar > cmax || std::isnan(sens) || std::isinf(sens))
      {
        _max_costs[a] = cmax;
      }
      else
      {
        w = binary_search(cmax + 1 - ec->pred.scalar, delta, sens);
        _max_costs[a] = (std::min)(ec->pred.scalar + sens * w, cmax);
        if (_max_costs[a] < cmin)
          _max_costs[a] = cmin;
      }
    }
  }

  // reset cb example data
  for (size_t i = 0; i < examples.size(); ++i)
  {
    examples[i]->pred.a_s = _ex_as[i];
    examples[i]->l.cb.costs = _ex_costs[i];
  }
}

template <bool is_learn>
void cb_explore_adf_regcb::predict_or_learn_impl(LEARNER::multi_learner& base, multi_ex& examples)
{
  if (is_learn)
  {
    for (size_t i = 0; i < examples.size() - 1; ++i)
    {
      CB::label& ld = examples[i]->l.cb;
      if (ld.costs.size() == 1)
        ld.costs[0].probability = 1.f;  // no importance weighting
    }

    LEARNER::multiline_learn_or_predict<true>(base, examples, examples[0]->ft_offset);
    ++_counter;
  }
  else
    LEARNER::multiline_learn_or_predict<false>(base, examples, examples[0]->ft_offset);

  v_array<ACTION_SCORE::action_score>& preds = examples[0]->pred.a_s;
  uint32_t num_actions = (uint32_t)preds.size();

  const float max_range = _max_cb_cost - _min_cb_cost;
  // threshold on empirical loss difference
  const float delta = _c0 * log((float)(num_actions * _counter)) * pow(max_range, 2);

  if (!is_learn)
  {
    get_cost_ranges(delta, base, examples, /*min_only=*/_regcbopt);

    if (_regcbopt)  // optimistic variant
    {
      float min_cost = FLT_MAX;
      size_t a_opt = 0;  // optimistic action
      for (size_t a = 0; a < num_actions; ++a)
      {
        if (_min_costs[a] < min_cost)
        {
          min_cost = _min_costs[a];
          a_opt = a;
        }
      }
      for (size_t i = 0; i < preds.size(); ++i)
      {
        if (preds[i].action == a_opt || (!_first_only && _min_costs[preds[i].action] == min_cost))
          preds[i].score = 1;
        else
          preds[i].score = 0;
      }
    }
    else  // elimination variant
    {
      float min_max_cost = FLT_MAX;
      for (size_t a = 0; a < num_actions; ++a)
        if (_max_costs[a] < min_max_cost)
          min_max_cost = _max_costs[a];
      for (size_t i = 0; i < preds.size(); ++i)
      {
        if (_min_costs[preds[i].action] <= min_max_cost)
          preds[i].score = 1;
        else
          preds[i].score = 0;
        // explore uniformly on support
        exploration::enforce_minimum_probability(
            1.0, /*update_zero_elements=*/false, begin_scores(preds), end_scores(preds));
      }
    }
  }
}

LEARNER::base_learner* setup(VW::config::options_i& options, vw& all)
{
  using config::make_option;
  bool cb_explore_adf_option = false;
  bool regcb = false;
  const std::string mtr = "mtr";
  std::string type_string(mtr);
  bool regcbopt = false;
  float c0 = 0.;
  bool first_only = false;
  float min_cb_cost = 0.;
  float max_cb_cost = 0.;
  config::option_group_definition new_options("Contextual Bandit Exploration with Action Dependent Features");
  new_options
      .add(make_option("cb_explore_adf", cb_explore_adf_option)
               .keep()
               .help("Online explore-exploit for a contextual bandit problem with multiline action dependent features"))
      .add(make_option("regcb", regcb).keep().help("RegCB-elim exploration"))
      .add(make_option("regcbopt", regcbopt).keep().help("RegCB optimistic exploration"))
      .add(make_option("mellowness", c0).keep().default_value(0.1f).help("RegCB mellowness parameter c_0. Default 0.1"))
      .add(make_option("cb_min_cost", min_cb_cost).keep().default_value(0.f).help("lower bound on cost"))
      .add(make_option("cb_max_cost", max_cb_cost).keep().default_value(1.f).help("upper bound on cost"))
      .add(make_option("first_only", first_only).keep().help("Only explore the first action in a tie-breaking event"))
      .add(make_option("cb_type", type_string)
               .keep()
               .help("contextual bandit method to use in {ips,dr,mtr}. Default: mtr"));
  options.add_and_parse(new_options);

  if (!cb_explore_adf_option || !(options.was_supplied("regcb") || options.was_supplied("regcbopt")))
    return nullptr;

  // Ensure serialization of cb_adf in all cases.
  if (!options.was_supplied("cb_adf"))
  {
    options.insert("cb_adf", "");
  }
  if (type_string != mtr)
  {
    all.trace_message << "warning: bad cb_type, RegCB only supports mtr; resetting to mtr." << std::endl;
    options.replace("cb_type", mtr);
  }

  all.delete_prediction = ACTION_SCORE::delete_action_scores;

  // Set explore_type
  size_t problem_multiplier = 1;

  LEARNER::multi_learner* base = as_multiline(setup_base(options, all));
  all.p->lp = CB::cb_label;
  all.label_type = label_type_t::cb;

  using explore_type = cb_explore_adf_base<cb_explore_adf_regcb>;
  auto data = scoped_calloc_or_throw<explore_type>(regcbopt, c0, first_only, min_cb_cost, max_cb_cost);
  LEARNER::learner<explore_type, multi_ex>& l = LEARNER::init_learner(
      data, base, explore_type::learn, explore_type::predict, problem_multiplier, prediction_type_t::action_probs);

  l.set_finish_example(explore_type::finish_multiline_example);
  return make_base(l);
}

}  // namespace regcb
}  // namespace cb_explore_adf
}  // namespace VW
