// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "reductions_fwd.h"

#include <vector>
#include <memory>

namespace VW
{
namespace cb_explore_adf
{
namespace synthcover
{
VW::LEARNER::base_learner* setup(VW::setup_base_i& stack_builder);
}  // namespace synthcover
}  // namespace cb_explore_adf
}  // namespace VW
