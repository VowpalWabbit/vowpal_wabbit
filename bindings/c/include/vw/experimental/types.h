// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

// uint32_t, uint64_t, etc
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

  // TODO: these status codes should match the codes used in the rest of the codebase.
  typedef uint32_t VWStatus;
  static const VWStatus VW_SUCCESS = 0;
  static const VWStatus VW_FAIL = 1;
  static const VWStatus VW_NOT_IMPLEMENTED = 2;

#ifdef __cplusplus
}
#endif
