// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "../large_action_space.h"
#include "kernel_impl.h"
#include "vw/core/reductions/gd.h"

namespace VW
{
namespace cb_explore_adf
{
class AO_triplet_constructor
{
public:
  AO_triplet_constructor(uint64_t weights_mask, uint64_t column_index, uint64_t seed, float& final_dot_product)
      : _weights_mask(weights_mask), _column_index(column_index), _seed(seed), _final_dot_product(final_dot_product)
  {
  }

  void set(float feature_value, uint64_t index)
  {
    /**
     * set is going to be called foreach feature
     *
     * The index and _column_index are used figure out the Omega cell and its value that is needed to multiply the
     * feature value with.
     * That combined index is then used to flip a coin and generate rademacher randomness.
     * The way the coin is flipped is by counting the number of bits in the combined index (plus the seed).
     * If the number of bits are even then we multiply with -1.f else we multiply with 1.f
     *
     * This is a sparse rademacher implementation where half the time we select zero and the remaining time we flip the
     * coin to get -1/+1. The below code is the equivalent of branching on the first bit count of the combined index
     * without the seed. If even then we would return 0 if odd then we would branch again on the combined index
     * including the seed and select -1/+1.
     *
     * Branching is avoided by using the lookup maps.
     */
    kernel_impl(feature_value, index, _weights_mask, _column_index, _seed, _final_dot_product);
  }

private:
  uint64_t _weights_mask;
  uint64_t _column_index;
  uint64_t _seed;
  float& _final_dot_product;
};

inline float compute_dot_prod_scalar(uint64_t col, VW::workspace* _all, uint64_t _seed, VW::example* ex)
{
  const auto& red_features = ex->ex_reduction_features.template get<VW::large_action_space::las_reduction_features>();
  float final_dot_prod = 0.f;

  AO_triplet_constructor tc(_all->weights.mask(), col, _seed, final_dot_prod);

  VW::foreach_feature<AO_triplet_constructor, uint64_t, triplet_construction, dense_parameters>(
      _all->weights.dense_weights, _all->ignore_some_linear, _all->ignore_linear,
      (red_features.generated_interactions ? *red_features.generated_interactions : *ex->interactions),
      (red_features.generated_extent_interactions ? *red_features.generated_extent_interactions
                                                  : *ex->extent_interactions),
      _all->permutations, *ex, tc, _all->generate_interactions_object_cache_state);

  return final_dot_prod;
}

}  // namespace cb_explore_adf
}  // namespace VW
