// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/cbzo.h"

#include "vw/core/io_buf.h"
#include "vw/core/learner.h"
#include "vw/core/parse_regressor.h"
#include "vw/core/prediction_type.h"
#include "vw/core/prob_dist_cont.h"
#include "vw/core/reductions/gd.h"
#include "vw/core/setup_base.h"
#include "vw/core/shared_data.h"
#include "vw/core/vw.h"
#include "vw/core/vw_math.h"
#include "vw/io/logger.h"

#include <cfloat>
#include <random>

using namespace VW::LEARNER;
using namespace VW::config;
using VW::continuous_actions::probability_density_function;

namespace
{
constexpr uint8_t CONSTANT_POLICY = 0;
constexpr uint8_t LINEAR_POLICY = 1;

class cbzo
{
public:
  float radius = 0.f;
  VW::workspace* all = nullptr;
  bool min_prediction_supplied = false;
  bool max_prediction_supplied = false;
};

class linear_update_data
{
public:
  float mult = 0.f;
  float part_grad = 0.f;
  VW::workspace* all = nullptr;
};

// uint64_t index variant of VW::get_weight
inline float get_weight(VW::workspace& all, uint64_t index, uint32_t offset)
{
  return (&all.weights[(index) << all.weights.stride_shift()])[offset];
}

// uint64_t index variant of VW::set_weight
inline void set_weight(VW::workspace& all, uint64_t index, uint32_t offset, float value)
{
  (&all.weights[(index) << all.weights.stride_shift()])[offset] = value;
}

float l1_grad(VW::workspace& all, uint64_t fi)
{
  if (all.no_bias && fi == VW::details::CONSTANT) { return 0.0f; }

  float fw = get_weight(all, fi, 0);
  return fw >= 0.0f ? all.l1_lambda : -all.l1_lambda;
}

float l2_grad(VW::workspace& all, uint64_t fi)
{
  if (all.no_bias && fi == VW::details::CONSTANT) { return 0.0f; }

  float fw = get_weight(all, fi, 0);
  return all.l2_lambda * fw;
}

inline void accumulate_dotprod(float& dotprod, float x, float& fw) { dotprod += x * fw; }

inline float constant_inference(VW::workspace& all)
{
  float wt = get_weight(all, VW::details::CONSTANT, 0);
  return wt;
}

float linear_inference(VW::workspace& all, VW::example& ec)
{
  float dotprod = 0;
  VW::foreach_feature<float, accumulate_dotprod>(all, ec, dotprod);
  return dotprod;
}

VW_WARNING_STATE_PUSH
VW_WARNING_DISABLE_COND_CONST_EXPR
template <uint8_t policy>
float inference(VW::workspace& all, VW::example& ec)
{
  if VW_STD17_CONSTEXPR (policy == CONSTANT_POLICY) { return constant_inference(all); }
  else if VW_STD17_CONSTEXPR (policy == LINEAR_POLICY) { return linear_inference(all, ec); }
  else
    THROW("Unknown policy encountered: " << policy);
}

template <bool feature_mask_off>
void constant_update(cbzo& data, VW::example& ec)
{
  float fw = get_weight(*data.all, VW::details::CONSTANT, 0);
  if (feature_mask_off || fw != 0.0f)
  {
    float action_centroid = inference<CONSTANT_POLICY>(*data.all, ec);
    float grad = ec.l.cb_cont.costs[0].cost / (ec.l.cb_cont.costs[0].action - action_centroid);
    float update =
        -data.all->eta * (grad + l1_grad(*data.all, VW::details::CONSTANT) + l2_grad(*data.all, VW::details::CONSTANT));

    set_weight(*data.all, VW::details::CONSTANT, 0, fw + update);
  }
}

template <bool feature_mask_off>
void linear_per_feature_update(linear_update_data& upd_data, float x, uint64_t fi)
{
  float fw = get_weight(*upd_data.all, fi, 0);

  if (feature_mask_off || fw != 0.0f)
  {
    float update = upd_data.mult * (upd_data.part_grad * x + (l1_grad(*upd_data.all, fi) + l2_grad(*upd_data.all, fi)));
    set_weight(*upd_data.all, fi, 0, fw + update);
  }
}

template <bool feature_mask_off>
void linear_update(cbzo& data, VW::example& ec)
{
  float mult = -data.all->eta;

  float action_centroid = inference<LINEAR_POLICY>(*data.all, ec);
  float part_grad = ec.l.cb_cont.costs[0].cost / (ec.l.cb_cont.costs[0].action - action_centroid);

  linear_update_data upd_data;
  upd_data.mult = mult;
  upd_data.part_grad = part_grad;
  upd_data.all = data.all;

  VW::foreach_feature<linear_update_data, uint64_t, linear_per_feature_update<feature_mask_off>>(
      *data.all, ec, upd_data);
}

template <uint8_t policy, bool feature_mask_off>
void update_weights(cbzo& data, VW::example& ec)
{
  if VW_STD17_CONSTEXPR (policy == CONSTANT_POLICY) { constant_update<feature_mask_off>(data, ec); }
  else if VW_STD17_CONSTEXPR (policy == LINEAR_POLICY) { linear_update<feature_mask_off>(data, ec); }
  else
    THROW("Unknown policy encountered: " << policy)
}
VW_WARNING_STATE_POP

void set_minmax(VW::shared_data* sd, float label, bool min_fixed, bool max_fixed)
{
  if (!min_fixed) { sd->min_label = std::min(label, sd->min_label); }
  if (!max_fixed) { sd->max_label = std::max(label, sd->max_label); }
}

void print_audit_features(VW::workspace& all, VW::example& ec)
{
  if (all.audit)
  {
    all.print_text_by_ref(all.stdout_adapter.get(),
        VW::to_string(ec.pred.pdf, std::numeric_limits<float>::max_digits10), ec.tag, all.logger);
  }

  VW::details::print_features(all, ec);
}

// Returns a value close to x and greater than it
inline float close_greater_value(float x)
{
  if (x != 0.f) { return std::nextafter(x, std::numeric_limits<float>::infinity()); }

  // When x==0, nextafter returns a very small value, thus use this instead
  return 1e-5f;
}

// Returns a value close to x and lesser than it
inline float close_lesser_value(float x)
{
  if (x != 0.f) { return std::nextafter(x, -std::numeric_limits<float>::infinity()); }

  // When x==0, nextafter returns a very small value, thus use this instead
  return -1e-5f;
}

// Approximates a uniform pmf over two values 'a' and 'b' as a 2 spike pdf
void approx_pmf_to_pdf(float a, float b, probability_density_function& pdf)
{
  float left = close_lesser_value(a);
  float right = close_greater_value(a);
  auto pdf_val = static_cast<float>(0.5 / (right - left));
  pdf.push_back({left, right, pdf_val});

  left = close_lesser_value(b);
  right = close_greater_value(b);
  pdf_val = static_cast<float>(0.5 / (right - left));
  pdf.push_back({left, right, pdf_val});
}

template <uint8_t policy, bool audit_or_hash_inv>
void predict(cbzo& data, VW::example& ec)
{
  ec.pred.pdf.clear();

  float action_centroid = inference<policy>(*data.all, ec);
  set_minmax(data.all->sd.get(), action_centroid, data.min_prediction_supplied, data.max_prediction_supplied);
  action_centroid = VW::math::clamp(action_centroid, data.all->sd->min_label, data.all->sd->max_label);

  approx_pmf_to_pdf(action_centroid - data.radius, action_centroid + data.radius, ec.pred.pdf);

  if (audit_or_hash_inv) { print_audit_features(*data.all, ec); }
}

template <uint8_t policy, bool feature_mask_off, bool audit_or_hash_inv>
void learn(cbzo& data, VW::example& ec)
{
  // update_weights() doesn't require predict() to be called. It is called
  // to respect --audit, --invert_hash, --predictions for train examples
  predict<policy, audit_or_hash_inv>(data, ec);
  update_weights<policy, feature_mask_off>(data, ec);
}

inline void save_load_regressor(VW::workspace& all, VW::io_buf& model_file, bool read, bool text)
{
  VW::details::save_load_regressor_gd(all, model_file, read, text);
}

void save_load(cbzo& data, VW::io_buf& model_file, bool read, bool text)
{
  VW::workspace& all = *data.all;
  if (read)
  {
    VW::details::initialize_regressor(all);
    if (data.all->initial_constant != 0.0f) { set_weight(all, VW::details::CONSTANT, 0, data.all->initial_constant); }
  }
  if (model_file.num_files() > 0) { save_load_regressor(all, model_file, read, text); }
}

bool is_labeled(const VW::example& ec)
{
  return (!ec.l.cb_cont.costs.empty() && ec.l.cb_cont.costs[0].action != FLT_MAX);
}

void update_stats_cbzo(const VW::workspace& /* all */, VW::shared_data& sd, const cbzo& /* data */,
    const VW::example& ec, VW::io::logger& /* logger */)
{
  const auto& costs = ec.l.cb_cont.costs;
  sd.update(ec.test_only, is_labeled(ec), costs.empty() ? 0.0f : costs[0].cost, ec.weight, ec.get_num_features());
  sd.weighted_labels += ec.weight;
}

void output_example_prediction_cbzo(
    VW::workspace& all, const cbzo& /* data */, const VW::example& ec, VW::io::logger& logger)
{
  auto pred_repr = VW::to_string(ec.pred.pdf, std::numeric_limits<float>::max_digits10);
  for (auto& sink : all.final_prediction_sink) { all.print_text_by_ref(sink.get(), pred_repr, ec.tag, logger); }
}

void print_update_cbzo(VW::workspace& all, VW::shared_data& sd, const cbzo& /* data */, const VW::example& ec,
    VW::io::logger& /* unused */)
{
  if (sd.weighted_examples() >= sd.dump_interval && !all.quiet)
  {
    const auto& costs = ec.l.cb_cont.costs;
    sd.print_update(*all.trace_message, all.holdout_set_off, all.current_pass,
        ec.test_only ? "unknown" : VW::to_string(costs[0]),
        VW::to_string(ec.pred.pdf, VW::details::DEFAULT_FLOAT_FORMATTING_DECIMAL_PRECISION), ec.get_num_features());
  }
}

void (*get_learn(VW::workspace& all, uint8_t policy, bool feature_mask_off))(cbzo&, VW::example&)
{
  if (policy == CONSTANT_POLICY)
  {
    if (feature_mask_off)
    {
      if (all.audit || all.hash_inv) { return learn<CONSTANT_POLICY, true, true>; }
      else { return learn<CONSTANT_POLICY, true, false>; }
    }
    else if (all.audit || all.hash_inv) { return learn<CONSTANT_POLICY, false, true>; }
    else { return learn<CONSTANT_POLICY, false, false>; }
  }
  else if (policy == LINEAR_POLICY)
  {
    if (feature_mask_off)
    {
      if (all.audit || all.hash_inv) { return learn<LINEAR_POLICY, true, true>; }
      else { return learn<LINEAR_POLICY, true, false>; }
    }
    else if (all.audit || all.hash_inv) { return learn<LINEAR_POLICY, false, true>; }
    else { return learn<LINEAR_POLICY, false, false>; }
  }
  else
    THROW("Unknown policy encountered: " << policy)
}

void (*get_predict(VW::workspace& all, uint8_t policy))(cbzo&, VW::example&)
{
  if (policy == CONSTANT_POLICY)
  {
    if (all.audit || all.hash_inv) { return predict<CONSTANT_POLICY, true>; }
    else { return predict<CONSTANT_POLICY, false>; }
  }
  else if (policy == LINEAR_POLICY)
  {
    if (all.audit || all.hash_inv) { return predict<LINEAR_POLICY, true>; }
    else { return predict<LINEAR_POLICY, false>; }
  }
  else
    THROW("Unknown policy encountered: " << policy)
}

}  // namespace

