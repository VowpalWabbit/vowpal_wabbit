// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "../large_action_space.h"
#include "compute_dot_prod_scalar.h"
#include "compute_dot_prod_simd.h"

namespace VW
{
namespace cb_explore_adf
{
/**
 * One pass SVD
 * one-pass refers to the the randomness we apply to the original A matrix, as opposed to two-pass randomized SVD which
 * is more commonly found in the literature and has a better reconstruction accuracy which we don't necessarily need
 *
 * Random sampling is used to identify a subspace that captures most of the action of a matrix
 * (https://arxiv.org/abs/0909.4061). By multiplying A with Omega, Omega being of dimensions Fxp, we are essentially
 * randomly sampling from A and projecting onto lower dimensions (p instead of F). We then proceed to apply SVD on
 * AOmega and truncate to d.
 *
 * A matrix with KxF dimenstions (sparse action features matrix)
 * Omega matrix with Fxp dimensions (Lazy Rademacher matrix)
 * AOmega is the dot product of A * Omega (matrix of dimenstions Fxp)
 * We then proceed to apply SVD on AOmega: (U, S, V) <- SVD(AOmega)
 * U <- U_d => truncate rows to d
 *
 * p = d + 10
 * one pass SVD is going to be less accurate than two pass SVD so we need to over-sample. This constant factor should be
 * enough, we need a higher probability that we get a fair coin flip in the Omega matrix
 *
 * The two basic properties we want to get from U after performing this truncated randomized SVD are:
 * 1. if an action is duplicated in A, the duplicates have the same representation in U
 * 2. if a third action is a linear combination of two other actions:
 *  a. one of the singular values is zero (or virtually close to zero)
 *  b. the third actions' representation in U is the linear combination of the other two actions representations in U
 *
 *
 * The matrices A and Omega are never materialized. The only matrix that is materialized is AOmega. We perform the
 * multiplication between A and Omega by keeping them in a lazy format: we iterate over A's rows (for each row), and for
 * every row in A we multiply it with the correct Omega column. We do that by calling foreach_feature on every
 * example-row and calculating the corresponding Omega cell that needs to be multiplied with the corresponding feature
 * (row's cell) on the fly, and adding the product to the final dotproduct corresponding to that example-row)
 */

void one_pass_svd_impl::generate_AOmega(const multi_ex& examples, const std::vector<float>& shrink_factors)
{
  auto num_actions = examples[0]->pred.a_s.size();
  // one pass SVD is going to be less accurate than two pass SVD so we need to over-sample
  // this constant factor should be enough, we need a higher probability that we get a fair coin flip in the Omega
  // matrix
  const uint64_t sampling_slack = 10;
  auto p = std::min(num_actions, static_cast<size_t>(_d + sampling_slack));
  const float scaling_factor = 1.f / std::sqrt(p);
  AOmega.resize(num_actions, p);

  auto compute_dot_prod = compute_dot_prod_scalar;
#ifdef BUILD_LAS_WITH_SIMD
  switch (_use_simd)
  {
    case (simd_type::AVX512):
      compute_dot_prod = compute_dot_prod_avx512;
      break;
    case (simd_type::AVX2):
      compute_dot_prod = compute_dot_prod_avx2;
      break;
    default:
      compute_dot_prod = compute_dot_prod_scalar;
  }
#endif

  auto calculate_aomega_row = [compute_dot_prod](uint64_t row_index_begin, uint64_t row_index_end, uint64_t p,
                                  VW::workspace* _all, uint64_t _seed, const multi_ex& examples,
                                  Eigen::MatrixXf& AOmega, const std::vector<float>& shrink_factors,
                                  float scaling_factor) -> void
  {
    for (auto row_index = row_index_begin; row_index < row_index_end; ++row_index)
    {
      VW::example* ex = examples[row_index];

      auto& red_features = ex->ex_reduction_features.template get<VW::large_action_space::las_reduction_features>();
      auto* shared_example = red_features.shared_example;
      if (shared_example != nullptr) { VW::details::truncate_example_namespaces_from_example(*ex, *shared_example); }

      for (uint64_t col = 0; col < p; ++col)
      {
        float final_dot_prod = compute_dot_prod(col, _all, _seed, ex);
        AOmega(row_index, col) = final_dot_prod * shrink_factors[row_index] * scaling_factor;
      }

      if (shared_example != nullptr) { VW::details::append_example_namespaces_from_example(*ex, *shared_example); }
    }
  };

  if (_block_size == 0)
  {
    // Compute block_size if not specified.
    const size_t num_blocks = std::max(size_t(1), this->_thread_pool.size());
    _block_size = std::max(size_t(1), examples.size() / num_blocks);  // Evenly split the examples into blocks
  }

  for (size_t row_index_begin = 0; row_index_begin < examples.size();)
  {
    size_t row_index_end = row_index_begin + _block_size;
    if ((row_index_end + _block_size) > examples.size()) { row_index_end = examples.size(); }

    _futures.emplace_back(_thread_pool.submit(calculate_aomega_row, row_index_begin, row_index_end, p, _all, _seed,
        std::cref(examples), std::ref(AOmega), std::cref(shrink_factors), scaling_factor));

    row_index_begin = row_index_end;
  }

  for (auto& ft : _futures) { ft.get(); }
  _futures.clear();
}

void one_pass_svd_impl::_test_only_set_rank(uint64_t rank) { _d = rank; }

void one_pass_svd_impl::run(const multi_ex& examples, const std::vector<float>& shrink_factors, Eigen::MatrixXf& U,
    Eigen::VectorXf& S, Eigen::MatrixXf& _V)
{
  generate_AOmega(examples, shrink_factors);
  _svd.compute(AOmega, Eigen::ComputeThinU | Eigen::ComputeThinV);
  U = _svd.matrixU().leftCols(_d);
  S = _svd.singularValues();

  if (_set_testing_components) { _V = _svd.matrixV(); }
}

one_pass_svd_impl::one_pass_svd_impl(VW::workspace* all, uint64_t d, uint64_t seed, size_t, size_t thread_pool_size,
    size_t block_size, bool use_explicit_simd)
    : _all(all), _d(d), _seed(seed), _thread_pool(thread_pool_size), _block_size(block_size)
{
#ifdef BUILD_LAS_WITH_SIMD
  _use_simd = simd_type::NO_SIMD;
  if (use_explicit_simd)
  {
    if (cpu_supports_avx512()) { _use_simd = simd_type::AVX512; }
    else if (cpu_supports_avx2()) { _use_simd = simd_type::AVX2; }
    else { all->logger.err_warn("System does not support AVX512 or AVX2. Using scalar code path."); }
  }
#else
  _UNUSED(use_explicit_simd);
#endif
}

}  // namespace cb_explore_adf
}  // namespace VW