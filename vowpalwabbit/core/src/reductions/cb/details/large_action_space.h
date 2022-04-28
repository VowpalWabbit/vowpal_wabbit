// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "vw/core/action_score.h"
#include "vw/core/rand48.h"
#include "vw/core/v_array.h"
#include "vw/core/vw_fwd.h"

#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <iostream>
#include <vector>

namespace VW
{
namespace cb_explore_adf
{
struct cb_explore_adf_large_action_space
{
private:
  uint64_t _d = 0;
  float _gamma = 1;
  VW::workspace* _all;
  uint64_t _seed = 0;
  std::vector<Eigen::Triplet<float>> _triplets;

public:
  Eigen::MatrixXf Q;
  Eigen::SparseMatrix<float> A;
  VW::v_array<float> shrink_factors;

  cb_explore_adf_large_action_space(uint64_t d, float gamma, VW::workspace* all);
  ~cb_explore_adf_large_action_space() = default;

  // Should be called through cb_explore_adf_base for pre/post-processing
  void predict(VW::LEARNER::multi_learner& base, multi_ex& examples);
  void learn(VW::LEARNER::multi_learner& base, multi_ex& examples);

  void calculate_shrink_factor(const ACTION_SCORE::action_scores& preds, float min_ck);
  void generate_Q(const multi_ex& examples);
  void generate_A(const multi_ex& examples);
  void QR_decomposition();
  // void generate_Z(const multi_ex& examples);

private:
  template <bool is_learn>
  void predict_or_learn_impl(VW::LEARNER::multi_learner& base, multi_ex& examples);
};
}  // namespace cb_explore_adf
}  // namespace VW