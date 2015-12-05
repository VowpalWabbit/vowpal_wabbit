/*
  Copyright (c) by respective owners including Yahoo!, Microsoft, and
  individual contributors. All rights reserved.  Released under a BSD
  license as described in the file LICENSE.
*/
#pragma once
#include <stdint.h>
#include <string.h>

static inline uint32_t float_to_bits(float a)
{
  uint32_t ret;
  memcpy(&ret, &a, sizeof(uint32_t));
  return ret;
}

static inline float bits_to_float(uint32_t a)
{
  float ret;
  memcpy(&ret, &a, sizeof(float));
  return ret;
}
