// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "vw/common/future_compat.h"
#include "vw/core/config.h"

#include <cinttypes>
#include <string>

namespace VW
{
class version_struct
{
public:
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

constexpr version_struct VERSION(VW_VERSION_MAJOR, VW_VERSION_MINOR, VW_VERSION_PATCH);
VW_DEPRECATED("VW::version renamed to VW::VERSION")
constexpr version_struct version(VW_VERSION_MAJOR, VW_VERSION_MINOR, VW_VERSION_PATCH);  // NOLINT

extern const std::string GIT_COMMIT;
VW_DEPRECATED("VW::git_commit renamed to VW::GIT_COMMIT")
extern const std::string git_commit;  // NOLINT
}  // namespace VW
