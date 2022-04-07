// Copyright (c) by respective owners including Yahoo!)
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "cb_type.h"

#include "vw/common/vw_exception.h"

#include <cassert>

#define CASE(type) \
  case type:       \
    return #type;

VW::string_view VW::to_string(VW::cb_type_t label_type)
{
  using namespace VW;
  switch (label_type)
  {
    CASE(cb_type_t::dr)
    CASE(cb_type_t::dm)
    CASE(cb_type_t::ips)
    CASE(cb_type_t::mtr)
    CASE(cb_type_t::sm)
  }

  // The above enum is exhaustive and will warn on a new label type being added due to the lack of `default`
  // The following is required by the compiler, otherwise it things control can reach the end of this function without
  // returning.
  assert(false);
  return "unknown label type enum";
}

VW::cb_type_t VW::cb_type_from_string(VW::string_view str)
{
  if (str == "dr") { return VW::cb_type_t::dr; }
  if (str == "dm") { return VW::cb_type_t::dm; }
  if (str == "ips") { return VW::cb_type_t::ips; }
  if (str == "mtr") { return VW::cb_type_t::mtr; }
  if (str == "sm") { return VW::cb_type_t::sm; }
  THROW("Unknown cb_type: " << str);
}