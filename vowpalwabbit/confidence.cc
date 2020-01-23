// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "reductions.h"
#include "vw.h"
#include "math.h"

using namespace LEARNER;

using namespace VW::config;

struct confidence
{
  vw* all;
};

template <bool is_learn, bool is_confidence_after_training>
void predict_or_learn_with_confidence(confidence& /* c */, single_learner& base, example& ec)
{
  float threshold = 0.f;
  float sensitivity = 0.f;

  float existing_label = ec.l.simple.label;
  if (existing_label == FLT_MAX)
  {
    base.predict(ec);
    float opposite_label = 1.f;
    if (ec.pred.scalar > 0)
      opposite_label = -1.f;
    ec.l.simple.label = opposite_label;
  }

  if (!is_confidence_after_training)
    sensitivity = base.sensitivity(ec);

  ec.l.simple.label = existing_label;
  if (is_learn)
    base.learn(ec);
  else
    base.predict(ec);

  if (is_confidence_after_training)
    sensitivity = base.sensitivity(ec);

  ec.confidence = fabsf(ec.pred.scalar - threshold) / sensitivity;
}

void confidence_print_result(int f, float res, float confidence, v_array<char> tag)
{
  if (f >= 0)
  {
    std::stringstream ss;
    ss << std::fixed << res << " " << confidence;
    if (!print_tag_by_ref(ss, tag))
      ss << ' ';
    ss << '\n';
    ssize_t len = ss.str().size();
    ssize_t t = io_buf::write_file_or_socket(f, ss.str().c_str(), (unsigned int)len);
    if (t != len)
      std::cerr << "write error: " << strerror(errno) << std::endl;
  }
}

void output_and_account_confidence_example(vw& all, example& ec)
{
  label_data& ld = ec.l.simple;

  all.sd->update(ec.test_only, ld.label != FLT_MAX, ec.loss, ec.weight, ec.num_features);
  if (ld.label != FLT_MAX && !ec.test_only)
    all.sd->weighted_labels += ld.label * ec.weight;
  all.sd->weighted_unlabeled_examples += ld.label == FLT_MAX ? ec.weight : 0;

  all.print_by_ref(all.raw_prediction, ec.partial_prediction, -1, ec.tag);
  for (size_t i = 0; i < all.final_prediction_sink.size(); i++)
  {
    int f = (int)all.final_prediction_sink[i];
    confidence_print_result(f, ec.pred.scalar, ec.confidence, ec.tag);
  }

  print_update(all, ec);
}

void return_confidence_example(vw& all, confidence& /* c */, example& ec)
{
  output_and_account_confidence_example(all, ec);
  VW::finish_example(all, ec);
}

base_learner* confidence_setup(options_i& options, vw& all)
{
  bool confidence_arg = false;
  bool confidence_after_training = false;
  option_group_definition new_options("Confidence");
  new_options.add(make_option("confidence", confidence_arg).keep().help("Get confidence for binary predictions"))
      .add(make_option("confidence_after_training", confidence_after_training).help("Confidence after training"));
  options.add_and_parse(new_options);

  if (!confidence_arg)
    return nullptr;

  if (!all.training)
  {
    std::cout
        << "Confidence does not work in test mode because learning algorithm state is needed.  Use --save_resume when "
           "saving the model and avoid --test_only"
        << std::endl;
    return nullptr;
  }

  auto data = scoped_calloc_or_throw<confidence>();
  data->all = &all;

  void (*learn_with_confidence_ptr)(confidence&, single_learner&, example&) = nullptr;
  void (*predict_with_confidence_ptr)(confidence&, single_learner&, example&) = nullptr;

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

  // Create new learner
  learner<confidence, example>& l = init_learner(
      data, as_singleline(setup_base(options, all)), learn_with_confidence_ptr, predict_with_confidence_ptr);

  l.set_finish_example(return_confidence_example);

  return make_base(l);
}
