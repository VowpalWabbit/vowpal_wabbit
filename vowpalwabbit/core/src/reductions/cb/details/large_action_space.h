// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "eigen/Eigen/Dense"
#include "vw/core/rand48.h"

#include "vw/core/action_score.h"
#include "vw/core/v_array.h"
#include "vw/core/vw_fwd.h"
#include <iostream>
#include <vector>

namespace VW
{
namespace cb_explore_adf
{
struct cb_explore_adf_large_action_space
{
  uint64_t d = 0;
  float gamma = 1;
  Eigen::MatrixXf Q;

  VW::workspace* all;
  uint64_t seed = 0;
  VW::v_array<float> shrink_factors;

public:
  cb_explore_adf_large_action_space(uint64_t d_, float gamma_, VW::workspace* all_);
  ~cb_explore_adf_large_action_space() = default;

  // Should be called through cb_explore_adf_base for pre/post-processing
  void predict(VW::LEARNER::multi_learner& base, multi_ex& examples);
  void learn(VW::LEARNER::multi_learner& base, multi_ex& examples);

  void calculate_shrink_factor(const ACTION_SCORE::action_scores& preds, float min_ck);
  void generate_Q(const multi_ex& examples);

private:
  template <bool is_learn>
  void predict_or_learn_impl(VW::LEARNER::multi_learner& base, multi_ex& examples);
};

// this is meant to be called with foreach_feature
// for each feature it will multiply the weight that corresponds to the index with the corresponding gaussian element
// If the returned values are summed the end resutlt is the dot product between the weight vector of the features of an
// example with a vector of gaussian elements
template <typename WeightsT>
struct LazyGaussianDotProduct
{
private:
  WeightsT& weights;
  uint64_t column_index;
  uint64_t seed;

public:
  LazyGaussianDotProduct(WeightsT& weights_, uint64_t column_index_, uint64_t seed_)
      : weights(weights_), column_index(column_index_), seed(seed_)
  {
  }
  inline float operator[](uint64_t index) const
  {
    auto combined_index = index + column_index + seed;
    return weights[index] * merand48_boxmuller(combined_index);
  }
};
}  // namespace cb_explore_adf
}  // namespace VW