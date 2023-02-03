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
namespace
{
// Alternative AVX2 implementations of necessary AVX512 intrinsics.

// https://arxiv.org/pdf/1611.07612.pdf
inline __m256i popcount64(const __m256i& v)
{
  const __m256i lookup =
      _mm256_setr_epi8(0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4);
  const __m256i low_mask = _mm256_set1_epi8(0x0f);
  const __m256i lo = _mm256_and_si256(v, low_mask);
  const __m256i hi = _mm256_and_si256(_mm256_srli_epi32(v, 4), low_mask);
  const __m256i popcnt1 = _mm256_shuffle_epi8(lookup, lo);
  const __m256i popcnt2 = _mm256_shuffle_epi8(lookup, hi);
  const __m256i total = _mm256_add_epi8(popcnt1, popcnt2);
  return _mm256_sad_epu8(total, _mm256_setzero_si256());
}

// https://stackoverflow.com/questions/69408063/how-to-convert-int-64-to-int-32-with-avx-but-without-avx-512
inline __m256i pack64to32(const __m256i& a, const __m256i& b)
{
  __m256 combined = _mm256_shuffle_ps(_mm256_castsi256_ps(a), _mm256_castsi256_ps(b), _MM_SHUFFLE(2, 0, 2, 0));
  __m256d ordered = _mm256_permute4x64_pd(_mm256_castps_pd(combined), _MM_SHUFFLE(3, 1, 2, 0));
  return _mm256_castpd_si256(ordered);
}

// https://stackoverflow.com/questions/23189488/horizontal-sum-of-32-bit-floats-in-256-bit-avx-vector
inline float horizontal_sum(const __m256& x)
{
  const __m128 x128 = _mm_add_ps(_mm256_extractf128_ps(x, 1), _mm256_castps256_ps128(x));
  const __m128 x64 = _mm_add_ps(x128, _mm_movehl_ps(x128, x128));
  const __m128 x32 = _mm_add_ss(x64, _mm_shuffle_ps(x64, x64, 0x55));
  return _mm_cvtss_f32(x32);
}

}  // namespace

inline void compute1(float feature_value, uint64_t feature_index, uint64_t offset, uint64_t weights_mask,
    uint64_t column_index, uint64_t seed, float& sum)
{
  uint64_t index = feature_index + offset;
  kernel_impl(feature_value, index, weights_mask, column_index, seed, sum);
}

// Process 8 features in parallel using AVX2, resulting in the same output of 8 compute1() executions.
inline void compute8(const __m256& feature_values, const __m256i& feature_indices1, const __m256i& feature_indices2,
    const __m256i& offsets, const __m256i& weights_masks, const __m256i& column_indices, const __m256i& seeds,
    __m256& sums)
{
  // value_maps must be the same as the scalar VALUE_MAP.
  const __m256 value_maps = _mm256_setr_ps(0, 0, 1.f, -1.f, 0, 0, 1.f, -1.f);
  const __m256i all_ones = _mm256_set1_epi32(1);

  __m256i indices1 = _mm256_add_epi64(feature_indices1, offsets);
  __m256i indices2 = _mm256_add_epi64(feature_indices2, offsets);

  indices1 = _mm256_add_epi64(_mm256_and_si256(indices1, weights_masks), column_indices);
  __m256i popcounts1 = popcount64(indices1);
  indices2 = _mm256_add_epi64(_mm256_and_si256(indices2, weights_masks), column_indices);
  __m256i popcounts2 = popcount64(indices2);

  // popcounts always fit into 32 bits, so truncate and pack all 8 popcounts.
  __m256i popcounts = pack64to32(popcounts1, popcounts2);
  __m256i sparsity_indices = _mm256_slli_epi32(_mm256_and_si256(popcounts, all_ones), 1);

  indices1 = _mm256_add_epi64(indices1, seeds);
  popcounts1 = popcount64(indices1);
  indices2 = _mm256_add_epi64(indices2, seeds);
  popcounts2 = popcount64(indices2);

  popcounts = pack64to32(popcounts1, popcounts2);
  __m256i value_indices = _mm256_add_epi32(sparsity_indices, _mm256_and_si256(popcounts, all_ones));

  __m256 tmp = _mm256_permutevar8x32_ps(value_maps, value_indices);
  sums = _mm256_fmadd_ps(feature_values, tmp, sums);
}

// A data parallel implementation of the foreach_feature that processes 8 features at once.
float compute_dot_prod_avx2(uint64_t column_index, VW::workspace* _all, uint64_t seed, VW::example* ex)
{
  float sum = 0.f;
  const uint64_t offset = ex->ft_offset;
  const uint64_t weights_mask = _all->weights.mask();

  __m256 sums = _mm256_setzero_ps();
  const __m256i column_indices = _mm256_set1_epi64x(column_index);
  const __m256i seeds = _mm256_set1_epi64x(seed);
  const __m256i weights_masks = _mm256_set1_epi64x(weights_mask);
  const __m256i offsets = _mm256_set1_epi64x(offset);

  const bool ignore_some_linear = _all->ignore_some_linear;
  const auto& ignore_linear = _all->ignore_linear;
  for (auto i = ex->begin(); i != ex->end(); ++i)
  {
    if (ignore_some_linear && ignore_linear[i.index()]) { continue; }
    const auto& features = *i;
    const size_t num_features = features.size();
    size_t j = 0;
    for (; j + 8 <= num_features; j += 8)
    {
      // Unroll the 64-bit indices twice to align with 32-bit values.
      __m256i indices1 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&features.indices[j]));
      __m256i indices2 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&features.indices[j + 4]));
      // If indices fit into 32 bits, convert indices to 32-bit here can speed up further.

      __m256 values = _mm256_loadu_ps(&features.values[j]);
      compute8(values, indices1, indices2, offsets, weights_masks, column_indices, seeds, sums);
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
      const __m256i halfhashes = _mm256_set1_epi64x(halfhash);
      const float val = ns0_values[i];
      const __m256 vals = _mm256_set1_ps(val);
      size_t j = same_namespace ? i : 0;

      for (; j + 8 <= num_features_ns1; j += 8)
      {
        __m256i indices1 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&ns1_indices[j]));
        __m256i indices2 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&ns1_indices[j + 4]));
        indices1 = _mm256_xor_si256(indices1, halfhashes);
        indices2 = _mm256_xor_si256(indices2, halfhashes);

        __m256 values = _mm256_loadu_ps(&ns1_values[j]);
        values = _mm256_mul_ps(vals, values);

        compute8(values, indices1, indices2, offsets, weights_masks, column_indices, seeds, sums);
      }
      for (; j < num_features_ns1; ++j)
      {
        float feature_value = val * ns1_values[j];
        auto index = (ns1_indices[j] ^ halfhash);
        compute1(feature_value, index, offset, weights_mask, column_index, seed, sum);
      }
    }
  }

  return sum + horizontal_sum(sums);
}

}  // namespace cb_explore_adf
}  // namespace VW

#endif
