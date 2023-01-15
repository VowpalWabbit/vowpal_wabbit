// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "../large_action_space.h"
#include "qr_decomposition.h"
#include "vw/common/random.h"
#include "vw/core/cb.h"
#include "vw/core/label_dictionary.h"
#include "vw/core/reductions/gd.h"

namespace VW
{
namespace cb_explore_adf
{
class Y_triplet_constructor
{
public:
  Y_triplet_constructor(uint64_t weights_mask, uint64_t row_index, uint64_t column_index, uint64_t seed,
      std::vector<Eigen::Triplet<float>>& triplets, uint64_t& max_col, std::set<uint64_t>& non_zero_rows,
      const std::vector<float>& shrink_factors)
      : _weights_mask(weights_mask)
      , _row_index(row_index)
      , _column_index(column_index)
      , _seed(seed)
      , _triplets(triplets)
      , _max_col(max_col)
      , _non_zero_rows(non_zero_rows)
      , _shrink_factors(shrink_factors)
  {
  }

  void set(float feature_value, uint64_t index)
  {
    if (feature_value != 0.f)
    {
      _non_zero_rows.emplace(index);
      auto combined_index = _row_index + _column_index + _seed;
      // index is the equivalent of going over A's rows which turn out to be A.transpose()'s columns
      auto calc = feature_value * VW::details::merand48_boxmuller(combined_index) * _shrink_factors[_row_index];
      _triplets.emplace_back(Eigen::Triplet<float>(index & _weights_mask, _column_index, calc));
      if ((index & _weights_mask) > _max_col) { _max_col = (index & _weights_mask); }
    }
  }

private:
  uint64_t _weights_mask;
  uint64_t _row_index;
  uint64_t _column_index;
  uint64_t _seed;
  std::vector<Eigen::Triplet<float>>& _triplets;
  uint64_t& _max_col;
  std::set<uint64_t>& _non_zero_rows;
  const std::vector<float>& _shrink_factors;
};

class B_triplet_constructor
{
public:
  B_triplet_constructor(
      uint64_t weights_mask, uint64_t column_index, Eigen::SparseMatrix<float>& Y, float& final_dot_product)
      : _weights_mask(weights_mask), _column_index(column_index), _Y(Y), _final_dot_product(final_dot_product)
  {
  }

