// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/baseline.h"

#include "vw/config/options.h"
#include "vw/core/learner.h"
#include "vw/core/loss_functions.h"
#include "vw/core/setup_base.h"
#include "vw/core/shared_data.h"
#include "vw/core/vw.h"

using namespace VW::LEARNER;
using namespace VW::config;
using namespace VW::reductions;

namespace
{
constexpr float MAX_MULTIPLIER = 1000.f;
}  // namespace

void VW::reductions::baseline::set_baseline_enabled(VW::example* ec)
{
  if (!baseline_enabled(ec)) { ec->indices.push_back(VW::details::BASELINE_ENABLED_MESSAGE_NAMESPACE); }
}

void VW::reductions::baseline::reset_baseline_disabled(VW::example* ec)
{
  const auto it = std::find(ec->indices.begin(), ec->indices.end(), VW::details::BASELINE_ENABLED_MESSAGE_NAMESPACE);
  if (it != ec->indices.end()) { ec->indices.erase(it); }
}

bool VW::reductions::baseline::baseline_enabled(const VW::example* ec)
{
  const auto it = std::find(ec->indices.begin(), ec->indices.end(), VW::details::BASELINE_ENABLED_MESSAGE_NAMESPACE);
  return it != ec->indices.end();
}

class baseline_data
{
public:
  VW::example ec;
  VW::workspace* all = nullptr;
  bool lr_scaling = false;  // whether to scale baseline_data learning rate based on max label
  float lr_multiplier = 1.f;
  bool global_only = false;  // only use a global constant for the baseline
  bool global_initialized = false;
  bool check_enabled = false;  // only use baseline when the example contains enabled flag
};

void init_global(baseline_data& data)
{
  if (!data.global_only) { return; }
  // use a separate global constant
  data.ec.indices.push_back(VW::details::CONSTANT_NAMESPACE);
  // different index from constant to avoid conflicts
  data.ec.feature_space[VW::details::CONSTANT_NAMESPACE].push_back(1,
      ((VW::details::CONSTANT - 17) * data.all->wpp) << data.all->weights.stride_shift(),
      VW::details::CONSTANT_NAMESPACE);
  data.ec.reset_total_sum_feat_sq();
  data.ec.num_features++;
}

template <bool is_learn>
void predict_or_learn(baseline_data& data, learner& base, VW::example& ec)
{
  // no baseline if check_enabled is true and example contains flag
  if (data.check_enabled && !VW::reductions::baseline::baseline_enabled(&ec))
  {
    if (is_learn) { base.learn(ec); }
    else { base.predict(ec); }
    return;
  }

  // always do a full prediction, for safety in accurate predictive validation
  if (data.global_only)
  {
    if (!data.global_initialized)
    {
      init_global(data);
      data.global_initialized = true;
    }
    VW::copy_example_metadata(&data.ec, &ec);
    base.predict(data.ec);
    auto& simple_red_features = ec.ex_reduction_features.template get<VW::simple_label_reduction_features>();
    simple_red_features.initial = data.ec.pred.scalar;
    base.predict(ec);
  }
  else { base.predict(ec); }

  if (is_learn)
  {
    const float pred = ec.pred.scalar;  // save 'safe' prediction

    // now learn
    data.ec.l.simple = ec.l.simple;
    if (!data.global_only)
    {
      // move label & constant features data over to baseline example
      VW::copy_example_metadata(&data.ec, &ec);
      VW::move_feature_namespace(&data.ec, &ec, VW::details::CONSTANT_NAMESPACE);
    }

    // regress baseline on label
    if (data.lr_scaling)
    {
      float multiplier = data.lr_multiplier;
      if (multiplier == 0)
      {
        multiplier = std::max(0.0001f, std::max(std::abs(data.all->sd->min_label), std::abs(data.all->sd->max_label)));
        if (multiplier > MAX_MULTIPLIER) { multiplier = MAX_MULTIPLIER; }
      }
      data.all->eta *= multiplier;
      base.learn(data.ec);
      data.all->eta /= multiplier;
    }
    else { base.learn(data.ec); }

    // regress residual
    auto& simple_red_features = ec.ex_reduction_features.template get<VW::simple_label_reduction_features>();
    simple_red_features.initial = data.ec.pred.scalar;
    base.learn(ec);

    if (!data.global_only)
    {
      // move feature data back to the original example
      VW::move_feature_namespace(&ec, &data.ec, VW::details::CONSTANT_NAMESPACE);
    }

    // return the safe prediction
    ec.pred.scalar = pred;
  }
}

float sensitivity(baseline_data& data, learner& base, VW::example& ec)
{
  // no baseline if check_enabled is true and example contains flag
  if (data.check_enabled && !VW::reductions::baseline::baseline_enabled(&ec)) { return base.sensitivity(ec); }

  if (!data.global_only) { THROW("sensitivity for baseline without --global_only not implemented") }

  // sensitivity of baseline term
  VW::copy_example_metadata(&data.ec, &ec);
  data.ec.l.simple.label = ec.l.simple.label;
  data.ec.pred.scalar = ec.pred.scalar;
  const float baseline_sens = base.sensitivity(data.ec);

  // sensitivity of residual
  require_singleline(&base)->predict(data.ec);
  auto& simple_red_features = ec.ex_reduction_features.template get<VW::simple_label_reduction_features>();
  simple_red_features.initial = data.ec.pred.scalar;
  const float sens = base.sensitivity(ec);
  return baseline_sens + sens;
}

std::shared_ptr<VW::LEARNER::learner> VW::reductions::baseline_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();
  auto data = VW::make_unique<baseline_data>();
  bool baseline_option = false;
  std::string loss_function;

  option_group_definition new_options("[Reduction] Baseline");
  new_options
      .add(make_option("baseline", baseline_option)
               .keep()
               .necessary()
               .help("Learn an additive baseline (from constant features) and a residual separately in regression"))
      .add(make_option("lr_multiplier", data->lr_multiplier)
               .default_value(1.f)
               .help("Learning rate multiplier for baseline model"))
      .add(make_option("global_only", data->global_only)
               .keep()
               .help("Use separate example with only global constant for baseline predictions"))
      .add(make_option("check_enabled", data->check_enabled)
               .keep()
               .help("Only use baseline when the example contains enabled flag"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  // initialize baseline example's interactions.
  data->ec.interactions = &all.interactions;
  data->ec.extent_interactions = &all.extent_interactions;
  data->all = &all;

  const auto loss_function_type = all.loss->get_type();
  if (loss_function_type != "logistic") { data->lr_scaling = true; }

  auto base = require_singleline(stack_builder.setup_base_learner());
  auto l = make_reduction_learner(std::move(data), base, predict_or_learn<true>, predict_or_learn<false>,
      stack_builder.get_setupfn_name(VW::reductions::baseline_setup))
               .set_input_prediction_type(VW::prediction_type_t::SCALAR)
               .set_output_prediction_type(VW::prediction_type_t::SCALAR)
               .set_input_label_type(VW::label_type_t::SIMPLE)
               .set_output_label_type(VW::label_type_t::SIMPLE)
               .set_sensitivity(sensitivity)
               .build();

  return l;
}
