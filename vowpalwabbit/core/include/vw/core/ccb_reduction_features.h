// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/common/future_compat.h"
#include "vw/core/v_array.h"

#include <cstdint>

namespace VW
{
enum class ccb_example_type : uint8_t
{
  UNSET = 0,
  SHARED = 1,
  ACTION = 2,
  SLOT = 3
};

class ccb_reduction_features
{
public:
  ccb_example_type type;
  VW::v_array<uint32_t> explicit_included_actions;
  void clear() { explicit_included_actions.clear(); }
};
}  // namespace VW

namespace VW
{
const char* to_string(ccb_example_type type);
}  // namespace VW

namespace CCB  // NOLINT
{
using example_type VW_DEPRECATED(
    "VW::ccb_example_type renamed to VW::ccb_example_type. VW::ccb_example_type will be removed in VW 10.") =
    VW::ccb_example_type;
using reduction_features VW_DEPRECATED(
    "CCB::reduction_features renamed to VW::ccb_reduction_features. CCB::reduction_features will be removed in VW "
    "10.") = VW::ccb_reduction_features;
}  // namespace CCB
