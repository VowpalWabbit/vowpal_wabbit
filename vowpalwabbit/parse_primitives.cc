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

uint64_t hashstring(substring s, uint64_t h)
{
  // trim leading whitespace but not UTF-8
  for (; s.begin < s.end && *(s.begin) <= 0x20 && (int)*(s.begin) >= 0; s.begin++)
    ;
  // trim trailing white space but not UTF-8
  for (; s.end > s.begin && *(s.end - 1) <= 0x20 && (int)*(s.end - 1) >= 0; s.end--)
    ;

  size_t ret = 0;
  char* p = s.begin;
  while (p != s.end)
    if (*p >= '0' && *p <= '9')
      ret = 10 * ret + *(p++) - '0';
    else
      return uniform_hash((unsigned char*)s.begin, s.end - s.begin, h);

  return ret + h;
}

uint64_t hashall(substring s, uint64_t h) { return uniform_hash((unsigned char*)s.begin, s.end - s.begin, h); }

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
