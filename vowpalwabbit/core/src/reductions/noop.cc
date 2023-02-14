// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

// This is a learner which does nothing with examples.  Used when VW is used as a compressor.

#include "vw/core/reductions/noop.h"

#include "vw/config/options.h"
#include "vw/core/learner.h"
#include "vw/core/setup_base.h"

using namespace VW::config;
namespace
{
void learn(char&, VW::example&) {}

}  // namespace

std::shared_ptr<VW::LEARNER::learner> VW::reductions::noop_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();

  bool noop = false;
  option_group_definition new_options("[Reduction] Noop Base Learner");
  new_options.add(make_option("noop", noop).keep().necessary().help("Do no learning"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  // While the learn function doesnt use anything, the implicit finish function expects scalar and simple.
  // This can change if we change the finish function.
  auto ret = VW::LEARNER::make_no_data_bottom_learner(
      learn, learn, stack_builder.get_setupfn_name(noop_setup), VW::prediction_type_t::SCALAR, VW::label_type_t::SIMPLE)
                 .set_output_example_prediction(VW::details::output_example_prediction_simple_label<char>)
                 .set_update_stats(VW::details::update_stats_simple_label<char>)
                 .set_print_update(VW::details::print_update_simple_label<char>)
                 .build();
  return ret;
}
