// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/count_label.h"

#include "vw/common/future_compat.h"
#include "vw/config/options.h"
#include "vw/core/best_constant.h"
#include "vw/core/example.h"
#include "vw/core/global_data.h"
#include "vw/core/learner.h"
#include "vw/core/memory.h"
#include "vw/core/parser.h"
#include "vw/core/setup_base.h"
#include "vw/io/logger.h"

namespace
{
class reduction_data
{
public:
  VW::workspace* all = nullptr;
  VW::LEARNER::learner* base = nullptr;

  explicit reduction_data(VW::workspace* all, VW::LEARNER::learner* base) : all(all), base(base) {}
};

template <bool is_learn>
void count_label_single(reduction_data& data, VW::LEARNER::learner& base, VW::example& ec)
{
  VW::shared_data* sd = data.all->sd.get();
  VW::count_label(*sd, ec.l.simple.label);

  if VW_STD17_CONSTEXPR (is_learn) { base.learn(ec); }
  else { base.predict(ec); }
}

template <bool is_learn>
void count_label_multi(reduction_data& data, VW::LEARNER::learner& base, VW::multi_ex& ec_seq)
{
  VW::shared_data* sd = data.all->sd.get();
  for (const auto* ex : ec_seq) { VW::count_label(*sd, ex->l.simple.label); }

  if VW_STD17_CONSTEXPR (is_learn) { base.learn(ec_seq); }
  else { base.predict(ec_seq); }
}
}  // namespace

std::shared_ptr<VW::LEARNER::learner> VW::reductions::count_label_setup(VW::setup_base_i& stack_builder)
{
  bool dont_output_best_constant = false;
  VW::config::option_group_definition reduction_options("[Reduction] Count label");
  reduction_options.add(VW::config::make_option("dont_output_best_constant", dont_output_best_constant)
                            .help("Don't track the best constant used in the output"));

  stack_builder.get_options()->add_and_parse(reduction_options);

  auto base = stack_builder.setup_base_learner();

  // When called to determine the name base will be nullptr and other `all` state will be nullptr.
  if (base == nullptr) { return nullptr; }

  auto* all = stack_builder.get_all_pointer();
  auto base_label_type = all->example_parser->lbl_parser.label_type;
  if (dont_output_best_constant)
  {
    if (base_label_type != label_type_t::SIMPLE)
    {
      all->logger.out_warn(
          "--dont_output_best_constant is not relevant. best constant is only tracked if the label type is simple.");
    }

    return base;
  }

  // TODO use field on base when that is available. In most reductions we would
  // return nullptr if the reduction is not active. However, in this reduction we
  // have already constructed the base. So we must return what we've already
  // constructed but it works because we aren't part of it
  if (base_label_type != label_type_t::SIMPLE) { return base; }

  auto data = VW::make_unique<reduction_data>(all, base.get());
  if (base->is_multiline())
  {
    auto learner = VW::LEARNER::make_reduction_learner(std::move(data), VW::LEARNER::require_multiline(base),
        count_label_multi<true>, count_label_multi<false>, stack_builder.get_setupfn_name(count_label_setup))
                       .set_learn_returns_prediction(base->learn_returns_prediction)
                       .set_output_prediction_type(base->get_output_prediction_type())
                       .set_input_label_type(label_type_t::SIMPLE)
                       .build();
    return learner;
  }

  auto learner = VW::LEARNER::make_reduction_learner(std::move(data), VW::LEARNER::require_singleline(base),
      count_label_single<true>, count_label_single<false>, stack_builder.get_setupfn_name(count_label_setup))
                     .set_learn_returns_prediction(base->learn_returns_prediction)
                     .set_input_prediction_type(base->get_output_prediction_type())
                     .set_output_prediction_type(base->get_output_prediction_type())
                     .set_input_label_type(label_type_t::SIMPLE)
                     .set_output_label_type(label_type_t::SIMPLE)
                     .build();
  return learner;
}
