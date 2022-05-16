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

  void set(uint64_t index) const
  {
    if (_weights[index] != 0.f)
    {
      _triplets.emplace_back(Eigen::Triplet<float>(_row_index, index, _weights[index]));
      if (index > _max_col) { _max_col = index; }
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

public:
  Y_triplet_constructor(WeightsT& weights, uint64_t row_index, uint64_t column_index, uint64_t seed,
      std::vector<Eigen::Triplet<float>>& triplets, uint64_t& max_col)
      : _weights(weights)
      , _row_index(row_index)
      , _column_index(column_index)
      , _seed(seed)
      , _triplets(triplets)
      , _max_col(max_col)
  {
  }

  void set(uint64_t index) const
  {
    if (_weights[index] != 0.f)
    {
      auto combined_index = _row_index + _column_index + _seed;
      auto calc = _weights[index] * merand48_boxmuller(combined_index);
      _triplets.emplace_back(Eigen::Triplet<float>(index, _column_index, calc));
      if (index > _max_col) { _max_col = index; }
    }
  }
};

template <typename WeightsT>
struct DotProduct
{
private:
  WeightsT& _weights;
  uint64_t _column_index;
  Eigen::SparseMatrix<float>& _Y;

public:
  DotProduct(WeightsT& weights, uint64_t column_index, Eigen::SparseMatrix<float>& Y)
      : _weights(weights), _column_index(column_index), _Y(Y)
  {
  }

  float operator[](uint64_t index) const { return _weights[index] * _Y.coeffRef(index, _column_index); }
};

void cb_explore_adf_large_action_space::predict(VW::LEARNER::multi_learner& base, multi_ex& examples)
{
  predict_or_learn_impl<false>(base, examples);
}
void cb_explore_adf_large_action_space::learn(VW::LEARNER::multi_learner& base, multi_ex& examples)
{
  predict_or_learn_impl<true>(base, examples);
}

cb_explore_adf_large_action_space::cb_explore_adf_large_action_space(uint64_t d, float gamma, VW::workspace* all)
    : _d(d), _gamma(gamma), _all(all), _seed(all->get_random_state()->get_current_state() * 10.f)
{
}

void cb_explore_adf_large_action_space::calculate_shrink_factor(const ACTION_SCORE::action_scores& preds, float min_ck)
{
  shrink_factors.clear();
  for (size_t i = 0; i < preds.size(); i++)
  { shrink_factors.push_back(std::sqrt(1 + _d + (_gamma / 4.0f * _d) * (preds[i].score - min_ck))); }
}

inline void just_add_weights(float& p, float, float fw) { p += fw; }

template <typename triplet_type>
inline void triplet_construction(triplet_type& tc, float, uint64_t feature_index)
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

    for (int col = 0; col < Y.outerSize(); ++col)
    {
      float dot_product = 0.f;
      if (_all->weights.sparse)
      {
        DotProduct<sparse_parameters> w(_all->weights.sparse_weights, col, Y);
        GD::foreach_feature<float, float, just_add_weights, DotProduct<sparse_parameters>>(w, _all->ignore_some_linear,
            _all->ignore_linear, _all->interactions, _all->extent_interactions, _all->permutations, *ex, dot_product,
            _all->_generate_interactions_object_cache);
      }
      else
      {
        DotProduct<dense_parameters> w(_all->weights.dense_weights, col, Y);
        GD::foreach_feature<float, float, just_add_weights, DotProduct<dense_parameters>>(w, _all->ignore_some_linear,
            _all->ignore_linear, _all->interactions, _all->extent_interactions, _all->permutations, *ex, dot_product,
            _all->_generate_interactions_object_cache);
      }

      B(row_index, col) = dot_product;
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
        Y_triplet_constructor<sparse_parameters> w(
            _all->weights.sparse_weights, row_index, col, _seed, _triplets, max_non_zero_col);
        GD::foreach_feature<Y_triplet_constructor<sparse_parameters>, uint64_t, triplet_construction,
            sparse_parameters>(_all->weights.sparse_weights, _all->ignore_some_linear, _all->ignore_linear,
            _all->interactions, _all->extent_interactions, _all->permutations, *ex, w,
            _all->_generate_interactions_object_cache);
      }
      else
      {
        Y_triplet_constructor<dense_parameters> w(
            _all->weights.dense_weights, row_index, col, _seed, _triplets, max_non_zero_col);
        GD::foreach_feature<Y_triplet_constructor<dense_parameters>, uint64_t, triplet_construction, dense_parameters>(
            _all->weights.dense_weights, _all->ignore_some_linear, _all->ignore_linear, _all->interactions,
            _all->extent_interactions, _all->permutations, *ex, w, _all->_generate_interactions_object_cache);
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

template <bool is_learn>
void cb_explore_adf_large_action_space::predict_or_learn_impl(VW::LEARNER::multi_learner& base, multi_ex& examples)
{
  if (is_learn) { base.learn(examples); }
  else
  {
    base.predict(examples);

    auto& preds = examples[0]->pred.a_s;

    float min_ck = std::min_element(preds.begin(), preds.end(),
        [](const ACTION_SCORE::action_score& a, const ACTION_SCORE::action_score& b) { return a.score < b.score; })
                       ->score;

    calculate_shrink_factor(preds, min_ck);
    if (_d < preds.size()) { randomized_SVD(examples); }

    // TODO apply spanner on U
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
  float gamma;

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
               .experimental())
      // TODO: Do we still need gamma if we aren't activating squarecb?
      .add(make_option("gamma", gamma).keep().allow_override().help("Gamma hyperparameter"));

  auto enabled = options.add_parse_and_check_necessary(new_options) && large_action_space;
  if (!enabled) { return nullptr; }

  // Ensure serialization of cb_adf in all cases.
  if (!options.was_supplied("cb_adf"))
  {
    options.insert("cb_adf", "");
    options.insert("no_predict", "");
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
  auto data = VW::make_unique<explore_type>(with_metrics, d, gamma, &all);

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
