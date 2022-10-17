// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/ccb_reduction_features.h"

const char* VW::to_string(VW::ccb_example_type ex_type)
{
#define CASE(type) \
  case type:       \
    return #type;

  using namespace VW;
  switch (ex_type)
  {
    CASE(ccb_example_type::UNSET)
    CASE(ccb_example_type::SHARED)
    CASE(ccb_example_type::ACTION)
    CASE(ccb_example_type::SLOT)
  }

  // The above enum is exhaustive and will warn on a new label type being added due to the lack of `default`
  // The following is required by the compiler, otherwise it things control can reach the end of this function without
  // returning.
  assert(false);
  return "unknown example_type enum";

#undef CASE
}
