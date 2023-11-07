// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/binary.h"

#include "vw/config/options.h"
#include "vw/core/debug_log.h"
#include "vw/core/global_data.h"
#include "vw/core/learner.h"
#include "vw/core/setup_base.h"
#include "vw/io/logger.h"

#include <cfloat>
#include <cmath>
#include <utility>

#undef VW_DEBUG_LOG
#define VW_DEBUG_LOG vw_dbg::BINARY

using namespace VW::config;
using namespace VW::reductions;
using std::endl;

class binary_data
{
public:
  VW::io::logger logger;
  explicit binary_data(VW::io::logger logger) : logger(std::move(logger)) {}
};

template <bool is_learn>
void predict_or_learn(binary_data& data, VW::LEARNER::learner& base, VW::example& ec)
{
  if (is_learn) { base.learn(ec); }
  else { base.predict(ec); }

  if (ec.pred.scalar > 0) { ec.pred.scalar = 1; }
  else { ec.pred.scalar = -1; }

  VW_DBG(ec) << "binary: final-pred " << VW::debug::scalar_pred_to_string(ec) << VW::debug::features_to_string(ec)
             << endl;

  if (ec.l.simple.label != FLT_MAX)
  {
    if (std::fabs(ec.l.simple.label) != 1.f)
    {
      data.logger.out_error("The label '{}' is not -1 or 1 as loss function expects.", ec.l.simple.label);
    }
    else if (ec.l.simple.label == ec.pred.scalar) { ec.loss = 0.; }
    else { ec.loss = ec.weight; }
  }
}

std::shared_ptr<VW::LEARNER::learner> VW::reductions::binary_setup(setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();

  bool binary = false;
  option_group_definition new_options("[Reduction] Binary Loss");
  new_options.add(
      make_option("binary", binary).keep().necessary().help("Report loss as binary classification on -1,1"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  auto bin_data = VW::make_unique<binary_data>(stack_builder.get_all_pointer()->logger);
  auto ret =
      VW::LEARNER::make_reduction_learner(std::move(bin_data), require_singleline(stack_builder.setup_base_learner()),
          predict_or_learn<true>, predict_or_learn<false>, stack_builder.get_setupfn_name(binary_setup))
          .set_input_label_type(label_type_t::SIMPLE)
          .set_output_label_type(label_type_t::SIMPLE)
          .set_input_prediction_type(prediction_type_t::SCALAR)
          .set_output_prediction_type(prediction_type_t::SCALAR)
          .set_learn_returns_prediction(true)
          .build();

  return ret;
}
