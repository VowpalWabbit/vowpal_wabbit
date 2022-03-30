// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "config.h"
#include "future_compat.h"

#include <cinttypes>
#include <string>

namespace VW
{
struct version_struct
{
  int32_t major;
  int32_t minor;
  int32_t rev;

  constexpr version_struct(int maj = 0, int min = 0, int rv = 0) : major{maj}, minor{min}, rev{rv} {}
  explicit version_struct(const char* v_str);

  constexpr version_struct(const version_struct& other) = default;
  constexpr version_struct(version_struct&& other) noexcept = default;
  VW_STD14_CONSTEXPR version_struct& operator=(const version_struct& other) = default;
  VW_STD14_CONSTEXPR version_struct& operator=(version_struct&& other) noexcept = default;

  constexpr bool operator==(const version_struct& v) const
  {
    return (major == v.major && minor == v.minor && rev == v.rev);
  }
  constexpr bool operator!=(const version_struct& v) const { return !(*this == v); }

  constexpr bool operator<(const version_struct& v) const
  {
    return (major < v.major) || (major == v.major && minor < v.minor) ||
        (major == v.major && minor == v.minor && rev < v.rev);
  }
  constexpr bool operator>=(const version_struct& v) const { return !(*this < v); }
  constexpr bool operator>(const version_struct& v) const { return (v < *this); }
  constexpr bool operator<=(const version_struct& v) const { return !(v < *this); }

  std::string to_string() const;
  static version_struct from_string(const char* str);
};

constexpr version_struct version(VW_VERSION_MAJOR, VW_VERSION_MINOR, VW_VERSION_PATCH);
extern const std::string git_commit;
}  // namespace VW
