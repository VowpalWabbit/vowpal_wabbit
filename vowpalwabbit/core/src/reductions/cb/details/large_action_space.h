// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "vw/core/action_score.h"
#include "vw/core/array_parameters_dense.h"
#include "vw/core/rand48.h"
#include "vw/core/thread_pool.h"
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
  one_pass_svd
};

struct vanilla_rand_svd_impl
{
private:
  VW::workspace* _all;
  uint64_t _d;
  uint64_t _seed;
  std::vector<Eigen::Triplet<float>> _triplets;

public:
  Eigen::MatrixXf B;
  Eigen::SparseMatrix<float> Y;
  Eigen::MatrixXf Z;
  bool _set_testing_components = false;
  vanilla_rand_svd_impl(VW::workspace* all, uint64_t d, uint64_t seed, size_t total_size, size_t thread_pool_size);
  void run(const multi_ex& examples, const std::vector<float>& shrink_factors, Eigen::MatrixXf& U, Eigen::VectorXf& _S,
      Eigen::MatrixXf& _V);
  bool generate_Y(const multi_ex& examples, const std::vector<float>& shrink_factors);
  void generate_B(const multi_ex& examples, const std::vector<float>& shrink_factors);
  // testing only
  void _set_rank(uint64_t rank);
};

struct model_weight_rand_svd_impl
{
private:
  VW::workspace* _all;
  uint64_t _d;
  uint64_t _seed;
  dense_parameters _internal_weights;
  void cleanup_model_weight_Y(const multi_ex& examples);

public:
  Eigen::MatrixXf B;
  Eigen::SparseMatrix<float> Y;
  Eigen::MatrixXf Z;
  bool _set_testing_components = false;

  model_weight_rand_svd_impl(VW::workspace* all, uint64_t d, uint64_t seed, size_t total_size, size_t thread_pool_size);

  void run(const multi_ex& examples, const std::vector<float>& shrink_factors, Eigen::MatrixXf& U, Eigen::VectorXf& _S,
      Eigen::MatrixXf& _V);
  bool generate_model_weight_Y(
      const multi_ex& examples, uint64_t& max_existing_column, const std::vector<float>& shrink_factors);
  void generate_B_model_weight(
      const multi_ex& examples, uint64_t max_existing_column, const std::vector<float>& shrink_factors);

  // the below methods are used only during unit testing and are not called otherwise
  void _populate_from_model_weight_Y(const multi_ex& examples);
  void _set_rank(uint64_t rank);
};

struct one_pass_svd_impl
{
private:
  VW::workspace* _all;
  uint64_t _d;
  uint64_t _seed;
  thread_pool _thread_pool;
  std::vector<std::future<void>> _futures;
  Eigen::JacobiSVD<Eigen::MatrixXf> _svd;

public:
  Eigen::MatrixXf AOmega;
  one_pass_svd_impl(VW::workspace* all, uint64_t d, uint64_t seed, size_t total_size, size_t thread_pool_size);
  void run(const multi_ex& examples, const std::vector<float>& shrink_factors, Eigen::MatrixXf& U, Eigen::VectorXf& _S,
      Eigen::MatrixXf& _V);
  void generate_AOmega(const multi_ex& examples, const std::vector<float>& shrink_factors);
  void generate_AOmega_ot(const multi_ex& examples, const std::vector<float>& shrink_factors);
  // for testing purposes only
  void _set_rank(uint64_t rank);
  bool _set_testing_components = false;
};

struct shrink_factor_config
{
public:
  const float _gamma_scale;
  const float _gamma_exponent;
  const bool _apply_shrink_factor;
  shrink_factor_config(float gamma_scale, float gamma_exponent, bool apply_shrink_factor)
      : _gamma_scale(gamma_scale), _gamma_exponent(gamma_exponent), _apply_shrink_factor(apply_shrink_factor){};

  void calculate_shrink_factor(
      size_t counter, size_t max_actions, const ACTION_SCORE::action_scores& preds, std::vector<float>& shrink_factors);
};

