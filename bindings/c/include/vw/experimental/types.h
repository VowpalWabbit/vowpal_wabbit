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

  typedef uint32_t VWStatus;
  static const VWStatus VW_success = 0;

// Generate all error codes based on the definitions provided in "error_data.h"
// This macro gets expanded for each individual error definition
#define ERROR_CODE_DEFINITION(code, name, message) static const VWStatus VW_##name = code;

#include "error_data.h"

#ifdef __cplusplus
}
#endif
