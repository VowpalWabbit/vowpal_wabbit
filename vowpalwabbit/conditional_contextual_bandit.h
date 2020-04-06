// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <cstdint>
#include <vector>
#include <string>

#include "reductions_fwd.h"
#include "action_score.h"

namespace CCB
{
void calculate_and_insert_interactions(
    example* shared, std::vector<example*> actions, std::vector<std::string>& generated_interactions);

LEARNER::base_learner* ccb_explore_adf_setup(VW::config::options_i& options, vw& all);
bool ec_is_example_header(example const& ec);
std::string generate_ccb_label_printout(const std::vector<example*>& slots);

}  // namespace CCB
