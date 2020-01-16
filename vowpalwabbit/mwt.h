// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once
#include "reductions_fwd.h"

LEARNER::base_learner* mwt_setup(VW::config::options_i& options, vw& all);

namespace MWT
{
void delete_scalars(void* v);
void print_scalars(int f, v_array<float>& scalars, v_array<char>& tag);
}  // namespace MWT
