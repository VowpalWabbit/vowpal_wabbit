// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "learner_no_throw.h"

// re-implement for slim TODO: fix
void return_simple_example(vw& all, void*, example& ec) {}
namespace VW
{
namespace LEARNER
{
float recur_sensitivity(void*, base_learner& base, example& ec) { return base.sensitivity(ec); }
}
}