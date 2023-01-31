// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/get_pmf.h"

#include "vw/config/options.h"
#include "vw/core/api_status.h"
#include "vw/core/cb_continuous_label.h"
#include "vw/core/debug_log.h"
#include "vw/core/error_constants.h"
#include "vw/core/global_data.h"
#include "vw/core/guard.h"
#include "vw/core/learner.h"
#include "vw/core/setup_base.h"

// Aliases
using VW::cb_continuous::continuous_label;
using VW::cb_continuous::continuous_label_elm;
using VW::config::make_option;
using VW::config::option_group_definition;
using VW::config::options_i;
using VW::LEARNER::learner;

// Enable/Disable indented debug statements
#undef VW_DEBUG_LOG
#define VW_DEBUG_LOG vw_dbg::CB_EXPLORE_GET_PMF

namespace
{
////////////////////////////////////////////////////
// BEGIN sample_pdf reduction and reduction methods
class get_pmf
{
public:
  int learn(VW::example& ec, VW::experimental::api_status* status);
  int predict(VW::example& ec, VW::experimental::api_status* status);

  void init(learner* p_base, float epsilon);

private:
  learner* _base = nullptr;
  float _epsilon = 0.f;
};

int get_pmf::learn(VW::example& ec, VW::experimental::api_status* /*unused*/)
{
  _base->learn(ec);
  return VW::experimental::error_code::success;
}

int get_pmf::predict(VW::example& ec, VW::experimental::api_status* /*unused*/)
{
  uint32_t base_prediction;

  {  // predict & restore prediction
    auto restore = VW::stash_guard(ec.pred);
    _base->predict(ec);
    base_prediction = ec.pred.multiclass - 1;
  }

  // Assume ec.pred.a_s allocated by the caller (probably pmf_to_pdf);
  ec.pred.a_s.clear();
  ec.pred.a_s.push_back({base_prediction, 1.0f});

  return VW::experimental::error_code::success;
}

void get_pmf::init(learner* p_base, float epsilon)
{
  _base = p_base;
  _epsilon = epsilon;
}

// Free function to tie function pointers to reduction class methods
template <bool is_learn>
void predict_or_learn(get_pmf& reduction, learner& /*unused*/, VW::example& ec)
{
  VW::experimental::api_status status;
  if (is_learn) { reduction.learn(ec, &status); }
  else { reduction.predict(ec, &status); }

  if (status.get_error_code() != VW::experimental::error_code::success)
  {
    VW_DBG(ec) << status.get_error_msg() << std::endl;
  }
}
}  // namespace

// END sample_pdf reduction and reduction methods
////////////////////////////////////////////////////

// Setup reduction in stack
std::shared_ptr<VW::LEARNER::learner> VW::reductions::get_pmf_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  option_group_definition new_options("[Reduction] Continuous Actions: Convert to Pmf");
  bool invoked = false;
  float epsilon = 0.0f;
  new_options.add(
      make_option("get_pmf", invoked).keep().necessary().help("Convert a single multiclass prediction to a pmf"));

  // If reduction was not invoked, don't add anything
  // to the reduction stack;
  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  auto p_base = stack_builder.setup_base_learner();
  auto p_reduction = VW::make_unique<get_pmf>();
  p_reduction->init(require_singleline(p_base).get(), epsilon);

  auto l = make_reduction_learner(std::move(p_reduction), require_singleline(p_base), predict_or_learn<true>,
      predict_or_learn<false>, stack_builder.get_setupfn_name(get_pmf_setup))
               .set_input_label_type(p_base->get_input_label_type())
               .set_output_label_type(p_base->get_input_label_type())
               .set_input_prediction_type(VW::prediction_type_t::MULTICLASS)
               .set_output_prediction_type(VW::prediction_type_t::ACTION_SCORES)
               .build();

  return l;
}
