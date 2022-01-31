// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "v_array.h"
#include "fmt/format.h"

#include <cstdint>

namespace CCB
{
enum class example_type : uint8_t
{
  unset = 0,
  shared = 1,
  action = 2,
  slot = 3
};

struct reduction_features
{
  example_type type;
  v_array<uint32_t> explicit_included_actions;
  void clear() { explicit_included_actions.clear(); }
};
}  // namespace CCB

namespace VW
{
const char* to_string(CCB::example_type type);
}  // namespace VW

template <>
struct fmt::formatter<CCB::example_type> : formatter<std::string>
{
  auto format(CCB::example_type c, format_context& ctx) -> decltype(ctx.out())
  {
    return formatter<std::string>::format(std::string{VW::to_string(c)}, ctx);
  }
};