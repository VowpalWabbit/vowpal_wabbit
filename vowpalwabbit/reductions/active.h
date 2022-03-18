// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "rand_state.h"
#include "reductions_fwd.h"
#include "shared_data.h"
#include "version.h"

#include <memory>
#include <utility>

struct active
{
  active(float active_c0, shared_data* shared_data, std::shared_ptr<VW::rand_state> random_state,
      VW::version_struct model_version)
      : active_c0(active_c0)
      , _shared_data(shared_data)
      , _random_state(std::move(random_state))
      , _model_version{std::move(model_version)}
  {
  }

  float active_c0;
  shared_data* _shared_data;  // statistics, loss
  std::shared_ptr<VW::rand_state> _random_state;

  float _min_seen_label = 0.f;
  float _max_seen_label = 1.f;
  VW::version_struct _model_version;
};

VW::LEARNER::base_learner* active_setup(VW::setup_base_i& stack_builder);
