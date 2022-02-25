// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "ccb_reduction_features.h"

const char* VW::to_string(CCB::example_type ex_type)
{
#define CASE(type) \
  case type:       \
    return #type;

  using namespace CCB;
  switch (ex_type)
  {
    CASE(example_type::unset)
    CASE(example_type::shared)
    CASE(example_type::action)
    CASE(example_type::slot)
  }

  // The above enum is exhaustive and will warn on a new label type being added due to the lack of `default`
  // The following is required by the compiler, otherwise it things control can reach the end of this function without
  // returning.
  assert(false);
  return "unknown example_type enum";

#undef CASE
}
