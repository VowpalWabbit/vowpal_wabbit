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
struct A_triplet_constructor
{
private:
  uint64_t _weights_mask;
  uint64_t _row_index;
  std::vector<Eigen::Triplet<float>>& _triplets;
  uint64_t& _max_col;

public:
  A_triplet_constructor(
      uint64_t weights_mask, uint64_t row_index, std::vector<Eigen::Triplet<float>>& triplets, uint64_t& max_col)
      : _weights_mask(weights_mask), _row_index(row_index), _triplets(triplets), _max_col(max_col)
  {
  }

  void set(float feature_value, uint64_t index)
  {
    if (feature_value != 0.f)
    {
      _triplets.emplace_back(Eigen::Triplet<float>(_row_index, index & _weights_mask, feature_value));
      if ((index & _weights_mask) > _max_col) { _max_col = (index & _weights_mask); }
    }
  }
};

struct AAtop_triplet_constructor
{
private:
  uint64_t _weights_mask;
  std::vector<float>& _vec_mult;
  std::set<uint64_t>& _indexes;

public:
  AAtop_triplet_constructor(uint64_t weights_mask, std::vector<float>& vec_mult, std::set<uint64_t>& indexes)
      : _weights_mask(weights_mask), _vec_mult(vec_mult), _indexes(indexes)
  {
  }

  void set(float feature_value, uint64_t index)
  {
    auto wi = index & _weights_mask;
    _vec_mult[wi] = feature_value;
    _indexes.insert(wi);
  }
};

bool _generate_A(VW::workspace* _all, const multi_ex& examples, std::vector<Eigen::Triplet<float>>& _triplets,
    Eigen::SparseMatrix<float>& _A)
{
  uint64_t row_index = 0;
  uint64_t max_non_zero_col = 0;
  _triplets.clear();
  for (auto* ex : examples)
  {
    assert(!CB::ec_is_example_header(*ex));

    auto& red_features = ex->_reduction_features.template get<VW::generated_interactions::reduction_features>();

    if (_all->weights.sparse)
    {
      A_triplet_constructor w(_all->weights.sparse_weights.mask(), row_index, _triplets, max_non_zero_col);
      GD::foreach_feature<A_triplet_constructor, uint64_t, triplet_construction, sparse_parameters>(
          _all->weights.sparse_weights, _all->ignore_some_linear, _all->ignore_linear,
          (red_features.generated_interactions ? *red_features.generated_interactions : *ex->interactions),
          (red_features.generated_extent_interactions ? *red_features.generated_extent_interactions
                                                      : *ex->extent_interactions),
          _all->permutations, *ex, w, _all->_generate_interactions_object_cache);
    }
    else
    {
      A_triplet_constructor w(_all->weights.dense_weights.mask(), row_index, _triplets, max_non_zero_col);

      GD::foreach_feature<A_triplet_constructor, uint64_t, triplet_construction, dense_parameters>(
          _all->weights.dense_weights, _all->ignore_some_linear, _all->ignore_linear,
          (red_features.generated_interactions ? *red_features.generated_interactions : *ex->interactions),
          (red_features.generated_extent_interactions ? *red_features.generated_extent_interactions
                                                      : *ex->extent_interactions),
          _all->permutations, *ex, w, _all->_generate_interactions_object_cache);
    }

    row_index++;
  }

  assert(row_index == examples[0]->pred.a_s.size());
  if (max_non_zero_col == 0)
  {
    // no non-zero columns were found for A, it is empty
    _A.resize(0, 0);
  }
  else
  {
    _A.resize(row_index, max_non_zero_col + 1);
    _A.setZero();
    _A.setFromTriplets(_triplets.begin(), _triplets.end());
  }

  return (_A.cols() != 0 && _A.rows() != 0);
}

bool aatop_impl::run(const multi_ex& examples, const std::vector<float>& shrink_factors)
{
  _triplets.clear();
  AAtop.resize(examples.size(), examples.size());
  _aatop_action_ft_vectors.clear();
  _aatop_action_indexes.clear();
  _aatop_action_indexes.resize(examples.size());
  _aatop_action_ft_vectors.resize(examples.size());

  for (size_t i = 0; i < examples.size(); ++i)
  {
    _aatop_action_ft_vectors[i].clear();
    _aatop_action_indexes[i].clear();
    _aatop_action_ft_vectors[i].resize(_all->weights.mask() + 1, 0.f);
    auto& red_features =
        examples[i]->_reduction_features.template get<VW::generated_interactions::reduction_features>();

    AAtop_triplet_constructor tc(_all->weights.mask(), _aatop_action_ft_vectors[i], _aatop_action_indexes[i]);
    GD::foreach_feature<AAtop_triplet_constructor, uint64_t, triplet_construction, dense_parameters>(
        _all->weights.dense_weights, _all->ignore_some_linear, _all->ignore_linear,
        (red_features.generated_interactions ? *red_features.generated_interactions : *examples[i]->interactions),
        (red_features.generated_extent_interactions ? *red_features.generated_extent_interactions
                                                    : *examples[i]->extent_interactions),
        _all->permutations, *examples[i], tc, _all->_generate_interactions_object_cache);
  }

  for (size_t i = 0; i < examples.size(); ++i)
  {
    for (size_t j = i; j < examples.size(); ++j)
    {
      float prod = 0.f;
      for (uint64_t index : _aatop_action_indexes[j])
      {
        if (_aatop_action_ft_vectors[i][index] != 0.f)
        { prod += _aatop_action_ft_vectors[j][index] * _aatop_action_ft_vectors[i][index]; }
      }

      prod *= shrink_factors[i] * shrink_factors[j];
      AAtop(i, j) = prod;
      AAtop(j, i) = prod;
    }
  }
  return true;
}

aatop_impl::aatop_impl(VW::workspace* all) : _all(all) {}

}  // namespace cb_explore_adf
}  // namespace VW