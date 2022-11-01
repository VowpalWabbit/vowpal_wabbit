// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "../large_action_space.h"
#include "vw/core/example.h"
#include "vw/core/global_data.h"
#include "vw/core/large_action_space_reduction_features.h"

#include <x86intrin.h>

namespace VW
{
namespace cb_explore_adf
{

// TODO: refactor
constexpr std::array<size_t, 2> INDEX_MAP = {0, 2};
constexpr std::array<float, 4> VALUE_MAP = {0.f, 0.f, 1.f, -1.f};
const __m512 value_maps = _mm512_setr4_ps(0, 0, 1.f, -1.f);
const __m512i perm_idx = _mm512_setr_epi32(0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30);

/*
// Alternative implementation of _mm512_popcnt_epi64
// https://github.com/WojciechMula/sse-popcount/blob/master/popcnt-avx512-harley-seal.cpp
inline __m512i popcount64(const __m512i v)
{
  const __m512i m1 = _mm512_set1_epi8(0x55);
  const __m512i m2 = _mm512_set1_epi8(0x33);
  const __m512i m4 = _mm512_set1_epi8(0x0F);

  const __m512i t1 = _mm512_sub_epi8(v, (_mm512_srli_epi16(v, 1) & m1));
  const __m512i t2 = _mm512_add_epi8(t1 & m2, (_mm512_srli_epi16(t1, 2) & m2));
  const __m512i t3 = _mm512_add_epi8(t2, _mm512_srli_epi16(t2, 4)) & m4;
  return _mm512_sad_epu8(t3, _mm512_setzero_si512());
}
*/

inline float compute_dot_prod_simd(uint64_t column_index_, VW::workspace* _all, uint64_t seed_, VW::example* ex,
    const VW::large_action_space::las_reduction_features& red_features)
{
  float sum = 0.f;
  const uint64_t offset_ = ex->ft_offset;
  const uint64_t weights_mask_ = _all->weights.mask();

  __m512 sum_vec = _mm512_setzero_ps();
  const __m512i column_index = _mm512_set1_epi64(column_index_);
  const __m512i seed = _mm512_set1_epi64(seed_);
  const __m512i weights_mask = _mm512_set1_epi64(weights_mask_);
  const __m512i offset = _mm512_set1_epi64(offset_);
  const __m512i all_ones = _mm512_set1_epi32(1);

  for (const auto& features : *ex)
  {
    const size_t num_features = features.size();
    size_t i = 0;
    for (; i + 16 <= num_features; i += 16)
    {
      // Unroll the 64-bit indices twice to align with 32-bit values.
      __m512i indices1 = _mm512_loadu_epi64(&features.indices[i]);
      __m512i indices2 = _mm512_loadu_epi64(&features.indices[i + 8]);
      // TODO: Convert indices to 32-bit here to further speed up.

      indices1 = _mm512_and_epi64(_mm512_add_epi64(indices1, offset), weights_mask);
      indices1 = _mm512_add_epi64(indices1, column_index);
      __m512i popcounts1 = _mm512_popcnt_epi64(indices1);
      indices2 = _mm512_and_epi64(_mm512_add_epi64(indices2, offset), weights_mask);
      indices2 = _mm512_add_epi64(indices2, column_index);
      __m512i popcounts2 = _mm512_popcnt_epi64(indices2);

      __m512i popcounts = _mm512_permutex2var_epi32(popcounts1, perm_idx, popcounts2);
      __m512i sparsity_indices = _mm512_slli_epi32(_mm512_and_epi32(popcounts, all_ones), 1);

      indices1 = _mm512_add_epi64(indices1, seed);
      popcounts1 = _mm512_popcnt_epi64(indices1);
      indices2 = _mm512_add_epi64(indices2, seed);
      popcounts2 = _mm512_popcnt_epi64(indices2);

      popcounts = _mm512_permutex2var_epi32(popcounts1, perm_idx, popcounts2);
      __m512i value_indices = _mm512_add_epi32(sparsity_indices, _mm512_and_epi32(popcounts, all_ones));
      __m512 tmp = _mm512_permutexvar_ps(value_indices, value_maps);

      __m512 values = _mm512_loadu_ps(&features.values[i]);
      sum_vec = _mm512_fmadd_ps(values, tmp, sum_vec);
    }
    for (; i < num_features; ++i)
    {
      // TODO: refactor
      auto index = features.indices[i] + offset_;
      size_t select_sparsity = __builtin_parity((index & weights_mask_) + column_index_);
      auto sparsity_index = INDEX_MAP[select_sparsity];
      size_t select_sign = __builtin_parity((index & weights_mask_) + column_index_ + seed_);
      auto value_index = sparsity_index + select_sign;
      float tmp = VALUE_MAP[value_index];
      sum += features.values[i] * tmp;
      // sum += feature_values[i][j] * select_sparsity * (1.f - (select_sign << 1));
    }
  }
  // TODO: other interactions, this only handles quadratics
  // TODO: permutations, return num_features
  for (const auto& ns : *red_features.generated_interactions)
  {
    assert(ns.size() == 2);
    const bool same_namespace = (ns[0] == ns[1]);
    const size_t num_features_ns0 = ex->feature_space[ns[0]].size();
    const size_t num_features_ns1 = ex->feature_space[ns[1]].size();
    const auto& ns0_indices = ex->feature_space[ns[0]].indices;
    const auto& ns1_indices = ex->feature_space[ns[1]].indices;
    const auto& ns0_values = ex->feature_space[ns[0]].values;
    const auto& ns1_values = ex->feature_space[ns[1]].values;
    for (size_t i = 0; i < num_features_ns0; ++i)
    {
      const uint64_t halfhash = VW::details::FNV_PRIME * ns0_indices[i];
      const __m512i halfhashes = _mm512_set1_epi64(halfhash);
      const float val = ns0_values[i];
      const __m512 vals = _mm512_set1_ps(val);
      size_t j = same_namespace ? i : 0;
      for (; j + 16 <= num_features_ns1; j += 16)
      {
        // Unroll the 64-bit indices twice to align with 32-bit values.
        __m512i indices1 = _mm512_loadu_epi64(&ns1_indices[j]);
        __m512i indices2 = _mm512_loadu_epi64(&ns1_indices[j + 8]);

        indices1 = _mm512_xor_epi64(indices1, halfhashes);
        indices1 = _mm512_and_epi64(_mm512_add_epi64(indices1, offset), weights_mask);
        indices1 = _mm512_add_epi64(indices1, column_index);
        __m512i popcounts1 = _mm512_popcnt_epi64(indices1);
        indices2 = _mm512_xor_epi64(indices2, halfhashes);
        indices2 = _mm512_and_epi64(_mm512_add_epi64(indices2, offset), weights_mask);
        indices2 = _mm512_add_epi64(indices2, column_index);
        __m512i popcounts2 = _mm512_popcnt_epi64(indices2);

        __m512i popcounts = _mm512_permutex2var_epi32(popcounts1, perm_idx, popcounts2);
        __m512i sparsity_indices = _mm512_slli_epi32(_mm512_and_epi32(popcounts, all_ones), 1);

        indices1 = _mm512_add_epi64(indices1, seed);
        popcounts1 = _mm512_popcnt_epi64(indices1);
        indices2 = _mm512_add_epi64(indices2, seed);
        popcounts2 = _mm512_popcnt_epi64(indices2);

        popcounts = _mm512_permutex2var_epi32(popcounts1, perm_idx, popcounts2);
        __m512i value_indices = _mm512_add_epi32(sparsity_indices, _mm512_and_epi32(popcounts, all_ones));
        __m512 tmp = _mm512_permutexvar_ps(value_indices, value_maps);

        __m512 values = _mm512_loadu_ps(&ns1_values[j]);
        values = _mm512_mul_ps(vals, values);
        sum_vec = _mm512_fmadd_ps(values, tmp, sum_vec);
      }
      for (; j < num_features_ns1; ++j)
      {
        float feature_value = val * ns1_values[j];
        auto index = (ns1_indices[j] ^ halfhash) + offset_;
        size_t select_sparsity = __builtin_parity((index & weights_mask_) + column_index_);
        auto sparsity_index = INDEX_MAP[select_sparsity];
        size_t select_sign = __builtin_parity((index & weights_mask_) + column_index_ + seed_);
        auto value_index = sparsity_index + select_sign;
        float tmp = VALUE_MAP[value_index];
        sum += feature_value * tmp;
      }
    }
  }
  return sum + _mm512_reduce_add_ps(sum_vec);
}

}  // namespace cb_explore_adf
}  // namespace VW