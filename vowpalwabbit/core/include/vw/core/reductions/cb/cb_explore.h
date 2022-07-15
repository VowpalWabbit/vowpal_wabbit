// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once
#include "vw/core/cb.h"
#include "vw/core/vw_fwd.h"

namespace VW
{
namespace reductions
{
VW::LEARNER::base_learner* cb_explore_setup(VW::setup_base_i& stack_builder);
}
}  // namespace VW

// TODO: Move these functions either into a CB-related lib in VW:: or under VW::reductions::
namespace CB_EXPLORE
{
void generic_output_example(VW::workspace& all, float loss, VW::example& ec, CB::label& ld);
}