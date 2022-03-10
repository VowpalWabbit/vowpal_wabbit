// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once
#include "cb.h"
#include "reductions_fwd.h"

VW::LEARNER::base_learner* cb_explore_setup(VW::setup_base_i& stack_builder);

namespace CB_EXPLORE
{
void generic_output_example(VW::workspace& all, float loss, example& ec, CB::label& ld);
}