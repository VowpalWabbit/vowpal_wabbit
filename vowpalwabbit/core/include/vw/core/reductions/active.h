// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/core/version.h"
#include "vw/core/vw_fwd.h"

#include <memory>

namespace VW
{
namespace reductions
{
class active
{
public:
  active(float active_c0, std::shared_ptr<shared_data> shared_data, std::shared_ptr<rand_state> random_state)
      : active_c0(active_c0)
      , _shared_data(shared_data)
      , _random_state(std::move(random_state))
  {
  }

  float active_c0;
  std::shared_ptr<shared_data> _shared_data;  // statistics, loss
  std::shared_ptr<rand_state> _random_state;

  float _min_seen_label = 0.f;
  float _max_seen_label = 1.f;
};

std::shared_ptr<VW::LEARNER::learner> active_setup(VW::setup_base_i& stack_builder);
}  // namespace reductions
}  // namespace VW
