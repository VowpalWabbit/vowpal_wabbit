// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

// for: namespace_index
#include <ostream>

#include "feature_group.h"

namespace INTERACTIONS
{

struct extent_interaction
{
  namespace_index _ns_char = 0;
  uint64_t _ns_hash = 0;
}


bool contains_wildcard(const std::vector<namespace_index>& interaction);
}  // namespace INTERACTIONS
