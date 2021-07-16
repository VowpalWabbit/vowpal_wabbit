// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "v_array.h"

#include <cstdint>

namespace CCB {
enum example_type : uint8_t
{
  unset = 0,
  shared = 1,
  action = 2,
  slot = 3
};

struct reduction_features
{
  example_type type = CCB::example_type::unset;
  v_array<uint32_t> explicit_included_actions;
  void clear()
  {
    type = CCB::example_type::unset;
    explicit_included_actions.clear();
  }
};
}
