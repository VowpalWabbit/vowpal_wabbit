// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "baseline.h"
#include "guard.h"

#include "cb.h"
#include "example.h"
#include "learner.h"

#include <cfloat>

// TODO: extend to handle CSOAA_LDF and WAP_LDF
VW::LEARNER::base_learner* cb_algs_setup(VW::setup_base_i& stack_builder);

namespace VW
{
enum class cb_type_t {
  dr,
  dm,
  ips,
  mtr,
  sm
};

inline cb_type_t cb_type_from_string(VW::string_view str)
{
  if (str == "dr") { return cb_type_t::dr; }
  if (str == "dm") { return VW::cb_type_t::dm; }
  if (str == "ips") { return VW::cb_type_t::ips; }
  if (str == "mtr") { return VW::cb_type_t::mtr; }
  if (str == "sm") { return VW::cb_type_t::sm; }
  THROW("Unknown cb_type: " << str);
}

inline VW::string_view to_string(cb_type_t type)
{
  switch(type)
  {
  case cb_type_t::dr:
    return "dr";
  case cb_type_t::dm:
    return "dm";
  case cb_type_t::ips:
    return "ips";
  case cb_type_t::mtr:
    return "mtr";
  case cb_type_t::sm:
    return "sm";
  }
  THROW("Unknown cb_type passed to to_string");
}
}  // namespace VW

namespace CB_ALGS
{
template <bool is_learn>
float get_cost_pred(
    VW::LEARNER::single_learner* scorer, const CB::cb_class& known_cost, example& ec, uint32_t index, uint32_t base)
{
  VW_DBG(ec) << "get_cost_pred:" << is_learn << std::endl;

  label_data simple_temp;
  if (index == known_cost.action)
    simple_temp.label = known_cost.cost;
  else
    simple_temp.label = FLT_MAX;

  const bool baseline_enabled_old = BASELINE::baseline_enabled(&ec);
  BASELINE::set_baseline_enabled(&ec);
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

  if (!baseline_enabled_old) BASELINE::reset_baseline_disabled(&ec);
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

inline bool example_is_newline_not_header(example const& ec)
{
  return (example_is_newline(ec) && !CB::ec_is_example_header(ec));
}

void generic_output_example(vw& all, float loss, example& ec, const CB::label& ld, CB::cb_class* known_cost);

}  // namespace CB_ALGS
