/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include <iostream>
#ifndef WIN32
#include <strings.h>
#else
#include <string>
#endif
#include <stdexcept>
#include <sstream>

#include "parse_primitives.h"
#include "hash.h"
#include "vw_exception.h"

bool substring_equal(const substring& a, const substring& b)
{
  return (a.end - a.begin == b.end - b.begin)  // same length
      && (strncmp(a.begin, b.begin, a.end - a.begin) == 0);
}

bool substring_equal(const substring& ss, const char* str)
{
  size_t len_ss = ss.end - ss.begin;
  size_t len_str = strlen(str);
  if (len_ss != len_str)
    return false;
  return (strncmp(ss.begin, str, len_ss) == 0);
}

size_t substring_len(substring& s) { return s.end - s.begin; }

hash_func_t getHasher(const std::string& s)
{
  if (s == "strings")
    return hashstring;
  else if (s == "all")
    return hashall;
  else
    THROW("Unknown hash function: " << s);
}

std::ostream& operator<<(std::ostream& os, const substring& ss)
{
  std::string s(ss.begin, ss.end - ss.begin);
  return os << s;
}

std::ostream& operator<<(std::ostream& os, const v_array<substring>& ss)
{
  substring* it = ss.cbegin();

  if (it == ss.cend())
  {
    return os;
  }

  os << *it;

  for (it++; it != ss.cend(); it++)
  {
    os << ",";
    os << *it;
  }

  return os;
}
