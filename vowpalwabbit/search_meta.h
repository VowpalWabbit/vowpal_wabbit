// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once
#include "search.h"

#define DECLARE_METATASK(X)                \
  namespace X                              \
  {                                        \
  extern Search::search_metatask metatask; \
  }

DECLARE_METATASK(DebugMT)
DECLARE_METATASK(SelectiveBranchingMT)

// namespace DebugMT              { extern Search::search_metatask metatask; }
// namespace SelectiveBranchingMT { extern Search::search_metatask metatask; }
