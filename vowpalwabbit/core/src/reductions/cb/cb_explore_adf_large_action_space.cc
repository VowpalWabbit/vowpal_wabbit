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
template <typename WeightsT>
struct A_triplet_constructor
{
private:
  WeightsT& _weights;
  uint64_t _row_index;
  std::vector<Eigen::Triplet<float>>& _triplets;
  uint64_t& _max_col;

public:
  A_triplet_constructor(
      WeightsT& weights, uint64_t row_index, std::vector<Eigen::Triplet<float>>& triplets, uint64_t& max_col)
      : _weights(weights), _row_index(row_index), _triplets(triplets), _max_col(max_col)
  {
  }

  void set(uint64_t index)
  {
    if (_weights[index] != 0.f)
    {
      _triplets.emplace_back(Eigen::Triplet<float>(_row_index, index & _weights.mask(), _weights[index]));
      if ((index & _weights.mask()) > _max_col) { _max_col = (index & _weights.mask()); }
    }
  }
};

template <typename WeightsT>
struct Y_triplet_constructor
{
private:
  WeightsT& _weights;
  uint64_t _row_index;
  uint64_t _column_index;
  uint64_t _seed;
  std::vector<Eigen::Triplet<float>>& _triplets;
  uint64_t& _max_col;
  const std::vector<float>& _shrink_factors;

public:
  Y_triplet_constructor(WeightsT& weights, uint64_t row_index, uint64_t column_index, uint64_t seed,
      std::vector<Eigen::Triplet<float>>& triplets, uint64_t& max_col, const std::vector<float>& shrink_factors)
      : _weights(weights)
      , _row_index(row_index)
      , _column_index(column_index)
      , _seed(seed)
      , _triplets(triplets)
      , _max_col(max_col)
      , _shrink_factors(shrink_factors)
  {
  }

  void set(uint64_t index)
  {
    if (_weights[index] != 0.f)
    {
      auto combined_index = _row_index + _column_index + _seed;
      // _weights[index] is the equivalent of going over A's rows which turn out to be A.transpose()'s columns
      auto calc = _weights[index] * merand48_boxmuller(combined_index) * _shrink_factors[_row_index];
      _triplets.emplace_back(Eigen::Triplet<float>(index & _weights.mask(), _column_index, calc));
      if ((index & _weights.mask()) > _max_col) { _max_col = (index & _weights.mask()); }
    }
  }
};

template <typename WeightsT>
struct dot_product
{
private:
  WeightsT& _weights;
  uint64_t _column_index;
  Eigen::SparseMatrix<float>& _Y;

public:
  dot_product(WeightsT& weights, uint64_t column_index, Eigen::SparseMatrix<float>& Y)
      : _weights(weights), _column_index(column_index), _Y(Y)
  {
  }

