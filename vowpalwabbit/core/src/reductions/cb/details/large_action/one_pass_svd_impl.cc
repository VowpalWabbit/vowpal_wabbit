// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "../large_action_space.h"
#include "vw/core/cb.h"
#include "vw/core/reductions/gd.h"
#ifdef _MSC_VER
#  include <intrin.h>
#endif

namespace VW
{
namespace cb_explore_adf
{
struct AO_triplet_constructor
{
private:
  uint64_t _weights_mask;
  uint64_t _row_index;
  uint64_t _column_index;
  uint64_t _seed;
  float& _final_dot_product;

public:
  AO_triplet_constructor(
      uint64_t weights_mask, uint64_t row_index, uint64_t column_index, uint64_t seed, float& final_dot_product)
      : _weights_mask(weights_mask)
      , _row_index(row_index)
      , _column_index(column_index)
      , _seed(seed)
      , _final_dot_product(final_dot_product)
  {
  }

  void set(float feature_value, uint64_t index)
  {
#ifdef _MSC_VER
    float val = ((__popcnt((index & _weights_mask) + _column_index + _seed) & 1) << 1) - 1.f;
#else
    float val = (__builtin_parity((index & _weights_mask) + _column_index + _seed) << 1) - 1.f;
#endif
    _final_dot_product += feature_value * val;
  }
};

void one_pass_svd_impl::generate_AOmega(const multi_ex& examples, const std::vector<float>& shrink_factors)
{
  auto num_actions = examples[0]->pred.a_s.size();
  // one pass SVD is going to be less accurate than two pass SVD so we need to over-sample
  // this constant factor should be enough, we need a higher probability that we get a fair coin flip in the Omega
  // matrix
  const uint64_t sampling_slack = 10;
  auto p = std::min(num_actions, _d + sampling_slack);
  AOmega.resize(num_actions, p);

  auto calculate_aomega_row = [](uint64_t row_index, uint64_t p, VW::workspace* _all, uint64_t _seed, VW::example* ex,
                                  Eigen::MatrixXf& AOmega, const std::vector<float>& shrink_factors) -> void {
    auto& red_features = ex->_reduction_features.template get<VW::generated_interactions::reduction_features>();

    for (uint64_t col = 0; col < p; ++col)
    {
      float final_dot_prod = 0.f;

      AO_triplet_constructor tc(_all->weights.mask(), row_index, col, _seed, final_dot_prod);

      GD::foreach_feature<AO_triplet_constructor, uint64_t, triplet_construction, dense_parameters>(
          _all->weights.dense_weights, _all->ignore_some_linear, _all->ignore_linear,
          (red_features.generated_interactions ? *red_features.generated_interactions : *ex->interactions),
          (red_features.generated_extent_interactions ? *red_features.generated_extent_interactions
                                                      : *ex->extent_interactions),
          _all->permutations, *ex, tc, _all->_generate_interactions_object_cache);

      AOmega(row_index, col) = final_dot_prod * shrink_factors[row_index];
    }
  };

  uint64_t row_index = 0;
  for (auto* ex : examples)
  {
    _futures.emplace_back(_thread_pool.submit(
        calculate_aomega_row, row_index, p, _all, _seed, ex, std::ref(AOmega), std ::ref(shrink_factors)));
    row_index++;
  }

  for (auto& ft : _futures) { ft.get(); }
  _futures.clear();
}

void one_pass_svd_impl::_set_rank(uint64_t rank) { _d = rank; }

void one_pass_svd_impl::run(const multi_ex& examples, const std::vector<float>& shrink_factors, Eigen::MatrixXf& U,
    Eigen::VectorXf& _S, Eigen::MatrixXf& _V)
{
  generate_AOmega(examples, shrink_factors);
  _svd.compute(AOmega, Eigen::ComputeThinU | Eigen::ComputeThinV);
  U = _svd.matrixU().leftCols(_d);
  if (_set_testing_components)
  {
    _S = _svd.singularValues();
    _V = _svd.matrixV();
  }
}

one_pass_svd_impl::one_pass_svd_impl(VW::workspace* all, uint64_t d, uint64_t seed, size_t, size_t thread_pool_size)
    : _all(all), _d(d), _seed(seed), _thread_pool(thread_pool_size)
{
}

}  // namespace cb_explore_adf
}  // namespace VW