struct spanner_state
{
private:
  const float _c = 2;
  Eigen::MatrixXf _X;

public:
  std::vector<bool> _spanner_bitvec;
  std::vector<uint64_t> _action_indices;
  spanner_state(float c, uint64_t d) : _c(c) { _action_indices.resize(d); };

  void compute_spanner(const Eigen::MatrixXf& U, size_t _d, const std::vector<float>& shrink_factors);
  void find_max_volume(
      const Eigen::MatrixXf& U, uint64_t X_rid, Eigen::MatrixXf& X, float& max_volume, uint64_t& U_rid);
};

struct one_rank_spanner_state
{
private:
  const float _c = 2;
  Eigen::MatrixXf _X;
  Eigen::MatrixXf _X_inv;
  float _log_determinant_factor = 0.f;

  void rank_one_determinant_update(
      const Eigen::MatrixXf& U, float max_volume, uint64_t U_rid, float shrink_factor, uint64_t row_iteration);
  void update_inverse(const Eigen::VectorXf& y, const Eigen::VectorXf& Xi, uint64_t row_iteration);
  void scale_all(float max_volume, uint64_t num_examples);

public:
  std::vector<bool> _spanner_bitvec;
  std::vector<uint64_t> _action_indices;
  one_rank_spanner_state(float c, uint64_t d) : _c(c) { _action_indices.resize(d); };
  void find_max_volume(const Eigen::MatrixXf& U, const Eigen::VectorXf& phi, float& max_volume, uint64_t& U_rid);
  void compute_spanner(const Eigen::MatrixXf& U, size_t _d, const std::vector<float>& shrink_factors);
};

inline void find_action_to_maximize_volume(
    const Eigen::MatrixXf& U, const Eigen::VectorXf& phi, float& max_volume, uint64_t& U_rid)
{
  // find which action (which row of U) will provide the maximum phi * a volume
  // that a will replace the current row in _X and provide the determinant with the maximum volume
  for (auto i = 0; i < U.rows(); ++i)
  {
    float vol = std::abs(U.row(i) * phi);
    if (vol > max_volume)
    {
      max_volume = vol;
      U_rid = i;
    }
  }
}

template <typename randomized_svd_impl, typename spanner_impl>
struct cb_explore_adf_large_action_space
{
private:
  VW::workspace* _all;
  uint64_t _d;
  uint64_t _seed;
  size_t _counter;
  implementation_type _impl_type;

public:
  spanner_impl _spanner_state;
  shrink_factor_config _shrink_factor_config;
  Eigen::MatrixXf U;
  std::vector<float> shrink_factors;
  bool _set_testing_components = false;
  randomized_svd_impl _impl;

  cb_explore_adf_large_action_space(uint64_t d, float gamma_scale, float gamma_exponent, float c,
      bool apply_shrink_factor, VW::workspace* all, uint64_t seed, size_t total_size, size_t thread_pool_size,
      implementation_type impl_type);

  ~cb_explore_adf_large_action_space() = default;

  void save_load(io_buf& io, bool read, bool text);
  // Should be called through cb_explore_adf_base for pre/post-processing
  void predict(VW::LEARNER::multi_learner& base, multi_ex& examples);
  void learn(VW::LEARNER::multi_learner& base, multi_ex& examples);

  void randomized_SVD(const multi_ex& examples);

  // the below methods are used only during unit testing and are not called otherwise
  void _populate_all_testing_components()
  {
    _set_testing_components = true;
    _impl._set_testing_components = true;
  }

  void _set_rank(uint64_t rank)
  {
    _d = rank;
    _impl._set_rank(rank);
    _spanner_state._action_indices.resize(_d);
  }

  // the below matrixes are used only during unit testing and are not set otherwise
  Eigen::SparseMatrix<float> _A;
  Eigen::VectorXf _S;
  Eigen::MatrixXf _V;

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
// the below methods are used only during unit testing and are not called otherwise
bool _generate_A(VW::workspace* _all, const multi_ex& examples, std::vector<Eigen::Triplet<float>>& _triplets,
    Eigen::SparseMatrix<float>& _A);

}  // namespace cb_explore_adf
}  // namespace VW
