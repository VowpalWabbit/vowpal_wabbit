// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/count_label.h"

#include "vw/core/best_constant.h"
#include "vw/core/example.h"
#include "vw/core/global_data.h"
#include "vw/core/learner.h"
#include "vw/core/memory.h"
#include "vw/core/parser.h"
#include "vw/core/setup_base.h"
#include "vw/common/future_compat.h"
#include "vw/config/options.h"
#include "vw/io/logger.h"

namespace
{
struct reduction_data
{
  shared_data* _sd;
  VW::LEARNER::base_learner* _base;

  explicit reduction_data(shared_data* sd, VW::LEARNER::base_learner* base) : _sd(sd), _base(base) {}
};

template <bool is_learn>
void count_label_single(reduction_data& data, VW::LEARNER::single_learner& base, VW::example& ec)
{
  VW::count_label(*data._sd, ec.l.simple.label);

  if VW_STD17_CONSTEXPR (is_learn) { base.learn(ec); }
  else
  {
    base.predict(ec);
  }
}

template <bool is_learn>
void count_label_multi(reduction_data& data, VW::LEARNER::multi_learner& base, VW::multi_ex& ec_seq)
{
  for (const auto* ex : ec_seq) { VW::count_label(*data._sd, ex->l.simple.label); }

  if VW_STD17_CONSTEXPR (is_learn) { base.learn(ec_seq); }
  else
  {
    base.predict(ec_seq);
  }
}

// This reduction must delegate finish to the one it is above as this is just a utility counter.
void finish_example_multi(VW::workspace& all, reduction_data& data, VW::multi_ex& ec)
{
  VW::LEARNER::as_multiline(data._base)->finish_example(all, ec);
}
void finish_example_single(VW::workspace& all, reduction_data& data, VW::example& ec)
{
  VW::LEARNER::as_singleline(data._base)->finish_example(all, ec);
}
}  // namespace

VW::LEARNER::base_learner* VW::reductions::count_label_setup(VW::setup_base_i& stack_builder)
{
  bool dont_output_best_constant = false;
  VW::config::option_group_definition reduction_options("[Reduction] Count label");
  reduction_options.add(VW::config::make_option("dont_output_best_constant", dont_output_best_constant)
                            .help("Don't track the best constant used in the output"));

  stack_builder.get_options()->add_and_parse(reduction_options);

  auto* base = stack_builder.setup_base_learner();

  // When called to determine the name base will be nullptr and other `all` state will be nullptr.
  if (base == nullptr) { return nullptr; }

  auto* all = stack_builder.get_all_pointer();
  auto base_label_type = all->example_parser->lbl_parser.label_type;
  if (dont_output_best_constant)
  {
    if (base_label_type != label_type_t::simple)
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
  if (base_label_type != label_type_t::simple) { return base; }

  auto data = VW::make_unique<reduction_data>(all->sd, base);
  if (base->is_multiline())
  {
    auto* learner = VW::LEARNER::make_reduction_learner(std::move(data), VW::LEARNER::as_multiline(base),
        count_label_multi<true>, count_label_multi<false>, stack_builder.get_setupfn_name(count_label_setup))
                        .set_learn_returns_prediction(base->learn_returns_prediction)
                        .set_output_prediction_type(base->get_output_prediction_type())
                        .set_input_label_type(label_type_t::simple)
                        .set_finish_example(finish_example_multi)
                        .build();
    return VW::LEARNER::make_base(*learner);
  }

  auto* learner = VW::LEARNER::make_reduction_learner(std::move(data), VW::LEARNER::as_singleline(base),
      count_label_single<true>, count_label_single<false>, stack_builder.get_setupfn_name(count_label_setup))
                      .set_learn_returns_prediction(base->learn_returns_prediction)
                      .set_output_prediction_type(base->get_output_prediction_type())
                      .set_input_label_type(label_type_t::simple)
                      .set_finish_example(finish_example_single)
                      .build();
  return VW::LEARNER::make_base(*learner);
}
