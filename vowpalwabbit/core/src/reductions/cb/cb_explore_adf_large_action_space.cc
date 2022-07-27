// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/cb/cb_explore_adf_large_action_space.h"

#include "details/large_action_space.h"
#include "vw/config/options.h"
#include "vw/core/gd_predict.h"
#include "vw/core/label_parser.h"
#include "vw/core/qr_decomposition.h"
#include "vw/core/rand_state.h"
#include "vw/core/reductions/cb/cb_adf.h"
#include "vw/core/reductions/cb/cb_explore.h"
#include "vw/core/reductions/cb/cb_explore_adf_common.h"
#include "vw/core/setup_base.h"
#include "vw/explore/explore.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <vector>
using namespace VW::cb_explore_adf;

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

struct Y_triplet_constructor
{
private:
  uint64_t _weights_mask;
  uint64_t _row_index;
  uint64_t _column_index;
  uint64_t _seed;
  std::vector<Eigen::Triplet<float>>& _triplets;
  uint64_t& _max_col;
  std::set<uint64_t>& _non_zero_rows;
  const std::vector<float>& _shrink_factors;

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
      _non_zero_rows.emplace((index & _weights_mask));
      auto combined_index = _row_index + _column_index + _seed;
      // index is the equivalent of going over A's rows which turn out to be A.transpose()'s columns
      auto calc = feature_value * merand48_boxmuller(combined_index) * _shrink_factors[_row_index];
      _triplets.emplace_back(Eigen::Triplet<float>(index & _weights_mask, _column_index, calc));
      if ((index & _weights_mask) > _max_col) { _max_col = (index & _weights_mask); }
    }
  }
};

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

struct B_triplet_constructor
{
private:
  uint64_t _weights_mask;
  uint64_t _column_index;
  Eigen::SparseMatrix<float>& _Y;
  float& _final_dot_product;

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
};

void cb_explore_adf_large_action_space::predict(VW::LEARNER::multi_learner& base, multi_ex& examples)
{
  predict_or_learn_impl<false>(base, examples);
}
void cb_explore_adf_large_action_space::learn(VW::LEARNER::multi_learner& base, multi_ex& examples)
{
  predict_or_learn_impl<true>(base, examples);
}

cb_explore_adf_large_action_space::cb_explore_adf_large_action_space(uint64_t d, float gamma_scale,
    float gamma_exponent, float c, bool apply_shrink_factor, VW::workspace* all, implementation_type impl_type)
    : _d(d)
    , _gamma_scale(gamma_scale)
    , _gamma_exponent(gamma_exponent)
    , _c(c)
    , _apply_shrink_factor(apply_shrink_factor)
    , _all(all)
    , _seed(all->get_random_state()->get_current_state() * 10.f)
    , _counter(0)
    , _impl_type(impl_type)
    , _internal_weights(1 << _all->num_bits)
{
  _action_indices.resize(_d);
}

void cb_explore_adf_large_action_space::save_load(io_buf& io, bool read, bool text)
{
  if (io.num_files() == 0) { return; }
  std::stringstream msg;
  if (!read) { msg << "cb large action space storing example counter:  = " << _counter << "\n"; }
  bin_text_read_write_fixed_validated(io, reinterpret_cast<char*>(&_counter), sizeof(_counter), read, msg, text);
}

void cb_explore_adf_large_action_space::calculate_shrink_factor(const ACTION_SCORE::action_scores& preds)
{
  if (_apply_shrink_factor)
  {
    shrink_factors.clear();
    float min_ck = std::min_element(preds.begin(), preds.end(), VW::action_score_compare_lt)->score;
    float gamma = _gamma_scale * static_cast<float>(std::pow(_counter, _gamma_exponent));
    for (size_t i = 0; i < preds.size(); i++)
    { shrink_factors.push_back(std::sqrt(1 + _d + gamma / (4.0f * _d) * (preds[i].score - min_ck))); }
  }
  else
  {
    shrink_factors.resize(preds.size(), 1.f);
  }
}

template <typename TripletType>
void triplet_construction(TripletType& tc, float feature_value, uint64_t feature_index)
{
  tc.set(feature_value, feature_index);
}

void cb_explore_adf_large_action_space::generate_Z(const multi_ex& examples)
{
  // create Z matrix with dimenstions Kxd where K = examples.size()
  // Z = B * P where P is a dxd gaussian matrix

  uint64_t num_actions = examples[0]->pred.a_s.size();
  Z.resize(num_actions, _d);
  Z.setZero();

  for (Eigen::Index row = 0; row < B.rows(); row++)
  {
    for (uint64_t col = 0; col < _d; col++)
    {
      for (uint64_t inner_index = 0; inner_index < _d; inner_index++)
      {
        auto combined_index = inner_index + col + _seed;
        auto dot_prod_prod = B(row, inner_index) * merand48_boxmuller(combined_index);
        Z(row, col) += dot_prod_prod;
      }
    }
  }
  VW::gram_schmidt(Z);
}

