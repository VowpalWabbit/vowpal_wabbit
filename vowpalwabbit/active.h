// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <memory>
#include "reductions_fwd.h"
#include "rand_state.h"

struct active
{
  float active_c0 = 0.f;
  vw* all = nullptr;  // statistics, loss
  std::shared_ptr<rand_state> _random_state;
};

VW::LEARNER::base_learner* active_setup(VW::setup_base_i& stack_builder);
