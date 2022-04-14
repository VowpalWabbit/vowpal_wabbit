// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/core/reductions/baseline.h"
#include "vw/core/cb.h"
#include "vw/core/cb_type.h"
#include "vw/core/example.h"
#include "vw/core/guard.h"
#include "vw/core/learner.h"

#include <cfloat>

namespace VW
{
namespace reductions
{
// TODO: extend to handle CSOAA_LDF and WAP_LDF
VW::LEARNER::base_learner* cb_algs_setup(VW::setup_base_i& stack_builder);
}  // namespace reductions
}  // namespace VW

// TODO: Move these functions either into a CB-related lib in VW:: or under VW::reductions::
namespace CB_ALGS
{
template <bool is_learn>
float get_cost_pred(
    VW::LEARNER::single_learner* scorer, const CB::cb_class& known_cost, VW::example& ec, uint32_t index, uint32_t base)
{
  VW_DBG(ec) << "get_cost_pred:" << is_learn << std::endl;

  label_data simple_temp;
  if (index == known_cost.action)
    simple_temp.label = known_cost.cost;
  else
    simple_temp.label = FLT_MAX;

  const bool baseline_enabled_old = VW::reductions::baseline::baseline_enabled(&ec);
  VW::reductions::baseline::set_baseline_enabled(&ec);
  ec.l.simple = simple_temp;
  bool learn = is_learn && index == known_cost.action;

  if (learn)
  {
    float old_weight = ec.weight;
    ec.weight /= known_cost.probability;
    scorer->learn(ec, index - 1 + base);
    ec.weight = old_weight;
  }
  else
    scorer->predict(ec, index - 1 + base);

  if (!baseline_enabled_old) VW::reductions::baseline::reset_baseline_disabled(&ec);
  float pred = ec.pred.scalar;
  return pred;
}

// IPS estimate
inline float get_cost_estimate(const CB::cb_class& observation, uint32_t action, float offset = 0.)
{
  if (action == observation.action) return (observation.cost - offset) / observation.probability;
  return 0.;
}

// doubly robust estimate
inline float get_cost_estimate(const CB::cb_class& observation, const COST_SENSITIVE::label& scores, uint32_t action)
{
  for (auto& cl : scores.costs)
    if (cl.class_index == action) return get_cost_estimate(observation, action, cl.x) + cl.x;
  // defaults to IPS when there are no scores
  return get_cost_estimate(observation, action);
}

// IPS
inline float get_cost_estimate(const CB::label& ld, uint32_t action)
{
  for (auto& cl : ld.costs)
    if (cl.action == action) return get_cost_estimate(cl, action);
  return 0.0f;
}

// doubly robust estimate
inline float get_cost_estimate(const ACTION_SCORE::action_score& a_s, float cost, uint32_t action, float offset = 0.)
{
  if (action == a_s.action) return (cost - offset) / a_s.score;
  return 0.;
}

inline bool example_is_newline_not_header(VW::example const& ec)
{
  return (VW::example_is_newline(ec) && !CB::ec_is_example_header(ec));
}

void generic_output_example(
    VW::workspace& all, float loss, const VW::example& ec, const CB::label& ld, const CB::cb_class* known_cost);

}  // namespace CB_ALGS
