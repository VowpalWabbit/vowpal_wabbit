// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
#include "vw/core/learner.h"

namespace VW
{
namespace reductions
{
// Setup reduction in stack
LEARNER::base_learner* get_pmf_setup(VW::setup_base_i& stack_builder);
}  // namespace reductions
}  // namespace VW
