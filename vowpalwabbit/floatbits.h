/*
  Copyright (c) by respective owners including Yahoo!, Microsoft, and
  individual contributors. All rights reserved.  Released under a BSD
  license as described in the file LICENSE.
*/
#pragma once
#include <stdint.h>

static inline uint32_t float_to_bits(float a)
{
  union {
    float f;
    uint32_t i;
  } x;
  x.f = a;
  return x.i;
}

static inline float bits_to_float(uint32_t a)
{
  union {
    float f;
    uint32_t i;
  } x;
  x.i = a;
  return x.f;
}
