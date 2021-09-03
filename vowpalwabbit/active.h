// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <memory>
#include "reductions_fwd.h"
#include "rand_state.h"

struct active
{
  active(float active_c0, shared_data* shared_data, std::shared_ptr<rand_state> random_state)
      : active_c0(active_c0), _shared_data(shared_data), _random_state(std::move(random_state))
  {
  }

  float active_c0;
  shared_data* _shared_data;  // statistics, loss
  std::shared_ptr<rand_state> _random_state;
};

VW::LEARNER::base_learner* active_setup(VW::setup_base_i& stack_builder);
