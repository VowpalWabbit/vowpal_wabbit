// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw_fwd.h"

#include <memory>
#include <vector>

namespace VW
{
namespace cb_explore_adf
{
namespace bag
{
VW::LEARNER::base_learner* setup(VW::setup_base_i& stack_builder);
}  // namespace bag
}  // namespace cb_explore_adf
}  // namespace VW
