// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "vw/core/action_score.h"
#include "vw/core/array_parameters_dense.h"
#include "vw/core/rand48.h"
#include "vw/core/v_array.h"
#include "vw/core/vw_fwd.h"

#include <Eigen/Dense>
#include <Eigen/SparseCore>
#include <iostream>
#include <set>
#include <unordered_map>
#include <vector>

namespace VW
{
namespace cb_explore_adf
{
enum class implementation_type
{
  vanilla_rand_svd,
  model_weight_rand_svd,
  aatop
};

struct vanilla_rand_svd_impl
{
 private:
  VW::workspace* _all;
  uint64_t _d;
  std::vector<Eigen::Triplet<float>> _triplets;
  uint64_t _seed;
 public:
  Eigen::MatrixXf B;
  Eigen::SparseMatrix<float> Y;
  Eigen::MatrixXf Z;
  Eigen::MatrixXf U;
  bool _set_testing_components = false;
  vanilla_rand_svd_impl(VW::workspace* all, uint64_t d, uint64_t seed);
  void run(const multi_ex& examples, std::vector<float>& shrink_factors);
  bool generate_Y(const multi_ex& examples, std::vector<float>& shrink_factors);
  void generate_B(const multi_ex& examples, std::vector<float>& shrink_factors);
  // the below matrixes are used only during unit testing and are not set otherwise
  Eigen::VectorXf _S;
  Eigen::MatrixXf _V;
};

struct model_weight_rand_svd_impl
{
 private:
  VW::workspace* _all;
  uint64_t _d;
  std::vector<Eigen::Triplet<float>> _triplets;
  void cleanup_model_weight_Y(const multi_ex& examples);
  uint64_t _seed;
  dense_parameters _internal_weights;
 public:
  Eigen::MatrixXf B;
  Eigen::SparseMatrix<float> Y;
  Eigen::MatrixXf Z;
  Eigen::MatrixXf U;
  bool _set_testing_components = false;

  model_weight_rand_svd_impl(VW::workspace* all, uint64_t d, uint64_t seed);

  void run(const multi_ex& examples, std::vector<float>& shrink_factors);
  bool generate_model_weight_Y(const multi_ex& examples, uint64_t& max_existing_column, std::vector<float>& shrink_factors);
  void generate_B_model_weight(const multi_ex& examples, uint64_t max_existing_column, std::vector<float>& shrink_factors);

  // the below matrixes are used only during unit testing and are not set otherwise
  Eigen::VectorXf _S;
  Eigen::MatrixXf _V;
  // the below methods are used only during unit testing and are not called otherwise
  void _populate_from_model_weight_Y(const multi_ex& examples);
};

struct cb_explore_adf_large_action_space
{
private:
  float _gamma_scale;
  float _gamma_exponent;
  float _c = 2;
  bool _apply_shrink_factor;
  VW::workspace* _all;
  size_t _counter;
  implementation_type _impl_type;
  std::vector<Eigen::Triplet<float>> _triplets;
  std::vector<uint64_t> _action_indices;
  std::vector<bool> _spanner_bitvec;
  std::vector<std::vector<float>> _aatop_action_ft_vectors;
  std::vector<std::set<uint64_t>> _aatop_action_indexes;

public:
  uint64_t _d;
  uint64_t _seed;
  vanilla_rand_svd_impl _vanilla_rand_svd_impl;
  model_weight_rand_svd_impl _model_weight_rand_svd_impl;
  Eigen::MatrixXf AAtop;
  Eigen::MatrixXf U;
  std::vector<float> shrink_factors;
  // the below matrixes are used only during unit testing and are not set otherwise
  Eigen::SparseMatrix<float> _A;
  bool _set_testing_components = false;

  cb_explore_adf_large_action_space(uint64_t d, float gamma_scale, float gamma_exponent, float c,
      bool apply_shrink_factor, VW::workspace* all,
      implementation_type impl_type = implementation_type::vanilla_rand_svd);
  ~cb_explore_adf_large_action_space() = default;
  void save_load(io_buf& io, bool read, bool text);

  // Should be called through cb_explore_adf_base for pre/post-processing
  void predict(VW::LEARNER::multi_learner& base, multi_ex& examples);
  void learn(VW::LEARNER::multi_learner& base, multi_ex& examples);

  void calculate_shrink_factor(const ACTION_SCORE::action_scores& preds);
  bool generate_AAtop(const multi_ex& examples);
  void randomized_SVD(const multi_ex& examples);
  std::pair<float, uint64_t> find_max_volume(uint64_t x_row, Eigen::MatrixXf& X);
  void compute_spanner();

  // the below methods are used only during unit testing and are not called otherwise
  bool _generate_A(const multi_ex& examples);
  void _populate_all_testing_components();
  void _set_rank(uint64_t rank);

private:
  template <bool is_learn>
  void predict_or_learn_impl(VW::LEARNER::multi_learner& base, multi_ex& examples);
  void update_example_prediction(VW::multi_ex& examples);
};

template <typename TripletType>
void triplet_construction(TripletType& tc, float feature_value, uint64_t feature_index)
{
  tc.set(feature_value, feature_index);
}

void generate_Z(const multi_ex& examples, Eigen::MatrixXf& Z, Eigen::MatrixXf& B, uint64_t d, uint64_t seed);

}  // namespace cb_explore_adf
}  // namespace VW