void cb_explore_adf_large_action_space::generate_B_model_weight(const multi_ex& examples, uint64_t max_existing_column)
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

void cb_explore_adf_large_action_space::generate_B(const multi_ex& examples)
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

    for (Eigen::Index col = 0; col < Y.outerSize(); ++col)
    {
      float final_dot_prod = 0.f;
      if (_all->weights.sparse)
      {
        B_triplet_constructor tc(_all->weights.sparse_weights.mask(), col, Y, final_dot_prod);
        GD::foreach_feature<B_triplet_constructor, uint64_t, triplet_construction, sparse_parameters>(
            _all->weights.sparse_weights, _all->ignore_some_linear, _all->ignore_linear,
            (red_features.generated_interactions ? *red_features.generated_interactions : *ex->interactions),
            (red_features.generated_extent_interactions ? *red_features.generated_extent_interactions
                                                        : *ex->extent_interactions),
            _all->permutations, *ex, tc, _all->_generate_interactions_object_cache);
      }
      else
      {
        B_triplet_constructor tc(_all->weights.dense_weights.mask(), col, Y, final_dot_prod);
        GD::foreach_feature<B_triplet_constructor, uint64_t, triplet_construction, dense_parameters>(
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

bool cb_explore_adf_large_action_space::generate_AAtop(const multi_ex& examples)
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

bool cb_explore_adf_large_action_space::generate_Y(const multi_ex& examples)
{
  uint64_t max_non_zero_col = 0;
  _triplets.clear();
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
        Y_triplet_constructor tc(_all->weights.sparse_weights.mask(), row_index, col, _seed, _triplets,
            max_non_zero_col, non_zero_rows, shrink_factors);
        GD::foreach_feature<Y_triplet_constructor, uint64_t, triplet_construction, sparse_parameters>(
            _all->weights.sparse_weights, _all->ignore_some_linear, _all->ignore_linear,
            (red_features.generated_interactions ? *red_features.generated_interactions : *ex->interactions),
            (red_features.generated_extent_interactions ? *red_features.generated_extent_interactions
                                                        : *ex->extent_interactions),
            _all->permutations, *ex, tc, _all->_generate_interactions_object_cache);
      }
      else
      {
        Y_triplet_constructor tc(_all->weights.dense_weights.mask(), row_index, col, _seed, _triplets, max_non_zero_col,
            non_zero_rows, shrink_factors);
        GD::foreach_feature<Y_triplet_constructor, uint64_t, triplet_construction, dense_parameters>(
            _all->weights.dense_weights, _all->ignore_some_linear, _all->ignore_linear,
            (red_features.generated_interactions ? *red_features.generated_interactions : *ex->interactions),
            (red_features.generated_extent_interactions ? *red_features.generated_extent_interactions
                                                        : *ex->extent_interactions),
            _all->permutations, *ex, tc, _all->_generate_interactions_object_cache);
      }
    }
    row_index++;
  }

  Y.resize(max_non_zero_col + 1, _d);
  Y.setZero();
  Y.setFromTriplets(_triplets.begin(), _triplets.end());
  // Orthonormalize Y
  VW::gram_schmidt(Y);

  return non_zero_rows.size() > _d;
}

void cb_explore_adf_large_action_space::_populate_from_model_weight_Y(const multi_ex& examples)
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

void cb_explore_adf_large_action_space::cleanup_model_weight_Y(const multi_ex& examples)
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

bool cb_explore_adf_large_action_space::generate_model_weight_Y(const multi_ex& examples, uint64_t& max_existing_column)
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

bool cb_explore_adf_large_action_space::_generate_A(const multi_ex& examples)
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

void cb_explore_adf_large_action_space::_populate_all_testing_components() { _set_testing_components = true; }
void cb_explore_adf_large_action_space::_set_rank(uint64_t rank)
{
  _d = rank;
  _action_indices.resize(_d);
}

