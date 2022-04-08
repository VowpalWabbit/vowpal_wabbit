// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "sample_pdf.h"

#include "api_status.h"
#include "config/options.h"
#include "debug_log.h"
#include "error_constants.h"
#include "explore.h"
#include "global_data.h"
#include "guard.h"
#include "rand_state.h"
#include "setup_base.h"

// Aliases
using VW::cb_continuous::continuous_label;
using VW::cb_continuous::continuous_label_elm;
using VW::config::make_option;
using VW::config::option_group_definition;
using VW::config::options_i;
using VW::LEARNER::single_learner;

// Enable/Disable indented debug statements
#undef VW_DEBUG_LOG
#define VW_DEBUG_LOG vw_dbg::cb_sample_pdf

namespace
{
////////////////////////////////////////////////////
// BEGIN sample_pdf reduction and reduction methods
struct sample_pdf
{
  int learn(example& ec, VW::experimental::api_status* status);
  int predict(example& ec, VW::experimental::api_status* status);

  void init(single_learner* p_base, std::shared_ptr<VW::rand_state> random_state);

private:
  std::shared_ptr<VW::rand_state> _p_random_state;
  VW::continuous_actions::probability_density_function _pred_pdf;
  single_learner* _base = nullptr;
};

int sample_pdf::learn(example& ec, VW::experimental::api_status*)
{
  // one of the base reductions will call predict so we need a valid
  // predict buffer
  _pred_pdf.clear();
  {  // scope to predict & restore prediction
    auto restore = VW::swap_guard(ec.pred.pdf, _pred_pdf);
    _base->learn(ec);
  }
  return VW::experimental::error_code::success;
}

int sample_pdf::predict(example& ec, VW::experimental::api_status*)
{
  _pred_pdf.clear();

  {  // scope to predict & restore prediction
    auto restore = VW::swap_guard(ec.pred.pdf, _pred_pdf);
    _base->predict(ec);
  }

  uint64_t seed = _p_random_state->get_current_state();
  const int ret_code = exploration::sample_pdf(
      &seed, std::begin(_pred_pdf), std::end(_pred_pdf), ec.pred.pdf_value.action, ec.pred.pdf_value.pdf_value);
  _p_random_state->get_and_update_random();

  if (ret_code != S_EXPLORATION_OK) { return VW::experimental::error_code::sample_pdf_failed; }

  return VW::experimental::error_code::success;
}

void sample_pdf::init(single_learner* p_base, std::shared_ptr<VW::rand_state> random_state)
{
  _base = p_base;
  _p_random_state = std::move(random_state);
  _pred_pdf.clear();
}

// Free function to tie function pointers to reduction class methods
template <bool is_learn>
void predict_or_learn(sample_pdf& reduction, single_learner&, example& ec)
{
  VW::experimental::api_status status;
  if (is_learn) { reduction.learn(ec, &status); }
  else
  {
    if (VW::experimental::error_code::success != reduction.predict(ec, &status))
      THROW(VW::experimental::error_code::sample_pdf_failed_s);
  }

  if (status.get_error_code() != VW::experimental::error_code::success)
  { VW_DBG(ec) << status.get_error_msg() << std::endl; }
}
}  // namespace

// END sample_pdf reduction and reduction methods
////////////////////////////////////////////////////

VW::LEARNER::base_learner* VW::reductions::sample_pdf_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();
  option_group_definition new_options("[Reduction] Continuous Actions: Sample Pdf");
  bool invoked = false;
  new_options.add(
      make_option("sample_pdf", invoked).keep().necessary().help("Sample a pdf and pick a continuous valued action"));

  // If sample_pdf reduction was not invoked, don't add anything
  // to the reduction stack;
  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  LEARNER::base_learner* p_base = stack_builder.setup_base_learner();
  auto p_reduction = VW::make_unique<sample_pdf>();
  p_reduction->init(as_singleline(p_base), all.get_random_state());

  // This learner will assume the label type from base, so should not call set_input_label_type
  auto* l = make_reduction_learner(std::move(p_reduction), as_singleline(p_base), predict_or_learn<true>,
      predict_or_learn<false>, stack_builder.get_setupfn_name(sample_pdf_setup))
                .set_output_prediction_type(VW::prediction_type_t::action_pdf_value)
                .build();

  return VW::LEARNER::make_base(*l);
}
