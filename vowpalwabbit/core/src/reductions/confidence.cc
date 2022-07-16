// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/confidence.h"

#include "vw/config/options.h"
#include "vw/core/example.h"
#include "vw/core/global_data.h"
#include "vw/core/learner.h"
#include "vw/core/setup_base.h"
#include "vw/core/shared_data.h"
#include "vw/core/vw.h"
#include "vw/io/logger.h"

#include <cfloat>
#include <cmath>

using namespace VW::LEARNER;
using namespace VW::config;

namespace
{
struct confidence
{
  VW::workspace* all = nullptr;
};

template <bool is_learn, bool is_confidence_after_training>
void predict_or_learn_with_confidence(confidence& /* c */, single_learner& base, VW::example& ec)
{
  float threshold = 0.f;
  float sensitivity = 0.f;

  float existing_label = ec.l.simple.label;
  if (existing_label == FLT_MAX)
  {
    base.predict(ec);
    float opposite_label = 1.f;
    if (ec.pred.scalar > 0) { opposite_label = -1.f; }
    ec.l.simple.label = opposite_label;
  }

  if (!is_confidence_after_training) { sensitivity = base.sensitivity(ec); }

  ec.l.simple.label = existing_label;
  if (is_learn) { base.learn(ec); }
  else
  {
    base.predict(ec);
  }

  if (is_confidence_after_training) { sensitivity = base.sensitivity(ec); }

  ec.confidence = fabsf(ec.pred.scalar - threshold) / sensitivity;
}

void confidence_print_result(
    VW::io::writer* f, float res, float confidence, const VW::v_array<char>& tag, VW::io::logger& logger)
{
  if (f != nullptr)
  {
    std::stringstream ss;
    ss << std::fixed << res << " " << confidence;
    ss << " ";
    if (!tag.empty()) { ss << VW::string_view{tag.begin(), tag.size()}; }
    ss << '\n';
    // avoid serializing the stringstream multiple times
    auto ss_string(ss.str());
    ssize_t len = ss_string.size();
    ssize_t t = f->write(ss_string.c_str(), static_cast<unsigned int>(len));
    if (t != len) { logger.err_error("write error: {}", VW::strerror_to_string(errno)); }
  }
}

void output_and_account_confidence_example(VW::workspace& all, VW::example& ec)
{
  label_data& ld = ec.l.simple;

  all.sd->update(ec.test_only, ld.label != FLT_MAX, ec.loss, ec.weight, ec.get_num_features());
  if (ld.label != FLT_MAX && !ec.test_only)
  { all.sd->weighted_labels += static_cast<double>(ld.label) * static_cast<double>(ec.weight); }
  all.sd->weighted_unlabeled_examples += ld.label == FLT_MAX ? ec.weight : 0;

  all.print_by_ref(all.raw_prediction.get(), ec.partial_prediction, -1, ec.tag, all.logger);
  for (const auto& sink : all.final_prediction_sink)
  { confidence_print_result(sink.get(), ec.pred.scalar, ec.confidence, ec.tag, all.logger); }

  print_update(all, ec);
}

void return_confidence_example(VW::workspace& all, confidence& /* c */, VW::example& ec)
{
  output_and_account_confidence_example(all, ec);
  VW::finish_example(all, ec);
}

}  // namespace

base_learner* VW::reductions::confidence_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();
  bool confidence_arg = false;
  bool confidence_after_training = false;
  option_group_definition new_options("[Reduction] Confidence");
  new_options
      .add(make_option("confidence", confidence_arg).keep().necessary().help("Get confidence for binary predictions"))
      .add(make_option("confidence_after_training", confidence_after_training).help("Confidence after training"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  if (!all.training)
  {
    all.logger.out_warn(
        "Confidence does not work in test mode because learning algorithm state is needed.  Do not use "
        "--predict_only_model "
        "when "
        "saving the model and avoid --test_only");
    return nullptr;
  }

  auto data = VW::make_unique<confidence>();
  data->all = &all;

  void (*learn_with_confidence_ptr)(confidence&, single_learner&, VW::example&) = nullptr;
  void (*predict_with_confidence_ptr)(confidence&, single_learner&, VW::example&) = nullptr;

  if (confidence_after_training)
  {
    learn_with_confidence_ptr = predict_or_learn_with_confidence<true, true>;
    predict_with_confidence_ptr = predict_or_learn_with_confidence<false, true>;
  }
  else
  {
    learn_with_confidence_ptr = predict_or_learn_with_confidence<true, false>;
    predict_with_confidence_ptr = predict_or_learn_with_confidence<false, false>;
  }

  auto base = as_singleline(stack_builder.setup_base_learner());

  // Create new learner
  auto* l = make_reduction_learner(std::move(data), base, learn_with_confidence_ptr, predict_with_confidence_ptr,
      stack_builder.get_setupfn_name(confidence_setup))
                .set_learn_returns_prediction(true)
                .set_input_label_type(VW::label_type_t::simple)
                .set_output_prediction_type(VW::prediction_type_t::scalar)
                .set_finish_example(return_confidence_example)
                .build();

  return make_base(*l);
}
