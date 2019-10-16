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

hash_func_t getHasher(const std::string& s)
{
  if (s == "strings")
    return hashstring;
  else if (s == "all")
    return hashall;
  else
    THROW("Unknown hash function: " << s);
}

std::ostream& operator<<(std::ostream& os, const v_array<VW::string_view>& ss)
{
  VW::string_view* it = ss.cbegin();

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
