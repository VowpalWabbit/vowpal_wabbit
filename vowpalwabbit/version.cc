// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "version.h"

#include <cstdio>

namespace VW
{
version_struct::version_struct(int maj, int min, int rv)
{
  major = maj;
  minor = min;
  rev = rv;
}

version_struct::version_struct(const char* v_str) { from_string(v_str); }

version_struct::version_struct(const version_struct& v)
{
  major = v.major;
  minor = v.minor;
  rev = v.rev;
}

void version_struct::operator=(const version_struct& v)
{
  major = v.major;
  minor = v.minor;
  rev = v.rev;
}

void version_struct::operator=(const char* v_str) { from_string(v_str); }

bool version_struct::operator==(const version_struct& v) const
{
  return (major == v.major && minor == v.minor && rev == v.rev);
}

bool version_struct::operator==(const char* v_str) const
{
  version_struct v_tmp(v_str);
  return (*this == v_tmp);
}

bool version_struct::operator!=(const version_struct& v) const { return !(*this == v); }

bool version_struct::operator!=(const char* v_str) const { return !(*this == v_str); }

bool version_struct::operator>=(const version_struct& v) const
{
  if (major < v.major)
    return false;
  if (major > v.major)
    return true;
  if (minor < v.minor)
    return false;
  if (minor > v.minor)
    return true;
  if (rev >= v.rev)
    return true;
  return false;
}

bool version_struct::operator>=(const char* v_str) const
{
  version_struct v_tmp(v_str);
  return (*this >= v_tmp);
}

bool version_struct::operator>(const version_struct& v) const
{
  if (major < v.major)
    return false;
  if (major > v.major)
    return true;
  if (minor < v.minor)
    return false;
  if (minor > v.minor)
    return true;
  if (rev > v.rev)
    return true;
  return false;
}

bool version_struct::operator>(const char* v_str) const
{
  version_struct v_tmp(v_str);
  return (*this > v_tmp);
}

bool version_struct::operator<=(const version_struct& v) const { return !(*this > v); }

bool version_struct::operator<=(const char* v_str) const { return !(*this > v_str); }

bool version_struct::operator<(const version_struct& v) const { return !(*this >= v); }

bool version_struct::operator<(const char* v_str) const { return !(*this >= v_str); }

std::string version_struct::to_string() const
{
  // int32_t has up to 10 digits, base-10.
  // 3 * 30 + 2 = 92 => 128
  char v_str[128];

  std::snprintf(v_str, sizeof(v_str), "%d.%d.%d", major, minor, rev);
  std::string s = v_str;
  return s;
}

void version_struct::from_string(const char* str)
{
#ifdef _WIN32
  sscanf_s(str, "%d.%d.%d", &major, &minor, &rev);
#else
  std::sscanf(str, "%d.%d.%d", &major, &minor, &rev);
#endif
}

const version_struct version(PACKAGE_VERSION);
const std::string git_commit(COMMIT_VERSION);
}  // namespace VW
