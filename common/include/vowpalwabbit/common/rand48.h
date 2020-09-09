// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once
// A quick implementation similar to drand48 for cross-platform compatibility
#include <cmath>
#include <cstdint>

#include "vowpalwabbit/common/future_compat.h"

//
// NB: the 'ULL' suffix is not part of the constant it is there to
// prevent truncation of constant to (32-bit long) when compiling
// in a 32-bit env: warning: integer constant is too large for "long" type
//
#ifdef __clang__
#  pragma clang diagnostic ignored "-Wc++11-long-long"
#endif
constexpr uint64_t CONSTANT_A = 0xeece66d5deece66dULL;
constexpr uint64_t CONSTANT_C = 2147483647;

constexpr int BIAS = 127 << 23;

namespace VW
{
namespace common
{
inline float merand48(uint64_t& initial)
{
  static_assert(sizeof(int32_t) == sizeof(float), "Floats and ints are converted between, they must be the same size.");
  initial = CONSTANT_A * initial + CONSTANT_C;
  int32_t temp = ((initial >> 25) & 0x7FFFFF) | BIAS;
  return reinterpret_cast<float&>(temp) - 1;
}

inline float merand48_noadvance(uint64_t v) { return merand48(v); }

inline float merand48_boxmuller(uint64_t& index)
{
  float x1 = 0.0;
  float x2 = 0.0;
  float temp = 0.0;
  do
  {
    x1 = 2.0f * merand48(index) - 1.0f;
    x2 = 2.0f * merand48(index) - 1.0f;
    temp = x1 * x1 + x2 * x2;
  } while ((temp >= 1.0) || (temp == 0.0));
  temp = std::sqrt((-2.0f * logf(temp)) / temp);
  return x1 * temp;
}

}  // namespace common
}  // namespace VW

// VW_DEPRECATED("Function has been moved into a namespace, use: VW::common::merand48(...) ")
inline float merand48(uint64_t& initial) { return VW::common::merand48(initial); }

// VW_DEPRECATED("Function has been moved into a namespace, use: VW::common::merand48_noadvance(...) ")
inline float merand48_noadvance(uint64_t v) { return VW::common::merand48_noadvance(v); }

// VW_DEPRECATED("Function has been moved into a namespace, use: VW::common::merand48_boxmuller(...) ")
inline float merand48_boxmuller(uint64_t& index) { return VW::common::merand48_boxmuller(index); }