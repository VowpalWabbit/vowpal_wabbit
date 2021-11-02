#pragma once
// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "reductions_fwd.h"

namespace VW
{
namespace binary
{
VW::LEARNER::base_learner* binary_setup(setup_base_i& stack_builder);
}
}  // namespace VW
