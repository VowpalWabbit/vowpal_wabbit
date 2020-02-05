// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "reductions_fwd.h"

LEARNER::base_learner* baseline_setup(VW::config::options_i& options, vw& all);

namespace BASELINE
{
// utility functions for disabling baseline on a given example
void set_baseline_enabled(example* ec);
void reset_baseline_disabled(example* ec);
bool baseline_enabled(example* ec);
}  // namespace BASELINE
