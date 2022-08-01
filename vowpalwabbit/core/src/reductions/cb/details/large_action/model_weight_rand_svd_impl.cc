// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "../large_action_space.h"
#include "vw/core/cb.h"
#include "vw/core/qr_decomposition.h"
#include "vw/core/reductions/gd.h"

namespace VW
{
namespace cb_explore_adf
{
struct Y_constructor
{
  uint64_t _weights_mask;
  dense_parameters& _weights;
  uint64_t _row_index;
  uint64_t _column_index;
  uint64_t _seed;
  std::set<uint64_t>& _non_zero_rows;
  const std::vector<float>& _shrink_factors;

public:
  Y_constructor(uint64_t weights_mask, dense_parameters& weights, uint64_t row_index, uint64_t column_index,
      uint64_t seed, std::set<uint64_t>& _non_zero_rows, const std::vector<float>& shrink_factors)
      : _weights_mask(weights_mask)
      , _weights(weights)
      , _row_index(row_index)
      , _column_index(column_index)
      , _seed(seed)
      , _non_zero_rows(_non_zero_rows)
      , _shrink_factors(shrink_factors)
  {
  }

  void set(float feature_value, uint64_t index)
  {
    if (feature_value != 0.f)
    {
      _non_zero_rows.emplace(index & _weights_mask);
      auto strided_index = (index & _weights_mask) + _column_index;
      auto combined_index = _row_index + _column_index + _seed;
      auto calc = feature_value * merand48_boxmuller(combined_index) * _shrink_factors[_row_index];
      _weights[strided_index] += calc;
    }
  }
};

struct Y_destructor
{
  uint64_t _weights_mask;
  dense_parameters& _weights;
  uint64_t _column_index;

public:
  Y_destructor(uint64_t weights_mask, dense_parameters& weights, uint64_t column_index)
      : _weights_mask(weights_mask), _weights(weights), _column_index(column_index)
  {
  }

  void set(float feature_value, uint64_t index)
  {
    if (feature_value != 0.f)
    {
      auto strided_index = (index & _weights_mask) + _column_index;
      _weights[strided_index] = 0.f;  // TODO initial weight defined by cli
    }
  }
};

struct Y_triplet_populator
{
  uint64_t _weights_mask;
  dense_parameters& _weights;
  std::vector<Eigen::Triplet<float>>& _triplets;
  uint64_t _column_index;
  uint64_t& _max_col;

public:
  Y_triplet_populator(uint64_t weights_mask, dense_parameters& weights, std::vector<Eigen::Triplet<float>>& triplets,
      uint64_t column_index, uint64_t& max_col)
      : _weights_mask(weights_mask)
      , _weights(weights)
      , _triplets(triplets)
      , _column_index(column_index)
      , _max_col(max_col)
  {
  }

  void set(float, uint64_t index)
  {
    auto strided_index = (index & _weights_mask) + _column_index;
    if (_weights[strided_index] != 0.f)
    {
      _triplets.emplace_back(Eigen::Triplet<float>((index & _weights_mask), _column_index, _weights[strided_index]));
      if ((index & _weights_mask) > _max_col) { _max_col = (index & _weights_mask); }
    }
  }
};

struct A_times_Y_dot_product
{
private:
  uint64_t _weights_mask;
  dense_parameters& _weights;
  uint64_t _column_index;
  float& _final_dot_product;

public:
  A_times_Y_dot_product(
      uint64_t weights_mask, dense_parameters& weights, uint64_t column_index, float& final_dot_product)
      : _weights_mask(weights_mask)
      , _weights(weights)
      , _column_index(column_index)
      , _final_dot_product(final_dot_product)
  {
  }