void cb_explore_adf_large_action_space::randomized_SVD(const multi_ex& examples)
{
  if (_impl_type == implementation_type::aatop)
  {
    generate_AAtop(examples);
    // TODO run svd here
  }
  else if (_impl_type == implementation_type::vanilla_rand_svd)
  {
    // This implementation is following the redsvd algorithm from this repo: https://github.com/ntessore/redsvd-h
    // It has been adapted so that all the matrixes do not need to be materialized and so that the implementation is
    // more natural to vw's example features representation

    // TODO can Y be stored in the model? on some strided location ^^ ?
    // if the model is empty then can't create Y and there is nothing left to do
    if (!generate_Y(examples) || Y.rows() < static_cast<Eigen::Index>(_d))
    {
      U.resize(0, 0);
      return;
    }

    generate_B(examples);
    generate_Z(examples);

    Eigen::MatrixXf C = Z.transpose() * B;

    Eigen::JacobiSVD<Eigen::MatrixXf> svd(C, Eigen::ComputeThinU | Eigen::ComputeThinV);

    U = Z * svd.matrixU();

    if (_set_testing_components)
    {
      _V = Y * svd.matrixV();
      _S = svd.singularValues();
    }
  }
  else if (_impl_type == implementation_type::model_weight_rand_svd)
  {
    uint64_t max_existing_column = 0;
    if (!generate_model_weight_Y(examples, max_existing_column))
    {
      U.resize(0, 0);
      return;
    }

    if (_set_testing_components) { _populate_from_model_weight_Y(examples); }

    generate_B_model_weight(examples, max_existing_column);
    cleanup_model_weight_Y(examples);

    generate_Z(examples);

    Eigen::MatrixXf C = Z.transpose() * B;

    Eigen::JacobiSVD<Eigen::MatrixXf> svd(C, Eigen::ComputeThinU | Eigen::ComputeThinV);

    U = Z * svd.matrixU();

    if (_set_testing_components)
    {
      _V = Y * svd.matrixV();
      _S = svd.singularValues();
    }
  }
}

std::pair<float, uint64_t> cb_explore_adf_large_action_space::find_max_volume(uint64_t X_rid, Eigen::MatrixXf& X)
{
  // Finds the max volume by replacing row X[X_rid] with some row in U.
  // Returns the max volume, and the row id of U used for replacing X[X_rid].

  float max_volume = -1.0f;
  uint64_t U_rid{};
  const Eigen::RowVectorXf original_row = X.row(X_rid);

  for (auto i = 0; i < U.rows(); ++i)
  {
    X.row(X_rid) = U.row(i);
    const float volume = std::abs(X.determinant());
    if (volume > max_volume)
    {
      max_volume = volume;
      U_rid = i;
    }
  }
  X.row(X_rid) = original_row;

  assert(max_volume >= 0.0f);
  return {max_volume, U_rid};
}

void cb_explore_adf_large_action_space::compute_spanner()
{
  // Implements the C-approximate barycentric spanner algorithm in Figure 2 of the following paper
  // Awerbuch & Kleinberg STOC'04: https://www.cs.cornell.edu/~rdk/papers/OLSP.pdf

  // The size of U is K x d, where K is the total number of all actions.
  assert(static_cast<uint64_t>(U.cols()) == _d);
  Eigen::MatrixXf X = Eigen::MatrixXf::Identity(_d, _d);

  // Compute a basis contained in U.
  for (uint64_t X_rid = 0; X_rid < _d; ++X_rid)
  {
    uint64_t U_rid = find_max_volume(X_rid, X).second;
    X.row(X_rid) = U.row(U_rid);
    _action_indices[X_rid] = U_rid;
  }

  // Transform the basis into C-approximate spanner.
  // According to the paper, the total number of iterations needed is O(d*log_c(d)).
  const int max_iterations = static_cast<int>(_d * std::log(_d) / std::log(_c));
  float X_volume = std::abs(X.determinant());
  for (int iter = 0; iter < max_iterations; ++iter)
  {
    bool found_larger_volume = false;

    // If replacing some row in X results in larger volume, replace it with the row from U.
    for (uint64_t X_rid = 0; X_rid < _d; ++X_rid)
    {
      const auto max_volume_and_row_id = find_max_volume(X_rid, X);
      const float max_volume = max_volume_and_row_id.first;
      if (max_volume > _c * X_volume)
      {
        uint64_t U_rid = max_volume_and_row_id.second;
        X.row(X_rid) = U.row(U_rid);
        _action_indices[X_rid] = U_rid;

        X_volume = max_volume;
        found_larger_volume = true;
        break;
      }
    }

    if (!found_larger_volume) { break; }
  }

  _spanner_bitvec.clear();
  _spanner_bitvec.resize(U.rows(), false);
  for (uint64_t idx : _action_indices) { _spanner_bitvec[idx] = true; }
}

void cb_explore_adf_large_action_space::update_example_prediction(VW::multi_ex& examples)
{
  auto& preds = examples[0]->pred.a_s;

  if (_d < preds.size())
  {
    calculate_shrink_factor(preds);
    randomized_SVD(examples);

    // The U matrix is empty before learning anything.
    if (U.rows() == 0)
    {
      // Set uniform random probability for empty U.
      const float prob = 1.0f / preds.size();
      for (auto& pred : preds) { pred.score = prob; }
      return;
    }

    compute_spanner();
    assert(_spanner_bitvec.size() == preds.size());
  }
  else
  {
    // When the number of actions is not larger than d, all actions are selected.
    _spanner_bitvec.clear();
    _spanner_bitvec.resize(preds.size(), true);
  }

  // Keep only the actions in the spanner so they can be fed into the e-greedy or squarecb reductions.
  // Removed actions will be added back with zero probabilities in the cb_actions_mask reduction later
  // if the --full_predictions flag is supplied.
  size_t index = 0;
  for (auto it = preds.begin(); it != preds.end(); it++)
  {
    if (!_spanner_bitvec[index]) { preds.erase(it--); }
    index++;
  }
}

