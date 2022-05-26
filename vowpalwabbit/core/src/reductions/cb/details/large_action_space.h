// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "vw/core/action_score.h"
#include "vw/core/rand48.h"
#include "vw/core/v_array.h"
#include "vw/core/vw_fwd.h"

#include <Eigen/Dense>
#include <Eigen/SparseCore>
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
  bool _apply_shrink_factor = false;
  VW::workspace* _all;
  uint64_t _seed = 0;
  std::vector<Eigen::Triplet<float>> _triplets;

public:
  Eigen::SparseMatrix<float> Y;
  Eigen::MatrixXf B;
  Eigen::MatrixXf Z;
  Eigen::MatrixXf U;
  std::vector<float> shrink_factors;
  // the below matrixes are used only during unit testing and are not set otherwise
  Eigen::VectorXf _S;
  Eigen::MatrixXf _V;
  Eigen::SparseMatrix<float> _A;
  bool _set_all_svd_components = false;

  cb_explore_adf_large_action_space(uint64_t d, float gamma, bool apply_shrink_factor, VW::workspace* all);
  ~cb_explore_adf_large_action_space() = default;

  // Should be called through cb_explore_adf_base for pre/post-processing
  void predict(VW::LEARNER::multi_learner& base, multi_ex& examples);
  void learn(VW::LEARNER::multi_learner& base, multi_ex& examples);

  void calculate_shrink_factor(const ACTION_SCORE::action_scores& preds);
  void generate_Z(const multi_ex& examples);
  void generate_B(const multi_ex& examples);
  bool generate_Y(const multi_ex& examples);
  void randomized_SVD(const multi_ex& examples);
  void set_rank(uint64_t rank);
  // the below methods are used only during unit testing and are not called otherwise
  bool _generate_A(const multi_ex& examples);
  void _populate_all_SVD_components();

private:
  template <bool is_learn>
  void predict_or_learn_impl(VW::LEARNER::multi_learner& base, multi_ex& examples);
};
}  // namespace cb_explore_adf
}  // namespace VW