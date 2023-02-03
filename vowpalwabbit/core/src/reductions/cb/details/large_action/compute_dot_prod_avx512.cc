// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#ifdef BUILD_LAS_WITH_SIMD

#  include "compute_dot_prod_simd.h"
#  include "kernel_impl.h"

#  include <x86intrin.h>

namespace VW
{
namespace cb_explore_adf
{
inline void compute1(float feature_value, uint64_t feature_index, uint64_t offset, uint64_t weights_mask,
    uint64_t column_index, uint64_t seed, float& sum)
{
  uint64_t index = feature_index + offset;
  kernel_impl(feature_value, index, weights_mask, column_index, seed, sum);
}

// Process 16 features in parallel using AVX-512, resulting in the same output of 16 compute1() executions.
inline void compute16(const __m512& feature_values, const __m512i& feature_indices1, const __m512i& feature_indices2,
    const __m512i& offsets, const __m512i& weights_masks, const __m512i& column_indices, const __m512i& seeds,
    __m512& sums)
{
  // value_maps must be the same as the scalar VALUE_MAP.
  const __m512 value_maps = _mm512_setr4_ps(0, 0, 1.f, -1.f);
  const __m512i perm_idx = _mm512_setr_epi32(0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30);
  const __m512i all_ones = _mm512_set1_epi32(1);

  __m512i indices1 = _mm512_add_epi64(feature_indices1, offsets);
  __m512i indices2 = _mm512_add_epi64(feature_indices2, offsets);

  indices1 = _mm512_add_epi64(_mm512_and_epi64(indices1, weights_masks), column_indices);
  __m512i popcounts1 = _mm512_popcnt_epi64(indices1);
  indices2 = _mm512_add_epi64(_mm512_and_epi64(indices2, weights_masks), column_indices);
  __m512i popcounts2 = _mm512_popcnt_epi64(indices2);

  // popcounts always fit into 32 bits, so truncate and pack all 16 popcounts.
  __m512i popcounts = _mm512_permutex2var_epi32(popcounts1, perm_idx, popcounts2);
  __m512i sparsity_indices = _mm512_slli_epi32(_mm512_and_epi32(popcounts, all_ones), 1);

  indices1 = _mm512_add_epi64(indices1, seeds);
  popcounts1 = _mm512_popcnt_epi64(indices1);
  indices2 = _mm512_add_epi64(indices2, seeds);
  popcounts2 = _mm512_popcnt_epi64(indices2);

  popcounts = _mm512_permutex2var_epi32(popcounts1, perm_idx, popcounts2);
  __m512i value_indices = _mm512_add_epi32(sparsity_indices, _mm512_and_epi32(popcounts, all_ones));

  __m512 tmp = _mm512_permutexvar_ps(value_indices, value_maps);
  sums = _mm512_fmadd_ps(feature_values, tmp, sums);
}

// A data parallel implementation of the foreach_feature that processes 16 features at once.
float compute_dot_prod_avx512(uint64_t column_index, VW::workspace* _all, uint64_t seed, VW::example* ex)
{
  float sum = 0.f;
  const uint64_t offset = ex->ft_offset;
  const uint64_t weights_mask = _all->weights.mask();

  __m512 sums = _mm512_setzero_ps();
  const __m512i column_indices = _mm512_set1_epi64(column_index);
  const __m512i seeds = _mm512_set1_epi64(seed);
  const __m512i weights_masks = _mm512_set1_epi64(weights_mask);
  const __m512i offsets = _mm512_set1_epi64(offset);

  const bool ignore_some_linear = _all->ignore_some_linear;
  const auto& ignore_linear = _all->ignore_linear;
  for (auto i = ex->begin(); i != ex->end(); ++i)
  {
    if (ignore_some_linear && ignore_linear[i.index()]) { continue; }
    const auto& features = *i;
    const size_t num_features = features.size();
    size_t j = 0;
    for (; j + 16 <= num_features; j += 16)
    {
      // Unroll the 64-bit indices twice to align with 32-bit values.
      __m512i indices1 = _mm512_loadu_si512(&features.indices[j]);
      __m512i indices2 = _mm512_loadu_si512(&features.indices[j + 8]);
      // If indices fit into 32 bits, convert indices to 32-bit here can speed up further.

      __m512 values = _mm512_loadu_ps(&features.values[j]);
      compute16(values, indices1, indices2, offsets, weights_masks, column_indices, seeds, sums);
    }
    for (; j < num_features; ++j)
    {
      // Handle tail of the loop using scalar implementation.
      compute1(features.values[j], features.indices[j], offset, weights_mask, column_index, seed, sum);
    }
  }

  const auto& red_features = ex->ex_reduction_features.template get<VW::large_action_space::las_reduction_features>();
  const auto& interactions =
      red_features.generated_interactions ? *red_features.generated_interactions : *ex->interactions;
  const auto& extent_interactions = red_features.generated_extent_interactions
      ? *red_features.generated_extent_interactions
      : *ex->extent_interactions;
  if (!extent_interactions.empty())
  {
    // TODO: Add support for extent_interactions.
    // This code should not be reachable, since we checked conflicting command line options.
    _all->logger.err_error("Extent_interactions are not supported yet in large action space with SIMD implementations");
  }

  for (const auto& ns : interactions)
  {
    if (ns.size() != 2)
    {
      // TODO: Add support for interactions other than quadratics.
      // This code should not be reachable, since we checked conflicting command line options.
      _all->logger.err_error(
          "Generic interactions are not supported yet in large action space with SIMD implementations");
    }

    const bool same_namespace = (!_all->permutations && (ns[0] == ns[1]));
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
        __m512i indices1 = _mm512_loadu_si512(&ns1_indices[j]);
        __m512i indices2 = _mm512_loadu_si512(&ns1_indices[j + 8]);
        indices1 = _mm512_xor_epi64(indices1, halfhashes);
        indices2 = _mm512_xor_epi64(indices2, halfhashes);

        __m512 values = _mm512_loadu_ps(&ns1_values[j]);
        values = _mm512_mul_ps(vals, values);

        compute16(values, indices1, indices2, offsets, weights_masks, column_indices, seeds, sums);
      }
      for (; j < num_features_ns1; ++j)
      {
        float feature_value = val * ns1_values[j];
        auto index = (ns1_indices[j] ^ halfhash);
        compute1(feature_value, index, offset, weights_mask, column_index, seed, sum);
      }
    }
  }

  return sum + _mm512_reduce_add_ps(sums);
}

}  // namespace cb_explore_adf
}  // namespace VW

#endif
