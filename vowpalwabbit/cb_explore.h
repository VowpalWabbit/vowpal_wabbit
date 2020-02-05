// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once
#include "reductions_fwd.h"

namespace LEARNER
{
template <class T, class E>
struct learner;
}

LEARNER::base_learner* cb_explore_setup(VW::config::options_i& options, vw& all);