template <bool is_learn>
void cb_explore_adf_large_action_space::predict_or_learn_impl(VW::LEARNER::multi_learner& base, multi_ex& examples)
{
  if (is_learn)
  {
    base.learn(examples);
    ++_counter;
  }
  else
  {
    base.predict(examples);
    update_example_prediction(examples);
  }
}

}  // namespace cb_explore_adf
}  // namespace VW

VW::LEARNER::base_learner* VW::reductions::cb_explore_adf_large_action_space_setup(VW::setup_base_i& stack_builder)
{
  VW::config::options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();
  using config::make_option;
  bool cb_explore_adf_option = false;
  bool large_action_space = false;
  uint64_t d;
  float gamma_scale = 1.f;
  float gamma_exponent = 0.f;
  float c;
  bool apply_shrink_factor = false;
  bool full_predictions = false;
  bool aatop = false;
  bool model_weight_impl = false;

  config::option_group_definition new_options(
      "[Reduction] Experimental: Contextual Bandit Exploration with ADF with large action space");
  new_options
      .add(make_option("cb_explore_adf", cb_explore_adf_option)
               .keep()
               .necessary()
               .help("Online explore-exploit for a contextual bandit problem with multiline action dependent features"))
      .add(make_option("aatop", aatop))
      .add(make_option("model_weight", model_weight_impl))
      .add(make_option("large_action_space", large_action_space)
               .necessary()
               .keep()
               .help("Large action space exploration")
               .experimental())
      .add(make_option("full_predictions", full_predictions)
               .help("Full representation of the prediction's action probabilities")
               .experimental())
      .add(make_option("max_actions", d)
               .keep()
               .allow_override()
               .default_value(50)
               .help("Max number of actions to explore")
               .experimental())
      .add(make_option("spanner_c", c)
               .keep()
               .allow_override()
               .default_value(2)
               .help("Parameter for computing c-approximate spanner")
               .experimental());

  auto enabled = options.add_parse_and_check_necessary(new_options) && large_action_space;
  if (!enabled) { return nullptr; }

  if (options.was_supplied("squarecb"))
  {
    apply_shrink_factor = true;
    gamma_scale = options.get_typed_option<float>("gamma_scale").value();
    gamma_exponent = options.get_typed_option<float>("gamma_exponent").value();
  }

  if (options.was_supplied("cb_type"))
  {
    auto cb_type = options.get_typed_option<std::string>("cb_type").value();
    if (cb_type != "mtr")
    {
      all.logger.err_warn(
          "Only cb_type 'mtr' is currently supported with large action spaces, resetting to mtr. Input was: '{}'",
          cb_type);
      options.replace("cb_type", "mtr");
    }
  }

  VW::LEARNER::multi_learner* base = as_multiline(stack_builder.setup_base_learner());
  all.example_parser->lbl_parser = CB::cb_label;

  bool with_metrics = options.was_supplied("extra_metrics");

  using explore_type = cb_explore_adf_base<cb_explore_adf_large_action_space>;

  size_t problem_multiplier = 1;

  auto impl_type = implementation_type::vanilla_rand_svd;
  if (aatop) { impl_type = implementation_type::aatop; }
  if (model_weight_impl)
  {
    impl_type = implementation_type::model_weight_rand_svd;
  }

  auto data = VW::make_unique<explore_type>(
      with_metrics, d, gamma_scale, gamma_exponent, c, apply_shrink_factor, &all, impl_type);

  auto* l = make_reduction_learner(std::move(data), base, explore_type::learn, explore_type::predict,
      stack_builder.get_setupfn_name(cb_explore_adf_large_action_space_setup))
                .set_input_label_type(VW::label_type_t::cb)
                .set_output_label_type(VW::label_type_t::cb)
                .set_input_prediction_type(VW::prediction_type_t::action_scores)
                .set_output_prediction_type(VW::prediction_type_t::action_scores)
                .set_params_per_weight(problem_multiplier)
                .set_finish_example(explore_type::finish_multiline_example)
                .set_print_example(explore_type::print_multiline_example)
                .set_persist_metrics(explore_type::persist_metrics)
                .set_save_load(explore_type::save_load)
                .build(&all.logger);
  return make_base(*l);
}
