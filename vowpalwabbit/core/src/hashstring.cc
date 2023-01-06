// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/hashstring.h"

#include "vw/common/vw_exception.h"
#include "vw/common/vw_throw.h"

#include <string>

VW::hash_func_t get_hasher(const std::string& s) { return VW::get_hasher(s); }

VW::hash_func_t VW::get_hasher(const std::string& s)
{
  if (s == "strings") { return VW::details::hashstring; }
  else if (s == "all") { return VW::details::hashall; }
  else
    THROW("Unknown hash function: " << s);
}
