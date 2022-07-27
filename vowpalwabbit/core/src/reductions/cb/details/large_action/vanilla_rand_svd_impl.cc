#include "../large_action_space.h"
#include "vw/core/cb.h"
#include "vw/core/qr_decomposition.h"
#include "vw/core/reductions/gd.h"

namespace VW
{
namespace cb_explore_adf
{
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

}  // namespace cb_explore_adf
}  // namespace VW