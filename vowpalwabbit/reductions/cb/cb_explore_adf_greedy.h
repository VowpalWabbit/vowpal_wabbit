// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include <vector>

#include "cb_explore_adf_common.h"
#include "reductions_fwd.h"

namespace VW
{
namespace cb_explore_adf
{
namespace greedy
{
VW::LEARNER::base_learner* setup(VW::setup_base_i& stack_builder);
}  // namespace greedy
}  // namespace cb_explore_adf
}  // namespace VW
