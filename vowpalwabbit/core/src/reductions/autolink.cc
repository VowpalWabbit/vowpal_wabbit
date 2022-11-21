// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/autolink.h"

#include "vw/config/options.h"
#include "vw/core/global_data.h"
#include "vw/core/learner.h"
#include "vw/core/setup_base.h"

#include <cstdint>

using namespace VW::config;

class autolink
{
public:
  autolink(uint32_t poly_degree, uint32_t stride_shift);
  void predict(VW::LEARNER::single_learner& base, VW::example& ec) const;
  void learn(VW::LEARNER::single_learner& base, VW::example& ec) const;

private:
  void prepare_example(VW::LEARNER::single_learner& base, VW::example& ec) const;
  static void reset_example(VW::example& ec);

  // degree of the polynomial
  const uint32_t _poly_degree;
  const uint32_t _stride_shift;
  static constexpr int AUTOCONSTANT = 524267083;
};

autolink::autolink(uint32_t poly_degree, uint32_t stride_shift) : _poly_degree(poly_degree), _stride_shift(stride_shift)
{
}

void autolink::predict(VW::LEARNER::single_learner& base, VW::example& ec) const
{
  prepare_example(base, ec);
  base.predict(ec);
  reset_example(ec);
}

void autolink::learn(VW::LEARNER::single_learner& base, VW::example& ec) const
{
  prepare_example(base, ec);
  base.learn(ec);
  reset_example(ec);
}

void autolink::prepare_example(VW::LEARNER::single_learner& base, VW::example& ec) const
{
  base.predict(ec);
  float base_pred = ec.pred.scalar;

  // Add features of label.
  ec.indices.push_back(VW::details::AUTOLINK_NAMESPACE);
  features& fs = ec.feature_space[VW::details::AUTOLINK_NAMESPACE];
  for (size_t i = 0; i < _poly_degree; i++)
  {
    if (base_pred != 0.f)
    {
      fs.push_back(base_pred, AUTOCONSTANT + (i << _stride_shift), VW::details::AUTOLINK_NAMESPACE);
      base_pred *= ec.pred.scalar;
    }
  }
  ec.reset_total_sum_feat_sq();
}

void autolink::reset_example(VW::example& ec)
{
  features& fs = ec.feature_space[VW::details::AUTOLINK_NAMESPACE];
  ec.reset_total_sum_feat_sq();
  fs.clear();
  ec.indices.pop_back();
}

template <bool is_learn>
void predict_or_learn(autolink& b, VW::LEARNER::single_learner& base, VW::example& ec)
{
  if (is_learn) { b.learn(base, ec); }
  else
  {
    b.predict(base, ec);
  }
}

struct options_autolink_v1
{
  uint32_t d;
};

std::unique_ptr<options_autolink_v1> get_autolink_options_instance(
    const VW::workspace&, VW::io::logger&, options_i& options)
{
  auto autolink_opts = VW::make_unique<options_autolink_v1>();
  option_group_definition new_options("[Reduction] Autolink");
  new_options.add(make_option("autolink", autolink_opts->d).keep().necessary().help("Create link function with polynomial d"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }
  return autolink_opts;
}

VW::LEARNER::base_learner* VW::reductions::autolink_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();
  auto autolink_opts = get_autolink_options_instance(all, all.logger, options);
  if (autolink_opts == nullptr) { return nullptr; }

  auto autolink_reduction = VW::make_unique<autolink>(autolink_opts->d, all.weights.stride_shift());
  auto* base = VW::LEARNER::as_singleline(stack_builder.setup_base_learner());
  auto* learner = VW::LEARNER::make_reduction_learner(std::move(autolink_reduction), base, predict_or_learn<true>,
      predict_or_learn<false>, stack_builder.get_setupfn_name(autolink_setup))
                      .set_output_prediction_type(VW::prediction_type_t::SCALAR)
                      .set_learn_returns_prediction(base->learn_returns_prediction)
                      .build();
  return make_base(*learner);
}
