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
  active(float active_c0, VW::workspace* all) : active_c0(active_c0), _all(all) {}

  float active_c0;
  VW::workspace* _all = nullptr;

  float _min_seen_label = 0.f;
  float _max_seen_label = 1.f;
  VW::version_struct _model_version;
};

std::shared_ptr<VW::LEARNER::learner> active_setup(VW::setup_base_i& stack_builder);
}  // namespace reductions
}  // namespace VW
