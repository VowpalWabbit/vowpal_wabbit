// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/core/vw_fwd.h"

#include <cstdint>
#include <string>
#include <vector>

namespace VW
{
namespace reductions
{
VW::LEARNER::base_learner* ccb_explore_adf_setup(VW::setup_base_i& stack_builder);

namespace ccb
{
void insert_ccb_interactions(std::vector<std::vector<VW::namespace_index>>&, std::vector<std::vector<extent_term>>&);
bool ec_is_example_header(VW::example const& ec);
bool ec_is_example_unset(VW::example const& ec);
std::string generate_ccb_label_printout(const VW::multi_ex& slots);
}  // namespace ccb
}  // namespace reductions
}  // namespace VW
