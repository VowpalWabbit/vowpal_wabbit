// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
#include <cstdint>  // defines size_t
#include "future_compat.h"
#include "hash.h"

VW_STD14_CONSTEXPR inline uint64_t hashall(const char * s, size_t len, uint64_t h)
{
  return uniform_hash(s, len, h);
}

VW_STD14_CONSTEXPR inline uint64_t hashstring(const char* s, size_t len, uint64_t h)
{
  const char* front = s;
  while (len > 0 && front[0] <= 0x20 && (int)(front[0]) >= 0)
  {
    ++front;
    --len;
  }
  while (len > 0 && front[len - 1] <= 0x20 && (int)(front[len - 1]) >= 0)
  {
    --len;
  }

  size_t ret = 0;
  const char* p = front;
  while (p != front + len)
    if (*p >= '0' && *p <= '9')
      ret = 10 * ret + *(p++) - '0';
    else
      return uniform_hash(front, len, h);

  return ret + h;
}
