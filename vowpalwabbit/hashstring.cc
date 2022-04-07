// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "hashstring.h"

#include "vw/common/vw_exception.h"

#include <string>

hash_func_t getHasher(const std::string& s)
{
  if (s == "strings") { return hashstring; }
  else if (s == "all")
  {
    return hashall;
  }
  else
    THROW("Unknown hash function: " << s);
}