  void set(float feature_value, uint64_t index)
  {
    if (feature_value == 0.f) { return; }
    _final_dot_product += feature_value * _Y.coeffRef((index & _weights_mask), _column_index);
  }

private:
  uint64_t _weights_mask;
  uint64_t _column_index;
  Eigen::SparseMatrix<float>& _Y;
  float& _final_dot_product;
};

bool two_pass_svd_impl::generate_Y(const multi_ex& examples, const std::vector<float>& shrink_factors)
{
  uint64_t max_non_zero_col = 0;
  _triplets.clear();
  uint64_t row_index = 0;
  std::set<uint64_t> non_zero_rows;

  for (auto* ex : examples)
  {
    assert(!VW::ec_is_example_header_cb(*ex));

    auto& red_features = ex->ex_reduction_features.template get<VW::large_action_space::las_reduction_features>();
    auto* shared_example = red_features.shared_example;
    if (shared_example != nullptr) { VW::details::truncate_example_namespaces_from_example(*ex, *shared_example); }

    for (uint64_t col = 0; col < _d; col++)
    {
      if (_all->weights.sparse)
      {
        Y_triplet_constructor tc(_all->weights.sparse_weights.mask(), row_index, col, _seed, _triplets,
            max_non_zero_col, non_zero_rows, shrink_factors);
        VW::foreach_feature<Y_triplet_constructor, uint64_t, triplet_construction, sparse_parameters>(
            _all->weights.sparse_weights, _all->ignore_some_linear, _all->ignore_linear,
            (red_features.generated_interactions ? *red_features.generated_interactions : *ex->interactions),
            (red_features.generated_extent_interactions ? *red_features.generated_extent_interactions
                                                        : *ex->extent_interactions),
            _all->permutations, *ex, tc, _all->generate_interactions_object_cache_state);
      }
      else
      {
        Y_triplet_constructor tc(_all->weights.dense_weights.mask(), row_index, col, _seed, _triplets, max_non_zero_col,
            non_zero_rows, shrink_factors);
        VW::foreach_feature<Y_triplet_constructor, uint64_t, triplet_construction, dense_parameters>(
            _all->weights.dense_weights, _all->ignore_some_linear, _all->ignore_linear,
            (red_features.generated_interactions ? *red_features.generated_interactions : *ex->interactions),
            (red_features.generated_extent_interactions ? *red_features.generated_extent_interactions
                                                        : *ex->extent_interactions),
            _all->permutations, *ex, tc, _all->generate_interactions_object_cache_state);
      }
    }

    if (shared_example != nullptr) { VW::details::append_example_namespaces_from_example(*ex, *shared_example); }

    row_index++;
  }

  Y.resize(max_non_zero_col + 1, _d);
  Y.setZero();
  Y.setFromTriplets(_triplets.begin(), _triplets.end());
  // Orthonormalize Y
  VW::gram_schmidt(Y);

  return non_zero_rows.size() > _d;
}

void two_pass_svd_impl::generate_B(const multi_ex& examples, const std::vector<float>& shrink_factors)
{
  // create B matrix with dimenstions Kxd where K = examples.size()
  uint64_t num_actions = examples[0]->pred.a_s.size();
  B.resize(num_actions, _d);
  B.setZero();

  uint64_t row_index = 0;
  for (auto* ex : examples)
  {
    assert(!VW::ec_is_example_header_cb(*ex));

    auto& red_features = ex->ex_reduction_features.template get<VW::large_action_space::las_reduction_features>();
    auto* shared_example = red_features.shared_example;
    if (shared_example != nullptr) { VW::details::truncate_example_namespaces_from_example(*ex, *shared_example); }

    for (Eigen::Index col = 0; col < Y.outerSize(); ++col)
    {
      float final_dot_prod = 0.f;
      if (_all->weights.sparse)
      {
        B_triplet_constructor tc(_all->weights.sparse_weights.mask(), col, Y, final_dot_prod);
        VW::foreach_feature<B_triplet_constructor, uint64_t, triplet_construction, sparse_parameters>(
            _all->weights.sparse_weights, _all->ignore_some_linear, _all->ignore_linear,
            (red_features.generated_interactions ? *red_features.generated_interactions : *ex->interactions),
            (red_features.generated_extent_interactions ? *red_features.generated_extent_interactions
                                                        : *ex->extent_interactions),
            _all->permutations, *ex, tc, _all->generate_interactions_object_cache_state);
      }
      else
      {
        B_triplet_constructor tc(_all->weights.dense_weights.mask(), col, Y, final_dot_prod);
        VW::foreach_feature<B_triplet_constructor, uint64_t, triplet_construction, dense_parameters>(
            _all->weights.dense_weights, _all->ignore_some_linear, _all->ignore_linear,
            (red_features.generated_interactions ? *red_features.generated_interactions : *ex->interactions),
            (red_features.generated_extent_interactions ? *red_features.generated_extent_interactions
                                                        : *ex->extent_interactions),
            _all->permutations, *ex, tc, _all->generate_interactions_object_cache_state);
      }

      B(row_index, col) = shrink_factors[row_index] * final_dot_prod;
    }

    if (shared_example != nullptr) { VW::details::append_example_namespaces_from_example(*ex, *shared_example); }

    row_index++;
  }
}

void two_pass_svd_impl::_test_only_set_rank(uint64_t rank) { _d = rank; }

void two_pass_svd_impl::run(const multi_ex& examples, const std::vector<float>& shrink_factors, Eigen::MatrixXf& U,
    Eigen::VectorXf& S, Eigen::MatrixXf& _V)
{
  // This implementation is following the redsvd algorithm from this repo: https://github.com/ntessore/redsvd-h
  // It has been adapted so that all the matrixes do not need to be materialized and so that the implementation is
  // more natural to vw's example features representation

  // if the model is empty then can't create Y and there is nothing left to do
  if (!generate_Y(examples, shrink_factors) || Y.rows() < static_cast<Eigen::Index>(_d))
  {
    U.resize(0, 0);
    return;
  }

  generate_B(examples, shrink_factors);
  generate_Z(examples, Z, B, _d, _seed);

  Eigen::MatrixXf C = Z.transpose() * B;

  Eigen::JacobiSVD<Eigen::MatrixXf> svd(C, Eigen::ComputeThinU | Eigen::ComputeThinV);

  U = Z * svd.matrixU();
  S = svd.singularValues();

  if (_set_testing_components) { _V = Y * svd.matrixV(); }
}

two_pass_svd_impl::two_pass_svd_impl(VW::workspace* all, uint64_t d, uint64_t seed, size_t, size_t, size_t, bool)
    : _all(all), _d(d), _seed(seed)
{
}

}  // namespace cb_explore_adf
}  // namespace VW