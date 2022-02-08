// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once
#include "reductions_fwd.h"

#include <ostream>

VW::LEARNER::base_learner* mwt_setup(VW::setup_base_i& stack_builder);

namespace MWT
{
void print_scalars(std::ostream& f, v_array<float>& scalars, v_array<char>& tag);
}  // namespace MWT