  void set(float feature_value, uint64_t index)
  {
    if (feature_value == 0.f) { return; }
    auto strided_index = (index & _weights_mask) + _column_index;
    _final_dot_product += feature_value * _weights[strided_index];
  }
};

bool model_weight_rand_svd_impl::generate_model_weight_Y(
    const multi_ex& examples, uint64_t& max_existing_column, const std::vector<float>& shrink_factors)
{
  uint64_t row_index = 0;
  std::set<uint64_t> non_zero_rows;
  for (auto* ex : examples)
  {
    assert(!CB::ec_is_example_header(*ex));

    auto& red_features = ex->_reduction_features.template get<VW::generated_interactions::reduction_features>();
    for (uint64_t col = 0; col < _d; col++)
    {
      if (_all->weights.sparse)
      {
        Y_constructor tc_sparse(
            _all->weights.mask(), _internal_weights, row_index, col, _seed, non_zero_rows, shrink_factors);

        GD::foreach_feature<Y_constructor, uint64_t, triplet_construction, sparse_parameters>(
            _all->weights.sparse_weights, _all->ignore_some_linear, _all->ignore_linear,
            (red_features.generated_interactions ? *red_features.generated_interactions : *ex->interactions),
            (red_features.generated_extent_interactions ? *red_features.generated_extent_interactions
                                                        : *ex->extent_interactions),
            _all->permutations, *ex, tc_sparse, _all->_generate_interactions_object_cache);
      }
      else
      {
        Y_constructor tc_dense(
            _all->weights.mask(), _internal_weights, row_index, col, _seed, non_zero_rows, shrink_factors);

        GD::foreach_feature<Y_constructor, uint64_t, triplet_construction, dense_parameters>(
            _all->weights.dense_weights, _all->ignore_some_linear, _all->ignore_linear,
            (red_features.generated_interactions ? *red_features.generated_interactions : *ex->interactions),
            (red_features.generated_extent_interactions ? *red_features.generated_extent_interactions
                                                        : *ex->extent_interactions),
            _all->permutations, *ex, tc_dense, _all->_generate_interactions_object_cache);
      }
    }
    row_index++;
  }

  if (non_zero_rows.empty())
  {
    // no non-zero columns were found for Y, it is empty
    max_existing_column = 0;
    return false;
  }
  else
  {
    // Orthonormalize Y
    max_existing_column = VW::gram_schmidt(_internal_weights, _d, non_zero_rows);
    return non_zero_rows.size() > _d;
  }

  return false;
}

void model_weight_rand_svd_impl::generate_B_model_weight(
    const multi_ex& examples, uint64_t max_existing_column, const std::vector<float>& shrink_factors)
{
  // create B matrix with dimenstions Kxd where K = examples.size()
  uint64_t num_actions = examples[0]->pred.a_s.size();
  B.resize(num_actions, _d);
  B.setZero();

  uint64_t row_index = 0;
  for (auto* ex : examples)
  {
    assert(!CB::ec_is_example_header(*ex));

    auto& red_features = ex->_reduction_features.template get<VW::generated_interactions::reduction_features>();

    for (uint64_t col = 0; col < max_existing_column; ++col)
    {
      float final_dot_prod = 0.f;
      if (_all->weights.sparse)
      {
        A_times_Y_dot_product tc(_all->weights.mask(), _internal_weights, col, final_dot_prod);
        GD::foreach_feature<A_times_Y_dot_product, uint64_t, triplet_construction, sparse_parameters>(
            _all->weights.sparse_weights, _all->ignore_some_linear, _all->ignore_linear,
            (red_features.generated_interactions ? *red_features.generated_interactions : *ex->interactions),
            (red_features.generated_extent_interactions ? *red_features.generated_extent_interactions
                                                        : *ex->extent_interactions),
            _all->permutations, *ex, tc, _all->_generate_interactions_object_cache);
      }
      else
      {
        A_times_Y_dot_product tc(_all->weights.mask(), _internal_weights, col, final_dot_prod);
        GD::foreach_feature<A_times_Y_dot_product, uint64_t, triplet_construction, dense_parameters>(
            _all->weights.dense_weights, _all->ignore_some_linear, _all->ignore_linear,
            (red_features.generated_interactions ? *red_features.generated_interactions : *ex->interactions),
            (red_features.generated_extent_interactions ? *red_features.generated_extent_interactions
                                                        : *ex->extent_interactions),
            _all->permutations, *ex, tc, _all->_generate_interactions_object_cache);
      }

      B(row_index, col) = shrink_factors[row_index] * final_dot_prod;
    }
    row_index++;
  }
}

void model_weight_rand_svd_impl::_populate_from_model_weight_Y(const multi_ex& examples)
{
  std::vector<Eigen::Triplet<float>> triplets;
  uint64_t max_non_zero_col = 0;
  for (auto* ex : examples)
  {
    assert(!CB::ec_is_example_header(*ex));

    auto& red_features = ex->_reduction_features.template get<VW::generated_interactions::reduction_features>();
    for (uint64_t col = 0; col < _d; col++)
    {
      if (_all->weights.sparse)
      {
        Y_triplet_populator tc_sparse(_all->weights.mask(), _internal_weights, triplets, col, max_non_zero_col);

        GD::foreach_feature<Y_triplet_populator, uint64_t, triplet_construction, sparse_parameters>(
            _all->weights.sparse_weights, _all->ignore_some_linear, _all->ignore_linear,
            (red_features.generated_interactions ? *red_features.generated_interactions : *ex->interactions),
            (red_features.generated_extent_interactions ? *red_features.generated_extent_interactions
                                                        : *ex->extent_interactions),
            _all->permutations, *ex, tc_sparse, _all->_generate_interactions_object_cache);
      }
      else
      {
        Y_triplet_populator tc_dense(_all->weights.mask(), _internal_weights, triplets, col, max_non_zero_col);

        GD::foreach_feature<Y_triplet_populator, uint64_t, triplet_construction, dense_parameters>(
            _all->weights.dense_weights, _all->ignore_some_linear, _all->ignore_linear,
            (red_features.generated_interactions ? *red_features.generated_interactions : *ex->interactions),
            (red_features.generated_extent_interactions ? *red_features.generated_extent_interactions
                                                        : *ex->extent_interactions),
            _all->permutations, *ex, tc_dense, _all->_generate_interactions_object_cache);
      }
    }
  }

  Y.resize(max_non_zero_col + 1, _d);
  Y.setZero();

  Y.setFromTriplets(triplets.begin(), triplets.end(), [](const float& a, const float& b) {
    assert(a == b);
    return b;
  });
}

void model_weight_rand_svd_impl::cleanup_model_weight_Y(const multi_ex& examples)
{
  for (auto* ex : examples)
  {
    assert(!CB::ec_is_example_header(*ex));

    auto& red_features = ex->_reduction_features.template get<VW::generated_interactions::reduction_features>();
    for (uint64_t col = 0; col < _d; col++)
    {
      if (_all->weights.sparse)
      {
        Y_destructor tc_sparse(_all->weights.mask(), _internal_weights, col);
        GD::foreach_feature<Y_destructor, uint64_t, triplet_construction, sparse_parameters>(
            _all->weights.sparse_weights, _all->ignore_some_linear, _all->ignore_linear,
            (red_features.generated_interactions ? *red_features.generated_interactions : *ex->interactions),
            (red_features.generated_extent_interactions ? *red_features.generated_extent_interactions
                                                        : *ex->extent_interactions),
            _all->permutations, *ex, tc_sparse, _all->_generate_interactions_object_cache);
      }
      else
      {
        Y_destructor tc_dense(_all->weights.mask(), _internal_weights, col);
        GD::foreach_feature<Y_destructor, uint64_t, triplet_construction, dense_parameters>(_all->weights.dense_weights,
            _all->ignore_some_linear, _all->ignore_linear,
            (red_features.generated_interactions ? *red_features.generated_interactions : *ex->interactions),
            (red_features.generated_extent_interactions ? *red_features.generated_extent_interactions
                                                        : *ex->extent_interactions),
            _all->permutations, *ex, tc_dense, _all->_generate_interactions_object_cache);
      }
    }
  }
}

void model_weight_rand_svd_impl::_set_rank(uint64_t rank) { _d = rank; }

void model_weight_rand_svd_impl::run(const multi_ex& examples, const std::vector<float>& shrink_factors,
    Eigen::MatrixXf& U, Eigen::VectorXf& _S, Eigen::MatrixXf& _V)
{
  uint64_t max_existing_column = 0;
  if (!generate_model_weight_Y(examples, max_existing_column, shrink_factors))
  {
    U.resize(0, 0);
    return;
  }

  if (_set_testing_components) { _populate_from_model_weight_Y(examples); }

  generate_B_model_weight(examples, max_existing_column, shrink_factors);
  cleanup_model_weight_Y(examples);

  generate_Z(examples, Z, B, _d, _seed);

  Eigen::MatrixXf C = Z.transpose() * B;

  Eigen::JacobiSVD<Eigen::MatrixXf> svd(C, Eigen::ComputeThinU | Eigen::ComputeThinV);

  U = Z * svd.matrixU();

  if (_set_testing_components)
  {
    _V = Y * svd.matrixV();
    _S = svd.singularValues();
  }
}

model_weight_rand_svd_impl::model_weight_rand_svd_impl(VW::workspace* all, uint64_t d, uint64_t seed, size_t total_size)
    : _all(all), _d(d), _seed(seed), _internal_weights(total_size)
{
}

}  // namespace cb_explore_adf
}  // namespace VW