// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "vw_fwd.h"

namespace VW
{
struct workspace;
}

namespace VW
{
namespace shared_feature_merger
{
VW::LEARNER::base_learner* shared_feature_merger_setup(VW::setup_base_i& stack_builder);

}  // namespace shared_feature_merger
}  // namespace VW
