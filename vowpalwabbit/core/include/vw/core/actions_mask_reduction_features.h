// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/core/v_array.h"
#include "vw/core/vw_fwd.h"

#include <vector>

namespace VW
{
namespace cb_explore_adf
{
namespace actions_mask
{
struct reduction_features
{
  VW::v_array<uint32_t> action_mask;

  reduction_features() = default;

  void reset_to_default() { action_mask.clear(); }
};
}  // namespace actions_mask
}  // namespace cb_explore_adf
}  // namespace VW
