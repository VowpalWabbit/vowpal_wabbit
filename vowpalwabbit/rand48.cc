// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

// A quick implementation similar to drand48 for cross-platform compatibility
#include <cstdint>
//
// NB: the 'ULL' suffix is not part of the constant it is there to
// prevent truncation of constant to (32-bit long) when compiling
// in a 32-bit env: warning: integer constant is too large for "long" type
//
#ifdef __clang__
#pragma clang diagnostic ignored "-Wc++11-long-long"
#endif
constexpr uint64_t a = 0xeece66d5deece66dULL;
constexpr uint64_t c = 2147483647;

constexpr int bias = 127 << 23;

float merand48(uint64_t& initial)
{
  static_assert(sizeof(int) == sizeof(float), "Floats and ints are converted between, they must be the same size.");
  initial = a * initial + c;
  int32_t temp = ((initial >> 25) & 0x7FFFFF) | bias;
  return reinterpret_cast<float&>(temp) - 1;
}

float merand48_noadvance(uint64_t v) { return merand48(v); }
