// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

// This is a learner which does nothing with examples.  Used when VW is used as a compressor.

#include "noop.h"

#include "learner.h"
#include "config/options.h"

using namespace VW::config;

void learn(char&, VW::LEARNER::base_learner&, example&) {}

VW::LEARNER::base_learner* noop_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();

  bool noop = false;
  option_group_definition new_options("[Reduction] Noop Base Learner");
  new_options.add(make_option("noop", noop).keep().necessary().help("Do no learning"));

  if (!options.add_parse_and_check_necessary(new_options)) return nullptr;

  // While the learn function doesnt use anything, the implicit finish function expects scalar and simple.
  // This can change if we change the finish function.
  auto ret = VW::LEARNER::make_no_data_base_learner(
      learn, learn, stack_builder.get_setupfn_name(noop_setup), VW::prediction_type_t::scalar, VW::label_type_t::simple)
                 .build();
  return VW::LEARNER::make_base(*ret);
}
