// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
#include "vw/core/example_predict.h"
#include "vw/core/interactions.h"
#include "vw/core/vw_fwd.h"

#include <set>
#include <vector>

namespace VW
{
namespace reductions
{
VW::LEARNER::base_learner* generate_interactions_setup(VW::setup_base_i& stack_builder);
}
}  // namespace VW
