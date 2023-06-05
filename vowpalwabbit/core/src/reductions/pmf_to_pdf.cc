// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/pmf_to_pdf.h"

#include "vw/config/option_group_definition.h"
#include "vw/config/options.h"
#include "vw/core/crossplat_compat.h"
#include "vw/core/guard.h"
#include "vw/core/learner.h"
#include "vw/core/setup_base.h"
#include "vw/core/shared_data.h"
#include "vw/core/vw.h"
#include "vw/explore/explore.h"
#include "vw/io/logger.h"

#include <cmath>

using namespace VW;
using namespace VW::config;
using namespace VW::LEARNER;

namespace VW
{
namespace reductions
{
void pmf_to_pdf_reduction::transform_prediction(example& ec)
{
  const float continuous_range = max_value - min_value;
  const float unit_range = continuous_range / num_actions;

  size_t n = temp_pred_a_s.size();
  assert(n != 0);

  auto score = temp_pred_a_s[0].score;
  // map discrete action (predicted tree leaf) to the continuous value of the centre of the leaf
  auto centre = min_value + temp_pred_a_s[0].action * unit_range + unit_range / 2.0f;

  // if zero bandwidth -> stay inside leaf by smoothing around unit_range / 2 (leaf range is unit_range)
  auto b = !bandwidth ? unit_range / 2.0f : bandwidth;

  pdf_lim.clear();
  if (centre - b != min_value) { pdf_lim.push_back(min_value); }

  uint32_t l = 0;
  uint32_t r = 0;
  while (l < n || r < n)
  {
    if (centre >= b)
    {
      if (l == n || centre + b < centre - b)
      {
        auto val = std::min(centre + b, max_value);
        pdf_lim.push_back(val);
        r++;
      }
      else if (r == n || centre - b < centre + b)
      {
        auto val = std::max(centre - b, min_value);
        if ((!pdf_lim.empty() && pdf_lim.back() != val) || pdf_lim.empty()) { pdf_lim.push_back(val); }
        l++;
      }
      else if (centre - b == centre + b)
      {
        auto val = std::max(centre - b, min_value);
        if ((!pdf_lim.empty() && pdf_lim.back() != val) || pdf_lim.empty()) { pdf_lim.push_back(val); }
        l++;
        r++;
      }
    }
    else
    {
      // centre < b so lower limit should be min_value (already added to pdf_lim)
      // so need to add centre + b
      auto val = std::min(centre + b, max_value);
      pdf_lim.push_back(val);
      l++;
      r++;
    }
  }

  if (pdf_lim.back() != max_value) { pdf_lim.push_back(max_value); }

  auto& p_dist = ec.pred.pdf;
  p_dist.clear();

  size_t m = pdf_lim.size();
  l = 0;
  for (uint32_t i = 0; i < m - 1; i++)
  {
    float p = 0;
    // there are 2 ways of knowing that we are entering the pdf limits of the chosen action and thus need to assign a
    // probability: (1) if centre - b < min_value -> pdf_lim would be 'min_value' or
    // (2) pdf_lim is 'centre - b'
    if (l < n && (((centre - min_value) < b && pdf_lim[i] == min_value) || pdf_lim[i] == centre - b))
    {
      // default: 2 * b : 'centre - b' to 'centre + b'
      float actual_b = std::min(max_value, centre + b) - std::max(min_value, centre - b);
      p += score / actual_b;
      l++;
    }
    const float left = pdf_lim[i];
    const float right = pdf_lim[i + 1];

    p_dist.push_back({left, right, p});
  }
}

void pmf_to_pdf_reduction::predict(example& ec)
{
  auto swap_label = VW::swap_guard(ec.l.cb, temp_lbl_cb);

  const auto& reduction_features = ec.ex_reduction_features.template get<VW::continuous_actions::reduction_features>();
  if (first_only && reduction_features.is_chosen_action_set())
  {
    float chosen_action = reduction_features.chosen_action;
    const float continuous_range = max_value - min_value;
    const float unit_range = continuous_range / num_actions;

    // discretize chosen action
    const float ac = (chosen_action - min_value) / unit_range;
    auto action = std::min(num_actions - 1, static_cast<uint32_t>(std::floor(ac)));

    temp_pred_a_s.clear();
    temp_pred_a_s.push_back({action, 1.f});
  }
  else
  {
    // scope for saving / restoring prediction
    auto save_prediction = VW::swap_guard(ec.pred.a_s, temp_pred_a_s);
    _p_base->predict(ec);
  }
  transform_prediction(ec);
}

void pmf_to_pdf_reduction::learn(example& ec)
{
  const float cost = ec.l.cb_cont.costs[0].cost;
  const float pdf_value = ec.l.cb_cont.costs[0].pdf_value;
  const float action_cont = ec.l.cb_cont.costs[0].action;

  const float continuous_range = max_value - min_value;
  const float unit_range = continuous_range / num_actions;

  const float ac = (action_cont - min_value) / unit_range;
  int action_segment_index = std::min(static_cast<int>(num_actions - 1), static_cast<int>(std::floor(ac)));
  const bool cond1 = min_value + action_segment_index * unit_range <= action_cont;
  const bool cond2 = action_cont < min_value + (action_segment_index + 1) * unit_range;

  if (!cond1 || !cond2)
  {
    if (!cond1) { action_segment_index--; }
    if (!cond2) { action_segment_index++; }
  }

  // going to pass label into tree, so need to used discretized version of bandwidth i.e. tree_bandwidth
  uint32_t b = tree_bandwidth;
  const uint32_t local_min_value = std::max(0, action_segment_index - static_cast<int>(b));
  const uint32_t local_max_value = std::min(num_actions - 1, action_segment_index + b);

  auto swap_label = VW::swap_guard(ec.l.cb, temp_lbl_cb);

  ec.l.cb.costs.clear();

  auto actual_bandwidth = !tree_bandwidth ? 1 : 2 * b;  // avoid zero division

  ec.l.cb.costs.push_back(
      VW::cb_class(cost, local_min_value + 1, pdf_value * actual_bandwidth * continuous_range / num_actions));
  ec.l.cb.costs.push_back(
      VW::cb_class(cost, local_max_value + 1, pdf_value * actual_bandwidth * continuous_range / num_actions));

  auto swap_prediction = VW::swap_guard(ec.pred.a_s, temp_pred_a_s);

  _p_base->learn(ec);
}

}  // namespace reductions
}  // namespace VW

