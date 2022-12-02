// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <array>
#include <cstdint>
#ifdef _MSC_VER
#  include <intrin.h>
#endif

namespace VW
{
namespace cb_explore_adf
{
inline void kernel_impl(float feature_value, uint64_t index, uint64_t weights_mask, uint64_t column_index,
    uint64_t seed, float& final_dot_product)
{
  // index mapped used to select sparsity or not from value map
  constexpr static std::array<int, 2> INDEX_MAP = {0, 2};
  constexpr static std::array<float, 4> VALUE_MAP = {0.f, 0.f, 1.f, -1.f};

#ifdef _MSC_VER
  int select_sparsity = __popcnt((index & weights_mask) + column_index) & 1;
  int sparsity_index = INDEX_MAP[select_sparsity];
  int select_sign = __popcnt((index & weights_mask) + column_index + seed) & 1;
  int value_index = sparsity_index + select_sign;
  float val = VALUE_MAP[value_index];
#else
  int select_sparsity = __builtin_parity((index & weights_mask) + column_index);
  int sparsity_index = INDEX_MAP[select_sparsity];
  int select_sign = __builtin_parity((index & weights_mask) + column_index + seed);
  int value_index = sparsity_index + select_sign;
  float val = VALUE_MAP[value_index];
#endif

  final_dot_product += feature_value * val;
}

}  // namespace cb_explore_adf
}  // namespace VW
