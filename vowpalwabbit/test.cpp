// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "example.h"
#include "global_data.h"
#include "learner.h"
#include "memory.h"
#include "options.h"
#include "vw.h"

#include <string>

struct reduction_data
{
};

void predict(reduction_data& data, VW::LEARNER::multi_learner& base, multi_ex& ec_seq) {}

void learn(reduction_data& data, VW::LEARNER::multi_learner& base, multi_ex& ec_seq) {}

void finish_example(vw& all, reduction_data& data, multi_ex& ec) { VW::finish_example(all, ec); }

VW::LEARNER::base_learner* reduction_setup(VW::setup_base_i& stack_builder)
{
  VW::config::options_i& options = *stack_builder.get_options();
  std::string arg;

  VW::config::option_group_definition reduction_options("Reduction options");
  reduction_options.add(VW::config::make_option("reduction", arg).keep().necessary().help(""));
  if (!options.add_parse_and_check_necessary(reduction_options)) { return nullptr; }

  auto data = VW::make_unique<reduction_data>();
  auto* base = VW::LEARNER::as_multiline(stack_builder.setup_base_learner());
  auto* learner = VW::LEARNER::make_reduction_learner(
      std::move(data), base, learn, predict, stack_builder.get_setupfn_name(reduction_setup))
                      .set_prediction_type(prediction_type_t::scalar)
                      .set_label_type(label_type_t::simple)
                      .set_finish_example(finish_example)
                      .build();

  return VW::LEARNER::make_base(*learner);
}
