// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include <cstdint>

#include "v_array.h"

namespace VW
{
struct active_multiclass_prediction
{
  uint32_t predicted_class = 0;
  v_array<uint32_t> more_info_required_for_classes;
};
}  // namespace VW