  float operator[](uint64_t index) const
  {
    if (_weights[index] == 0.f) { return 0.f; }
    return _weights[index] * _Y.coeffRef((index & _weights.mask()), _column_index);
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

cb_explore_adf_large_action_space::cb_explore_adf_large_action_space(
    uint64_t d, float gamma, bool apply_shrink_factor, VW::workspace* all)
    : _d(d)
    , _gamma(gamma)
    , _apply_shrink_factor(apply_shrink_factor)
    , _all(all)
    , _seed(all->get_random_state()->get_current_state() * 10.f)
{
  _action_indices.resize(_d);
}

void cb_explore_adf_large_action_space::calculate_shrink_factor(const ACTION_SCORE::action_scores& preds)
{
  if (_apply_shrink_factor)
  {
    shrink_factors.clear();
    float min_ck = std::min_element(preds.begin(), preds.end(), VW::action_score_compare_lt)->score;
    for (size_t i = 0; i < preds.size(); i++)
    { shrink_factors.push_back(std::sqrt(1 + _d + _gamma / (4.0f * _d) * (preds[i].score - min_ck))); }
  }
  else
  {
    shrink_factors.resize(preds.size(), 1.f);
  }
}

inline void just_add_weights(float& p, float, float fw) { p += fw; }

template <typename TripletType>
void triplet_construction(TripletType& tc, float, uint64_t feature_index)
{
  tc.set(feature_index);
}

void cb_explore_adf_large_action_space::generate_Z(const multi_ex& examples)
{
  // create Z matrix with dimenstions Kxd where K = examples.size()
  // Z = B * P where P is a dxd gaussian matrix

  uint64_t num_actions = examples[0]->pred.a_s.size();
  Z.resize(num_actions, _d);
  Z.setZero();

  // TODO extend wildspace interactions before calling foreach
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

void cb_explore_adf_large_action_space::generate_B(const multi_ex& examples)
{
  // create B matrix with dimenstions Kxd where K = examples.size()
  uint64_t num_actions = examples[0]->pred.a_s.size();
  B.resize(num_actions, _d);
  B.setZero();

  // TODO extend wildspace interactions before calling foreach
  uint64_t row_index = 0;
  for (auto* ex : examples)
  {
    assert(!CB::ec_is_example_header(*ex));

    for (Eigen::Index col = 0; col < Y.outerSize(); ++col)
    {
      float final_dot_prod = 0.f;
      if (_all->weights.sparse)
      {
        dot_product<sparse_parameters> weights(_all->weights.sparse_weights, col, Y);
        GD::foreach_feature<float, float, just_add_weights, dot_product<sparse_parameters>>(weights,
            _all->ignore_some_linear, _all->ignore_linear, _all->interactions, _all->extent_interactions,
            _all->permutations, *ex, final_dot_prod, _all->_generate_interactions_object_cache);
      }
      else
      {
        dot_product<dense_parameters> weights(_all->weights.dense_weights, col, Y);
        GD::foreach_feature<float, float, just_add_weights, dot_product<dense_parameters>>(weights,
            _all->ignore_some_linear, _all->ignore_linear, _all->interactions, _all->extent_interactions,
            _all->permutations, *ex, final_dot_prod, _all->_generate_interactions_object_cache);
      }

      B(row_index, col) = shrink_factors[row_index] * final_dot_prod;
    }
    row_index++;
  }
}

bool cb_explore_adf_large_action_space::generate_Y(const multi_ex& examples)
{
  // TODO extend wildspace interactions before calling foreach
  uint64_t max_non_zero_col = 0;
  _triplets.clear();
  uint64_t row_index = 0;
  for (auto* ex : examples)
  {
    assert(!CB::ec_is_example_header(*ex));

    for (uint64_t col = 0; col < _d; col++)
    {
      if (_all->weights.sparse)
      {
        Y_triplet_constructor<sparse_parameters> tc(
            _all->weights.sparse_weights, row_index, col, _seed, _triplets, max_non_zero_col, shrink_factors);
        GD::foreach_feature<Y_triplet_constructor<sparse_parameters>, uint64_t, triplet_construction,
            sparse_parameters>(_all->weights.sparse_weights, _all->ignore_some_linear, _all->ignore_linear,
            _all->interactions, _all->extent_interactions, _all->permutations, *ex, tc,
            _all->_generate_interactions_object_cache);
      }
      else
      {
        Y_triplet_constructor<dense_parameters> tc(
            _all->weights.dense_weights, row_index, col, _seed, _triplets, max_non_zero_col, shrink_factors);
        GD::foreach_feature<Y_triplet_constructor<dense_parameters>, uint64_t, triplet_construction, dense_parameters>(
            _all->weights.dense_weights, _all->ignore_some_linear, _all->ignore_linear, _all->interactions,
            _all->extent_interactions, _all->permutations, *ex, tc, _all->_generate_interactions_object_cache);
      }
    }
    row_index++;
  }

  if (max_non_zero_col == 0)
  {
    // no non-zero columns were found for Y, it is empty
    Y.resize(0, 0);
  }
  else
  {
    Y.resize(max_non_zero_col + 1, _d);
    Y.setZero();
    Y.setFromTriplets(_triplets.begin(), _triplets.end());
    // Orthonormalize Y
    VW::gram_schmidt(Y);
  }

  return (Y.cols() != 0 && Y.rows() != 0);
}

bool cb_explore_adf_large_action_space::_generate_A(const multi_ex& examples)
{
  // TODO extend wildspace interactions before calling foreach
  uint64_t row_index = 0;
  uint64_t max_non_zero_col = 0;
  _triplets.clear();
  for (auto* ex : examples)
  {
    assert(!CB::ec_is_example_header(*ex));

    if (_all->weights.sparse)
    {
      A_triplet_constructor<sparse_parameters> w(_all->weights.sparse_weights, row_index, _triplets, max_non_zero_col);
      GD::foreach_feature<A_triplet_constructor<sparse_parameters>, uint64_t, triplet_construction, sparse_parameters>(
          _all->weights.sparse_weights, _all->ignore_some_linear, _all->ignore_linear, _all->interactions,
          _all->extent_interactions, _all->permutations, *ex, w, _all->_generate_interactions_object_cache);
    }
    else
    {
      A_triplet_constructor<dense_parameters> w(_all->weights.dense_weights, row_index, _triplets, max_non_zero_col);

      GD::foreach_feature<A_triplet_constructor<dense_parameters>, uint64_t, triplet_construction, dense_parameters>(
          _all->weights.dense_weights, _all->ignore_some_linear, _all->ignore_linear, _all->interactions,
          _all->extent_interactions, _all->permutations, *ex, w, _all->_generate_interactions_object_cache);
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

void cb_explore_adf_large_action_space::_populate_all_SVD_components() { _set_all_svd_components = true; }
void cb_explore_adf_large_action_space::set_rank(uint64_t rank) { _d = rank; }

void cb_explore_adf_large_action_space::randomized_SVD(const multi_ex& examples)
{
  // This implementation is following the redsvd algorithm from this repo: https://github.com/ntessore/redsvd-h
  // It has been adapted so that all the matrixes do not need to be materialized and so that the implementation is more
  // natural to vw's example features representation

  // TODO can Y be stored in the model? on some strided location ^^ ?
  // if the model is empty then can't create Y and there is nothing left to do
  if (!generate_Y(examples)) { return; }
  generate_B(examples);
  generate_Z(examples);

  Eigen::MatrixXf C = Z.transpose() * B;

  Eigen::JacobiSVD<Eigen::MatrixXf> svd(C, Eigen::ComputeThinU | Eigen::ComputeThinV);

  U = Z * svd.matrixU();

  if (_set_all_svd_components)
  {
    _V = Y * svd.matrixV();
    _S = svd.singularValues();
  }
}

std::pair<float, uint64_t> cb_explore_adf_large_action_space::find_max_volume(uint64_t X_rid, Eigen::MatrixXf& X)
{
  // Finds the max volume by replacing row X[X_rid] with some row in U.
  // Returns the max volume, and the row id of U used for replacing X[X_rid].

  float max_volume = -1.0f;
  uint64_t U_rid{};
  Eigen::RowVectorXf original_row = X.row(X_rid);

  for (auto i{0}; i < U.rows(); ++i)
  {
    X.row(X_rid) = U.row(i);
    float volume = abs(X.determinant());
    if (volume > max_volume)
    {
      max_volume = volume;
      U_rid = i;
    }
    X.row(X_rid) = original_row;
  }

  assert(max_volume >= 0.0f);
  return {max_volume, U_rid};
}

void cb_explore_adf_large_action_space::compute_spanner()
{
  // Implements the C-approximate barycentric spanner algorithm in Figure 2 of the following paper
  // Awerbuch & Kleinberg STOC'04: https://www.cs.cornell.edu/~rdk/papers/OLSP.pdf

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
  constexpr int C = 2;  // TODO make this a parameter?
  float X_volume = std::abs(X.determinant());
  for (int iter = 0; iter < static_cast<int>(_d * log(_d)); ++iter)
  {
    bool found_larger_volume = false;

    // If replacing some row in X results in larger volume, replace it with the row from U.
    for (uint64_t X_rid = 0; X_rid < _d; ++X_rid)
    {
      const auto max_volume_and_row_id = find_max_volume(X_rid, X);
      float max_volume = max_volume_and_row_id.first;
      if (max_volume > C * X_volume)
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

template <bool is_learn>
void cb_explore_adf_large_action_space::predict_or_learn_impl(VW::LEARNER::multi_learner& base, multi_ex& examples)
{
  if (is_learn) { base.learn(examples); }
  else
  {
    base.predict(examples);

    auto& preds = examples[0]->pred.a_s;

    const auto min_ck_idx = std::min_element(preds.begin(), preds.end(), VW::action_score_compare_lt) - preds.begin();
    const float min_ck = preds[min_ck_idx].score;

    if (_d < preds.size())
    {
      calculate_shrink_factor(preds);
      randomized_SVD(examples);

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
      _spanner_bitvec.clear();
      _spanner_bitvec.resize(preds.size(), true);
    }

    // Set the exploration distribution over S and the minimizer.
    _spanner_bitvec[min_ck_idx] = false;
    float sum_scores = 0.0f;
    for (auto i{0u}; i < preds.size(); ++i)
    {
      if (_spanner_bitvec[i])
      {
        preds[i].score = 1 / (1 + _d + _gamma / (4.0f * _d) * (preds[i].score - min_ck));
        sum_scores += preds[i].score;
      }
      else
      {
        preds[i].score = 0.0f;
      }
    }
    preds[min_ck_idx].score = 1.0f - sum_scores;
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
  float gamma = 0;
  bool apply_shrink_factor = false;

  config::option_group_definition new_options(
      "[Reduction] Experimental: Contextual Bandit Exploration with ADF with large action space");
  new_options
      .add(make_option("cb_explore_adf", cb_explore_adf_option)
               .keep()
               .necessary()
               .help("Online explore-exploit for a contextual bandit problem with multiline action dependent features"))
      .add(make_option("large_action_space", large_action_space)
               .necessary()
               .keep()
               .help("Large action space exploration")
               .experimental())
      .add(make_option("max_actions", d)
               .keep()
               .allow_override()
               .default_value(50)
               .help("Max number of actions to explore")
               .experimental());

  auto enabled = options.add_parse_and_check_necessary(new_options) && large_action_space;
  if (!enabled) { return nullptr; }

  // Ensure serialization of cb_adf in all cases.
  if (!options.was_supplied("cb_adf"))
  {
    options.insert("cb_adf", "");
    options.insert("no_predict", "");
  }

  if (options.was_supplied("squarecb"))
  {
    apply_shrink_factor = true;
    gamma = options.get_typed_option<float>("gamma_scale").value();
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

  size_t problem_multiplier = 1;

  VW::LEARNER::multi_learner* base = as_multiline(stack_builder.setup_base_learner());
  all.example_parser->lbl_parser = CB::cb_label;

  bool with_metrics = options.was_supplied("extra_metrics");

  using explore_type = cb_explore_adf_base<cb_explore_adf_large_action_space>;
  auto data = VW::make_unique<explore_type>(with_metrics, d, gamma, apply_shrink_factor, &all);

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
                .build(&all.logger);
  return make_base(*l);
}
