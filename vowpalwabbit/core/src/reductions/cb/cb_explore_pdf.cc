// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/cb/cb_explore_pdf.h"

#include "vw/config/options.h"
#include "vw/core/api_status.h"
#include "vw/core/debug_log.h"
#include "vw/core/error_constants.h"
#include "vw/core/global_data.h"
#include "vw/core/setup_base.h"

// Aliases
using std::endl;
using VW::config::make_option;
using VW::config::option_group_definition;
using VW::config::options_i;
using VW::LEARNER::single_learner;

// Enable/Disable indented debug statements
#undef VW_DEBUG_LOG
#define VW_DEBUG_LOG vw_dbg::cb_explore_pdf

namespace
{
////////////////////////////////////////////////////
// BEGIN sample_pdf reduction and reduction methods
struct cb_explore_pdf
{
  int learn(VW::example& ec, VW::experimental::api_status* status);
  int predict(VW::example& ec, VW::experimental::api_status* status);

  void init(single_learner* p_base);

  float epsilon;
  float min_value;
  float max_value;
  bool first_only;

private:
  single_learner* _base = nullptr;
};

int cb_explore_pdf::learn(VW::example& ec, VW::experimental::api_status*)
{
  _base->learn(ec);
  return VW::experimental::error_code::success;
}

int cb_explore_pdf::predict(VW::example& ec, VW::experimental::api_status*)
{
  const auto& reduction_features = ec._reduction_features.template get<VW::continuous_actions::reduction_features>();
  if (first_only && !reduction_features.is_pdf_set() && !reduction_features.is_chosen_action_set())
  {
    // uniform random
    ec.pred.pdf.push_back(
        VW::continuous_actions::pdf_segment{min_value, max_value, static_cast<float>(1. / (max_value - min_value))});
    return VW::experimental::error_code::success;
  }
  else if (first_only && reduction_features.is_pdf_set())
  {
    // pdf provided
    ec.pred.pdf = reduction_features.pdf;
    return VW::experimental::error_code::success;
  }

  _base->predict(ec);

  VW::continuous_actions::probability_density_function& _pred_pdf = ec.pred.pdf;
  for (uint32_t i = 0; i < _pred_pdf.size(); i++)
  { _pred_pdf[i].pdf_value = _pred_pdf[i].pdf_value * (1 - epsilon) + epsilon / (max_value - min_value); }
  return VW::experimental::error_code::success;
}

void cb_explore_pdf::init(single_learner* p_base) { _base = p_base; }

// Free function to tie function pointers to reduction class methods
template <bool is_learn>
void predict_or_learn(cb_explore_pdf& reduction, single_learner&, VW::example& ec)
{
  VW::experimental::api_status status;
  if (is_learn) { reduction.learn(ec, &status); }
  else
  {
    reduction.predict(ec, &status);
  }

  if (status.get_error_code() != VW::experimental::error_code::success)
  { VW_DBG(ec) << status.get_error_msg() << endl; }
}

}  // namespace
// END sample_pdf reduction and reduction methods
////////////////////////////////////////////////////

// Setup reduction in stack
VW::LEARNER::base_learner* VW::reductions::cb_explore_pdf_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();
  option_group_definition new_options("[Reduction] Continuous Actions: cb_explore_pdf");
  bool invoked = false;
  float epsilon;
  float min;
  float max;
  bool first_only = false;
  new_options
      .add(make_option("cb_explore_pdf", invoked)
               .keep()
               .necessary()
               .help("Sample a pdf and pick a continuous valued action"))
      .add(make_option("epsilon", epsilon)
               .keep()
               .allow_override()
               .default_value(0.05f)
               .help("Epsilon-greedy exploration"))
      .add(make_option("min_value", min).keep().default_value(0.0f).help("Min value for continuous range"))
      .add(make_option("max_value", max).keep().default_value(1.0f).help("Max value for continuous range"))
      .add(make_option("first_only", first_only)
               .keep()
               .help("Use user provided first action or user provided pdf or uniform random"));

  // If reduction was not invoked, don't add anything
  // to the reduction stack;
  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  if (!options.was_supplied("min_value") || !options.was_supplied("max_value"))
    THROW("Min and max values must be supplied with cb_explore_pdf");

  auto* p_base = stack_builder.setup_base_learner();
  auto p_reduction = VW::make_unique<cb_explore_pdf>();
  p_reduction->init(as_singleline(p_base));
  p_reduction->epsilon = epsilon;
  p_reduction->min_value = min;
  p_reduction->max_value = max;
  p_reduction->first_only = first_only;

  auto* l = make_reduction_learner(std::move(p_reduction), as_singleline(p_base), predict_or_learn<true>,
      predict_or_learn<false>, stack_builder.get_setupfn_name(cb_explore_pdf_setup))
                .set_input_label_type(VW::label_type_t::cb)
                .set_output_label_type(VW::label_type_t::continuous)
                .set_input_prediction_type(VW::prediction_type_t::pdf)
                .set_output_prediction_type(VW::prediction_type_t::pdf)
                .build(&all.logger);
  return make_base(*l);
}
