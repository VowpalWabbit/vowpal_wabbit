// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <memory>
#include "reductions_fwd.h"

struct rand_state;

struct active
{
  float active_c0;
  vw* all;  // statistics, loss
  std::shared_ptr<rand_state> _random_state;
};

LEARNER::base_learner* active_setup(VW::config::options_i& options, vw& all);
