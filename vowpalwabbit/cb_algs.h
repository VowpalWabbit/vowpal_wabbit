// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "baseline.h"

// TODO: extend to handle CSOAA_LDF and WAP_LDF
LEARNER::base_learner* cb_algs_setup(VW::config::options_i& options, vw& all);

#define CB_TYPE_DR 0
#define CB_TYPE_DM 1
#define CB_TYPE_IPS 2
#define CB_TYPE_MTR 3
#define CB_TYPE_SM 4

namespace CB_ALGS
{
template <bool is_learn>
float get_cost_pred(
    LEARNER::single_learner* scorer, CB::cb_class* known_cost, example& ec, uint32_t index, uint32_t base)
{
  CB::label ld = ec.l.cb;

  label_data simple_temp;
  simple_temp.initial = 0.;
  if (known_cost != nullptr && index == known_cost->action)
    simple_temp.label = known_cost->cost;
  else
    simple_temp.label = FLT_MAX;

  const bool baseline_enabled_old = BASELINE::baseline_enabled(&ec);
  BASELINE::set_baseline_enabled(&ec);
  ec.l.simple = simple_temp;
  polyprediction p = ec.pred;
  if (is_learn && known_cost != nullptr && index == known_cost->action)
  {
    float old_weight = ec.weight;
    ec.weight /= known_cost->probability;
    scorer->learn(ec, index - 1 + base);
    ec.weight = old_weight;
  }
  else
    scorer->predict(ec, index - 1 + base);

  if (!baseline_enabled_old)
    BASELINE::reset_baseline_disabled(&ec);
  float pred = ec.pred.scalar;
  ec.pred = p;

  ec.l.cb = ld;

  return pred;
}

inline float get_cost_estimate(CB::cb_class* observation, uint32_t action, float offset = 0.)
{
  if (action == observation->action)
    return (observation->cost - offset) / observation->probability;
  return 0.;
}

inline float get_cost_estimate(CB::cb_class* observation, COST_SENSITIVE::label& scores, uint32_t action)
{
  for (auto& cl : scores.costs)
    if (cl.class_index == action)
      return get_cost_estimate(observation, action, cl.x) + cl.x;
  return get_cost_estimate(observation, action);
}

inline float get_cost_estimate(ACTION_SCORE::action_score& a_s, float cost, uint32_t action, float offset = 0.)
{
  if (action == a_s.action)
    return (cost - offset) / a_s.score;
  return 0.;
}

inline bool example_is_newline_not_header(example const& ec)
{
  return (example_is_newline(ec) && !CB::ec_is_example_header(ec));
}
}  // namespace CB_ALGS
