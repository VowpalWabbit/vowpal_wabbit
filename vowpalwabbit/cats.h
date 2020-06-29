// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
#include "learner.h"
#include "options.h"

namespace VW { namespace continuous_action { namespace cats {

  LEARNER::base_learner* setup(config::options_i& options, vw& all);

}}}  // namespace VW
