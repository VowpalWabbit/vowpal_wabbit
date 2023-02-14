// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/core/cb.h"
#include "vw/core/cb_type.h"
#include "vw/core/example.h"
#include "vw/core/guard.h"
#include "vw/core/learner.h"
#include "vw/core/reductions/baseline.h"
#include "vw/core/simple_label.h"

#include <cfloat>

namespace VW
{
namespace reductions
{
// TODO: extend to handle CSOAA_LDF and WAP_LDF
std::shared_ptr<VW::LEARNER::learner> cb_algs_setup(VW::setup_base_i& stack_builder);
}  // namespace reductions
}  // namespace VW

// TODO: Move these functions either into a CB-related lib in VW:: or under VW::reductions::
namespace VW
{
template <bool is_learn>
float get_cost_pred(
    VW::LEARNER::learner* scorer, const VW::cb_class& known_cost, VW::example& ec, uint32_t index, uint32_t base)
{
  VW_DBG(ec) << "get_cost_pred:" << is_learn << std::endl;

  VW::simple_label simple_temp;
  if (index == known_cost.action) { simple_temp.label = known_cost.cost; }
  else { simple_temp.label = FLT_MAX; }

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
  else { scorer->predict(ec, index - 1 + base); }

  if (!baseline_enabled_old) { VW::reductions::baseline::reset_baseline_disabled(&ec); }
  float pred = ec.pred.scalar;
  return pred;
}

// IPS estimate
inline float get_cost_estimate(const VW::cb_class& observation, uint32_t action, float offset = 0.)
{
  if (action == observation.action) { return (observation.cost - offset) / observation.probability; }
  return 0.;
}

// doubly robust estimate
inline float get_cost_estimate(const VW::cb_class& observation, const VW::cs_label& scores, uint32_t action)
{
  for (auto& cl : scores.costs)
  {
    if (cl.class_index == action) { return get_cost_estimate(observation, action, cl.x) + cl.x; }
  }
  // defaults to IPS when there are no scores
  return get_cost_estimate(observation, action);
}

// IPS
inline float get_cost_estimate(const VW::cb_label& ld, uint32_t action)
{
  for (auto& cl : ld.costs)
  {
    if (cl.action == action) { return get_cost_estimate(cl, action); }
  }
  return 0.0f;
}

// doubly robust estimate
inline float get_cost_estimate(const VW::action_score& a_s, float cost, uint32_t action, float offset = 0.)
{
  if (action == a_s.action) { return (cost - offset) / a_s.score; }
  return 0.;
}

inline bool example_is_newline_not_header_cb(VW::example const& ec)
{
  return (VW::example_is_newline(ec) && !VW::ec_is_example_header_cb(ec));
}
}  // namespace VW

namespace CB_ALGS  // NOLINT
{
template <bool is_learn>
VW_DEPRECATED("Moved into VW namespace.")
float get_cost_pred(
    VW::LEARNER::learner* scorer, const VW::cb_class& known_cost, VW::example& ec, uint32_t index, uint32_t base)
{
  return VW::get_cost_pred<is_learn>(scorer, known_cost, ec, index, base);
}

// IPS estimate
VW_DEPRECATED("Moved into VW namespace.")
inline float get_cost_estimate(const VW::cb_class& observation, uint32_t action, float offset = 0.)
{
  return VW::get_cost_estimate(observation, action, offset);
}

// doubly robust estimate
VW_DEPRECATED("Moved into VW namespace.")
inline float get_cost_estimate(const VW::cb_class& observation, const VW::cs_label& scores, uint32_t action)
{
  return VW::get_cost_estimate(observation, scores, action);
}

// IPS
VW_DEPRECATED("Moved into VW namespace.") inline float get_cost_estimate(const VW::cb_label& ld, uint32_t action)
{
  return VW::get_cost_estimate(ld, action);
}

// doubly robust estimate
VW_DEPRECATED("Moved into VW namespace.")
inline float get_cost_estimate(const VW::action_score& a_s, float cost, uint32_t action, float offset = 0.)
{
  return VW::get_cost_estimate(a_s, cost, action, offset);
}

VW_DEPRECATED("Moved into VW namespace.") inline bool example_is_newline_not_header(VW::example const& ec)
{
  return VW::example_is_newline_not_header_cb(ec);
}
}  // namespace CB_ALGS
