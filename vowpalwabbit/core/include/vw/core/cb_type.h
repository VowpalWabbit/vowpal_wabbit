// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/common/string_view.h"

#include <cstdint>

namespace VW
{
enum class cb_type_t : uint32_t
{
  DR,
  DM,
  IPS,
  MTR,
  SM
};

cb_type_t cb_type_from_string(string_view str);
string_view to_string(cb_type_t type);
}  // namespace VW