namespace
{
void predict(VW::reductions::pmf_to_pdf_reduction& data, learner&, VW::example& ec) { data.predict(ec); }
void learn(VW::reductions::pmf_to_pdf_reduction& data, learner&, VW::example& ec) { data.learn(ec); }
}  // namespace

std::shared_ptr<VW::LEARNER::learner> VW::reductions::pmf_to_pdf_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();
  auto data = VW::make_unique<VW::reductions::pmf_to_pdf_reduction>();

  option_group_definition new_options("[Reduction] Convert Discrete PMF into Continuous PDF");
  new_options
      .add(make_option("pmf_to_pdf", data->num_actions)
               .default_value(0)
               .necessary()
               .keep()
               .help("Number of discrete actions <k> for pmf_to_pdf"))
      .add(make_option("min_value", data->min_value).keep().help("Minimum continuous value"))
      .add(make_option("max_value", data->max_value).keep().help("Maximum continuous value"))
      .add(make_option("bandwidth", data->bandwidth)
               .keep()
               .help("Bandwidth (radius) of randomization around discrete actions in terms of continuous range. By "
                     "default will be set to half of the continuous action unit-range resulting in smoothing that "
                     "stays inside the action space unit-range:\nunit_range = (max_value - "
                     "min_value)/num-of-actions\ndefault bandwidth = unit_range / 2.0"))
      .add(make_option("first_only", data->first_only)
               .keep()
               .help("Use user provided first action or user provided pdf or uniform random"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  if (data->num_actions == 0) { return nullptr; }
  if (!options.was_supplied("min_value") || !options.was_supplied("max_value"))
  {
    THROW("Min and max values must be supplied with cb_continuous");
  }

  float leaf_width = (data->max_value - data->min_value) / (data->num_actions);  // aka unit range
  float half_leaf_width = leaf_width / 2.f;

  if (!options.was_supplied("bandwidth"))
  {
    data->bandwidth = half_leaf_width;
    all.logger.err_info(
        "Bandwidth was not supplied, setting default to half the continuous action unit range: {}", data->bandwidth);
  }

  if (!(data->bandwidth >= 0.0f)) { THROW("error: Bandwidth must be positive"); }

  if (data->bandwidth >= (data->max_value - data->min_value))
  {
    all.logger.err_warn("Bandwidth is larger than continuous action range, this will result in a uniform pdf.");
  }

  // Translate user provided bandwidth which is in terms of continuous action range (max_value - min_value)
  // to the internal tree bandwidth which is in terms of #actions
  if (data->bandwidth <= half_leaf_width) { data->tree_bandwidth = 0; }
  else if (std::fmod((data->bandwidth), leaf_width) == 0)
  {
    data->tree_bandwidth = static_cast<uint32_t>((data->bandwidth) / leaf_width);
  }
  else { data->tree_bandwidth = static_cast<uint32_t>((data->bandwidth) / leaf_width) + 1; }

  options.replace("tree_bandwidth", std::to_string(data->tree_bandwidth));

  auto p_base = require_singleline(stack_builder.setup_base_learner());
  data->_p_base = p_base.get();

  auto l =
      make_reduction_learner(std::move(data), p_base, learn, predict, stack_builder.get_setupfn_name(pmf_to_pdf_setup))
          .set_input_label_type(VW::label_type_t::CONTINUOUS)
          .set_output_label_type(VW::label_type_t::CB)
          .set_input_prediction_type(VW::prediction_type_t::ACTION_SCORES)
          .set_output_prediction_type(VW::prediction_type_t::PDF)
          .build();
  return l;
}
