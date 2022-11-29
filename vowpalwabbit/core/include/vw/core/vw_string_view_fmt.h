// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

// Anything that wants to call fmt::join on a container using VW::string_view must include this file
// In reality this functionality should live within vw_string_view.h, but due to dependency issues
// we can't expose spdlog or fmtlib outside of the VW project. So until we get a proper API surface
// implemented, we'll need to be strict about our include chains

// This file should NOT be included in other header files.
#pragma once
#include "vw/common/string_view.h"

#include <fmt/core.h>
#include <fmt/format.h>

namespace fmt
{
// Enable VW::string_view in fmt calls (uses the fmt::string_view formatter underneath)
template <>
class formatter<VW::string_view> : public formatter<fmt::string_view>
{
public:
#if FMT_VERSION >= 90000
  template <typename FormatContext>
  auto format(const VW::string_view& sv, FormatContext& ctx) const -> decltype(ctx.out())
#else
  template <typename FormatContext>
  auto format(const VW::string_view& sv, FormatContext& ctx) -> decltype(ctx.out())
#endif
  {
    return formatter<fmt::string_view>::format({sv.data(), sv.size()}, ctx);
  }
};
}  // namespace fmt
