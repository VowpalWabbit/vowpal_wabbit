// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include <cinttypes>
#include <string>

#include "config.h"

namespace VW
{
struct version_struct
{
  int32_t major;
  int32_t minor;
  int32_t rev;

  version_struct(int maj = 0, int min = 0, int rv = 0);
  version_struct(const char* v_str);
  version_struct(const version_struct& v);

  ~version_struct() = default;

  void operator=(const version_struct& v);
  void operator=(const char* v_str);

  bool operator==(const version_struct& v) const;
  bool operator==(const char* v_str) const;

  bool operator!=(const version_struct& v) const;
  bool operator!=(const char* v_str) const;

  bool operator>=(const version_struct& v) const;
  bool operator>=(const char* v_str) const;

  bool operator>(const version_struct& v) const;
  bool operator>(const char* v_str) const;

  bool operator<=(const version_struct& v) const;
  bool operator<=(const char* v_str) const;

  bool operator<(const version_struct& v) const;
  bool operator<(const char* v_str) const;

  std::string to_string() const;
  void from_string(const char* str);
};

extern const version_struct version;
extern const std::string git_commit;
}  // namespace VW
