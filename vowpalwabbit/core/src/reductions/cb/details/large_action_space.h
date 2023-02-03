// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "vw/core/action_score.h"
#include "vw/core/array_parameters_dense.h"
#include "vw/core/learner_fwd.h"
#include "vw/core/multi_ex.h"
#include "vw/core/thread_pool.h"
#include "vw/core/v_array.h"
#include "vw/core/vw_fwd.h"

// Eigen explicit vectorization may not work well with AVX2 when using smaller MAX_ALIGN_BYTES.
// For more info:
// https://eigen.tuxfamily.org/dox/TopicPreprocessorDirectives.html#TopicPreprocessorDirectivesPerformance
#define EIGEN_MAX_ALIGN_BYTES 32

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
  two_pass_svd,
  one_pass_svd
};

class two_pass_svd_impl
{
public:
  Eigen::MatrixXf B;
  Eigen::SparseMatrix<float> Y;
  Eigen::MatrixXf Z;

  two_pass_svd_impl(VW::workspace* all, uint64_t d, uint64_t seed, size_t total_size, size_t thread_pool_size,
      size_t block_size, bool use_explicit_simd);
  void run(const multi_ex& examples, const std::vector<float>& shrink_factors, Eigen::MatrixXf& U, Eigen::VectorXf& S,
      Eigen::MatrixXf& _V);
  bool generate_Y(const multi_ex& examples, const std::vector<float>& shrink_factors);
  void generate_B(const multi_ex& examples, const std::vector<float>& shrink_factors);

  // testing only
  void _test_only_set_rank(uint64_t rank);
  bool _set_testing_components = false;

private:
  VW::workspace* _all;
  uint64_t _d;
  uint64_t _seed;
  std::vector<Eigen::Triplet<float>> _triplets;
};

class one_pass_svd_impl
{
public:
  Eigen::MatrixXf AOmega;

  one_pass_svd_impl(VW::workspace* all, uint64_t d, uint64_t seed, size_t total_size, size_t thread_pool_size,
      size_t block_size, bool use_explicit_simd);
  void run(const multi_ex& examples, const std::vector<float>& shrink_factors, Eigen::MatrixXf& U, Eigen::VectorXf& S,
      Eigen::MatrixXf& _V);
  void generate_AOmega(const multi_ex& examples, const std::vector<float>& shrink_factors);

  // for testing purposes only
  void _test_only_set_rank(uint64_t rank);
  bool _set_testing_components = false;
#ifdef BUILD_LAS_WITH_SIMD
  bool _test_only_use_simd() { return _use_simd != simd_type::NO_SIMD; }
#endif

private:
  VW::workspace* _all;
  uint64_t _d;
  uint64_t _seed;
  thread_pool _thread_pool;
  size_t _block_size;
#ifdef BUILD_LAS_WITH_SIMD
  enum class simd_type
  {
    NO_SIMD,
    AVX2,
    AVX512
  };
  simd_type _use_simd = simd_type::NO_SIMD;
#endif
  std::vector<std::future<void>> _futures;
  Eigen::JacobiSVD<Eigen::MatrixXf> _svd;
};

class shrink_factor_config
{
public:
  const bool _apply_shrink_factor;
  shrink_factor_config(bool apply_shrink_factor);

  void calculate_shrink_factor(
      float gamma, size_t max_actions, const VW::action_scores& preds, std::vector<float>& shrink_factors);
};

class one_rank_spanner_state
{
public:
  one_rank_spanner_state(float c, uint64_t d) : _c(c), _action_indices(d), _log_determinant_factor(0.f){};
  void find_max_volume(const Eigen::MatrixXf& U, const Eigen::VectorXf& phi, float& max_volume, uint64_t& U_rid);
  void compute_spanner(const Eigen::MatrixXf& U, size_t _d, const std::vector<float>& shrink_factors);
  bool is_action_in_spanner(uint32_t action);
  size_t spanner_size();

  void _test_only_set_rank(uint64_t rank);

private:
  const float _c = 2;
  std::vector<uint64_t> _action_indices;
  float _log_determinant_factor;
  Eigen::MatrixXf _X;
  Eigen::MatrixXf _X_inv;
  std::vector<bool> _spanner_bitvec;

  void rank_one_determinant_update(
      const Eigen::MatrixXf& U, float max_volume, uint64_t U_rid, float shrink_factor, uint64_t row_iteration);
  void update_inverse(const Eigen::VectorXf& y, const Eigen::VectorXf& Xi, uint64_t row_iteration);
  void scale_all(float max_volume, uint64_t num_examples);
};

template <typename randomized_svd_impl, typename spanner_impl>
class cb_explore_adf_large_action_space
{
private:
  uint64_t _d;
  VW::workspace* _all;
  uint64_t _seed;
  implementation_type _impl_type;
  size_t _non_degenerate_singular_values;
  bool _set_testing_components = false;

public:
  spanner_impl spanner_state;
  shrink_factor_config shrink_fact_config;
  randomized_svd_impl impl;
  Eigen::MatrixXf U;
  std::vector<float> shrink_factors;
  Eigen::VectorXf S;

  // the below matrixes are used only during unit testing and are not set otherwise
  Eigen::SparseMatrix<float> _A;
  Eigen::MatrixXf _V;

  cb_explore_adf_large_action_space(uint64_t d, float c, bool apply_shrink_factor, VW::workspace* all, uint64_t seed,
      size_t total_size, size_t thread_pool_size, size_t block_size, bool use_explicit_simd,
      implementation_type impl_type);

  ~cb_explore_adf_large_action_space() = default;

  void save_load(io_buf& io, bool read, bool text);
  void predict(VW::LEARNER::learner& base, multi_ex& examples);
  void learn(VW::LEARNER::learner& base, multi_ex& examples);

  void randomized_SVD(const multi_ex& examples);

  size_t number_of_non_degenerate_singular_values();

  // the below methods are used only during unit testing and are not called otherwise
  void _populate_all_testing_components()
  {
    _set_testing_components = true;
    impl._set_testing_components = true;
  }

  void _test_only_set_rank(uint64_t rank)
  {
    _d = rank;
    impl._test_only_set_rank(rank);
    spanner_state._test_only_set_rank(rank);
  }

private:
  void update_example_prediction(VW::multi_ex& examples);
};

template <typename TripletType>
void triplet_construction(TripletType& tc, float feature_value, uint64_t feature_index)
{
  tc.set(feature_value, feature_index);
}

void generate_Z(const multi_ex& examples, Eigen::MatrixXf& Z, Eigen::MatrixXf& B, uint64_t d, uint64_t seed);
// the below methods are used only during unit testing and are not called otherwise
bool _test_only_generate_A(VW::workspace* _all, const multi_ex& examples, std::vector<Eigen::Triplet<float>>& _triplets,
    Eigen::SparseMatrix<float>& _A);

}  // namespace cb_explore_adf
}  // namespace VW
