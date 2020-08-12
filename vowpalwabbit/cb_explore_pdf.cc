// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "cb_explore_pdf.h"
#include "err_constants.h"
#include "api_status.h"
#include "debug_log.h"
#include "parse_args.h"

// Aliases
using VW::LEARNER::single_learner;
using std::endl;
using VW::config::make_option;
using VW::config::option_group_definition;
using VW::config::options_i;

// Enable/Disable indented debug statements
VW_DEBUG_ENABLE(false)

namespace VW
{
namespace continuous_action
{
  ////////////////////////////////////////////////////
  // BEGIN sample_pdf reduction and reduction methods
  struct cb_explore_pdf
  {
    int learn(example& ec, experimental::api_status* status);
    int predict(example& ec, experimental::api_status* status);

    void init(single_learner* p_base);

    float epsilon;
    float min_value;
    float max_value;

    private:
      single_learner* _base = nullptr;
  };

  int cb_explore_pdf::learn(example& ec, experimental::api_status*)
  {
    _base->learn(ec);
    return error_code::success;
  }

  int cb_explore_pdf::predict(example& ec, experimental::api_status*)
  {
    _base->predict(ec);

    continuous_actions::probability_density_function& _pred_pdf = ec.pred.pdf;
    for (uint32_t i = 0; i < _pred_pdf.size(); i++)
    {
      _pred_pdf[i].pdf_value = _pred_pdf[i].pdf_value * (1 - epsilon) + epsilon / (max_value - min_value);
    }
    return error_code::success;
  }

  void cb_explore_pdf::init(single_learner* p_base)
  {
    _base = p_base;
  }

  // Free function to tie function pointers to reduction class methods
  template <bool is_learn>
  void predict_or_learn(cb_explore_pdf& reduction, single_learner&, example& ec)
  {
    experimental::api_status status;
    if (is_learn)
      reduction.learn(ec, &status);
    else
      reduction.predict(ec, &status);

    if (status.get_error_code() != error_code::success)
    {
      VW_DBG(ec) << status.get_error_msg() << endl;
    }
  }

  // END sample_pdf reduction and reduction methods
  ////////////////////////////////////////////////////

  // Setup reduction in stack
  LEARNER::base_learner* cb_explore_pdf_setup(config::options_i& options, vw& all)
  {
    option_group_definition new_options("Continuous actions");
    bool invoked = false;
    float epsilon;
    float min;
    float max;
    new_options
        .add(make_option("cb_explore_pdf", invoked).keep().help("Sample a pdf and pick a continuous valued action"))
        .add(make_option("epsilon", epsilon).keep().allow_override().default_value(0.05f).help("epsilon-greedy exploration"))
        .add(make_option("min_value", min).keep().default_value(0.0f).help("min value for continuous range"))
        .add(make_option("max_value", max).keep().default_value(1.0f).help("max value for continuous range"));

    options.add_and_parse(new_options);

    // If reduction was not invoked, don't add anything
    // to the reduction stack;
    if (!options.was_supplied("cb_explore_pdf"))
      return nullptr;

    if (!options.was_supplied("min_value") || !options.was_supplied("max_value"))
      THROW("error: min and max values must be supplied with cb_explore_pdf");

    LEARNER::base_learner* p_base = setup_base(options, all);
    auto p_reduction = scoped_calloc_or_throw<cb_explore_pdf>();
    p_reduction->init(as_singleline(p_base));
    p_reduction->epsilon = epsilon;
    p_reduction->min_value = min;
    p_reduction->max_value = max;

    LEARNER::learner<cb_explore_pdf, example>& l = init_learner(p_reduction, as_singleline(p_base),
        predict_or_learn<true>,
        predict_or_learn<false>, 1, prediction_type_t::pdf);

    return make_base(l);
  }
  }  // namespace continuous_action
}  // namespace VW