std::shared_ptr<VW::LEARNER::learner> VW::reductions::cbzo_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();

  auto data = VW::make_unique<cbzo>();

  std::string policy_str;
  bool cbzo_option = false;

  option_group_definition new_options(
      "[Reduction] Continuous Action Contextual Bandit using Zeroth-Order Optimization");
  new_options
      .add(make_option("cbzo", cbzo_option)
               .keep()
               .necessary()
               .help("Solve 1-slot Continuous Action Contextual Bandit using Zeroth-Order Optimization"))
      .add(make_option("policy", policy_str)
               .default_value("linear")
               .one_of({"linear", "constant"})
               .keep()
               .help("Policy/Model to Learn"))
      .add(make_option("radius", data->radius).default_value(0.1f).keep(all.save_resume).help("Exploration Radius"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  bool feature_mask_off = true;
  if (options.was_supplied("feature_mask")) { feature_mask_off = false; }

  uint8_t policy;
  if (policy_str == "constant") { policy = CONSTANT_POLICY; }
  else if (policy_str == "linear") { policy = LINEAR_POLICY; }
  else
    THROW("policy must be in {'constant', 'linear'}");

  if (policy == CONSTANT_POLICY)
  {
    if (options.was_supplied("noconstant")) THROW("constant policy can't be learnt when --noconstant is used")

    if (!feature_mask_off)
    {
      all.logger.err_warn("Feature_mask used with constant policy (where there is only one weight to learn).");
    }
  }

  all.example_parser->lbl_parser = cb_continuous::the_label_parser;
  data->all = &all;
  data->min_prediction_supplied = options.was_supplied("min_prediction");
  data->max_prediction_supplied = options.was_supplied("max_prediction");

  auto l = make_bottom_learner(std::move(data), get_learn(all, policy, feature_mask_off), get_predict(all, policy),
      stack_builder.get_setupfn_name(cbzo_setup), prediction_type_t::PDF, label_type_t::CONTINUOUS)
               .set_params_per_weight(0)
               .set_save_load(save_load)
               .set_output_example_prediction(output_example_prediction_cbzo)
               .set_print_update(print_update_cbzo)
               .set_update_stats(update_stats_cbzo)
               .build();

  return l;
}
