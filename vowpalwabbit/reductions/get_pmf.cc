// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "get_pmf.h"

#include "api_status.h"
#include "config/options.h"
#include "debug_log.h"
#include "error_constants.h"
#include "global_data.h"
#include "guard.h"

// Aliases
using std::endl;
using VW::cb_continuous::continuous_label;
using VW::cb_continuous::continuous_label_elm;
using VW::config::make_option;
using VW::config::option_group_definition;
using VW::config::options_i;
using VW::LEARNER::single_learner;

// Enable/Disable indented debug statements
#undef VW_DEBUG_LOG
#define VW_DEBUG_LOG vw_dbg::cb_explore_get_pmf

namespace VW
{
namespace continuous_action
{
////////////////////////////////////////////////////
// BEGIN sample_pdf reduction and reduction methods
struct get_pmf
{
  int learn(example& ec, experimental::api_status* status);
  int predict(example& ec, experimental::api_status* status);

  void init(single_learner* p_base, float epsilon);

private:
  single_learner* _base = nullptr;
  float _epsilon = 0.f;
};

int get_pmf::learn(example& ec, experimental::api_status*)
{
  _base->learn(ec);
  return VW::experimental::error_code::success;
}

int get_pmf::predict(example& ec, experimental::api_status*)
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

void get_pmf::init(single_learner* p_base, float epsilon)
{
  _base = p_base;
  _epsilon = epsilon;
}

// Free function to tie function pointers to reduction class methods
template <bool is_learn>
void predict_or_learn(get_pmf& reduction, single_learner&, example& ec)
{
  experimental::api_status status;
  if (is_learn)
    reduction.learn(ec, &status);
  else
    reduction.predict(ec, &status);

  if (status.get_error_code() != VW::experimental::error_code::success)
  { VW_DBG(ec) << status.get_error_msg() << endl; }
}

// END sample_pdf reduction and reduction methods
////////////////////////////////////////////////////

// Setup reduction in stack
LEARNER::base_learner* get_pmf_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  option_group_definition new_options("[Reduction] Continuous Actions: Convert to Pmf");
  bool invoked = false;
  float epsilon = 0.0f;
  new_options.add(
      make_option("get_pmf", invoked).keep().necessary().help("Convert a single multiclass prediction to a pmf"));

  // If reduction was not invoked, don't add anything
  // to the reduction stack;
  if (!options.add_parse_and_check_necessary(new_options)) return nullptr;

  LEARNER::base_learner* p_base = stack_builder.setup_base_learner();
  auto p_reduction = VW::make_unique<get_pmf>();
  p_reduction->init(as_singleline(p_base), epsilon);

  auto* l = make_reduction_learner(std::move(p_reduction), as_singleline(p_base), predict_or_learn<true>,
      predict_or_learn<false>, stack_builder.get_setupfn_name(get_pmf_setup))
                .set_output_prediction_type(VW::prediction_type_t::pdf)
                .build();

  return make_base(*l);
}
}  // namespace continuous_action
}  // namespace VW
