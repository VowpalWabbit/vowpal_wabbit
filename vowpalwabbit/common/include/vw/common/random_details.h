// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include <cmath>
#include <cstdint>

namespace VW
{
namespace details
{

union int_float_convert
{
  int32_t int_value;
  float float_value;
};

//
// NB: the 'ULL' suffix is not part of the constant it is there to
// prevent truncation of constant to (32-bit long) when compiling
// in a 32-bit env: warning: integer constant is too large for "long" type
//
constexpr uint64_t CONSTANT_A = 0xeece66d5deece66dULL;
constexpr uint64_t CONSTANT_C = 2147483647;

constexpr int BIAS = 127 << 23;

inline float merand48(uint64_t& initial)
{
  initial = CONSTANT_A * initial + CONSTANT_C;
  int_float_convert temp{};
  temp.int_value = ((initial >> 25) & 0x7FFFFF) | BIAS;
  return temp.float_value - 1;
}

inline float merand48_noadvance(uint64_t v) { return merand48(v); }

inline float merand48_boxmuller(uint64_t& index)
{
  float x1 = 0.0;
  float x2 = 0.0;
  float temp = 0.0;
  do {
    x1 = 2.0f * merand48(index) - 1.0f;
    x2 = 2.0f * merand48(index) - 1.0f;
    temp = x1 * x1 + x2 * x2;
  } while ((temp >= 1.0) || (temp == 0.0));
  temp = std::sqrt((-2.0f * logf(temp)) / temp);
  return x1 * temp;
}
}  // namespace details
}  // namespace VW
