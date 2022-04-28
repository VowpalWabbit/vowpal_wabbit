// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/cb/cb_explore_adf_large_action_space.h"

#include "details/large_action_space.h"
#include "vw/config/options.h"
#include "vw/core/gd_predict.h"
#include "vw/core/label_parser.h"
#include "vw/core/rand_state.h"
#include "vw/core/reductions/cb/cb_adf.h"
#include "vw/core/reductions/cb/cb_explore.h"
#include "vw/core/reductions/cb/cb_explore_adf_common.h"
#include "vw/core/setup_base.h"
#include "vw/explore/explore.h"

#include <Eigen/QR>
#include <algorithm>
#include <cmath>
#include <functional>
#include <vector>
using namespace VW::cb_explore_adf;

namespace VW
{
namespace cb_explore_adf
{
// this is meant to be called with foreach_feature
// for each feature it will multiply the weight that corresponds to the index with the corresponding gaussian element
// If the returned values are summed the end resutlt is the dot product between the weight vector of the features of an
// example with a vector of gaussian elements
template <typename WeightsT>
struct LazyGaussianDotProduct
{
private:
  WeightsT& _weights;
  uint64_t _column_index;
  uint64_t _seed;

public:
  LazyGaussianDotProduct(WeightsT& weights, uint64_t column_index, uint64_t seed)
      : _weights(weights), _column_index(column_index), _seed(seed)
  {
  }
  float operator[](uint64_t index) const
  {
    auto combined_index = index + _column_index + _seed;
    return _weights[index] * merand48_boxmuller(combined_index);
  }
};

template <typename WeightsT>
struct triplet_constructor
{
private:
  WeightsT& _weights;
  uint64_t _row_index;
  std::vector<Eigen::Triplet<float>>& _triplets;
  uint64_t& _max_col;

public:
  triplet_constructor(
      WeightsT& weights, uint64_t row_index, std::vector<Eigen::Triplet<float>>& triplets, uint64_t& max_col)
      : _weights(weights), _row_index(row_index), _triplets(triplets), _max_col(max_col)
  {
  }

  float operator[](uint64_t index) const
  {
    if (_weights[index] != 0.f)
    {
      _triplets.push_back(Eigen::Triplet<float>(_row_index, index, _weights[index]));
      if (index > _max_col) { _max_col = index; }
    }
    // no op operation
    return 0.f;
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

cb_explore_adf_large_action_space::cb_explore_adf_large_action_space(uint64_t d, float gamma, VW::workspace* all)
    : _d(d), _gamma(gamma), _all(all), _seed(all->get_random_state()->get_current_state())
{
}

void cb_explore_adf_large_action_space::calculate_shrink_factor(const ACTION_SCORE::action_scores& preds, float min_ck)
{
  shrink_factors.clear();
  for (size_t i = 0; i < preds.size(); i++)
  { shrink_factors.push_back(std::sqrt(1 + _d + (_gamma / 4.0f * _d) * (preds[i].score - min_ck))); }
}

inline void just_add_weights(float& p, float, float fw) { p += fw; }

inline void no_op(float&, float, float) {}

void cb_explore_adf_large_action_space::generate_A(const multi_ex& examples)
{
  // TODO extend wildspace interactions before calling foreach
  uint64_t row_index = 0;
  uint64_t max_col = 0;
  // TODO check triplets type
  _triplets.clear();
  for (auto* ex : examples)
  {
    assert(!CB::ec_is_example_header(*ex));

    float no_op_float = 0.f;
    if (_all->weights.sparse)
    {
      triplet_constructor<sparse_parameters> w(_all->weights.sparse_weights, row_index, _triplets, max_col);
      GD::foreach_feature<float, float, no_op, triplet_constructor<sparse_parameters>>(w, _all->ignore_some_linear,
          _all->ignore_linear, _all->interactions, _all->extent_interactions, _all->permutations, *ex, no_op_float,
          _all->_generate_interactions_object_cache);
    }
    else
    {
      triplet_constructor<dense_parameters> w(_all->weights.dense_weights, row_index, _triplets, max_col);
      GD::foreach_feature<float, float, no_op, triplet_constructor<dense_parameters>>(w, _all->ignore_some_linear,
          _all->ignore_linear, _all->interactions, _all->extent_interactions, _all->permutations, *ex, no_op_float,
          _all->_generate_interactions_object_cache);
    }

    row_index++;
  }

  assert(row_index == examples[0]->pred.a_s.size());
  if (max_col == 0)
  {
    // no non-zero columns were found for A, it is empty
    A.resize(0, 0);
  }
  else
  {
    A.resize(row_index, max_col + 1);
    A.setFromTriplets(_triplets.begin(), _triplets.end());
  }
}

void cb_explore_adf_large_action_space::generate_Q(const multi_ex& examples)
{
  // create Q matrix with dimenstions Kxd where K = examples.size()
  uint64_t num_actions = examples[0]->pred.a_s.size();
  Q.resize(num_actions, _d);

  // TODO extend wildspace interactions before calling foreach
  uint64_t row_index = 0;
  for (auto* ex : examples)
  {
    assert(!CB::ec_is_example_header(*ex));

    for (size_t col = 0; col < _d; col++)
    {
      float dot_product = 0.f;
      if (_all->weights.sparse)
      {
        LazyGaussianDotProduct<sparse_parameters> w(_all->weights.sparse_weights, col, _seed);
        GD::foreach_feature<float, float, just_add_weights, LazyGaussianDotProduct<sparse_parameters>>(w,
            _all->ignore_some_linear, _all->ignore_linear, _all->interactions, _all->extent_interactions,
            _all->permutations, *ex, dot_product, _all->_generate_interactions_object_cache);
      }
      else
      {
        LazyGaussianDotProduct<dense_parameters> w(_all->weights.dense_weights, col, _seed);
        GD::foreach_feature<float, float, just_add_weights, LazyGaussianDotProduct<dense_parameters>>(w,
            _all->ignore_some_linear, _all->ignore_linear, _all->interactions, _all->extent_interactions,
            _all->permutations, *ex, dot_product, _all->_generate_interactions_object_cache);
      }

      Q(row_index, col) = dot_product;
    }
    row_index++;
  }
}

void cb_explore_adf_large_action_space::QR_decomposition()
{
  Eigen::MatrixXf thinQ(Eigen::MatrixXf::Identity(Q.rows(), Q.cols()));
  Eigen::HouseholderQR<Eigen::MatrixXf> qr(Q);
  Q = qr.householderQ() * thinQ;
}

// void cb_explore_adf_large_action_space::generate_Z(const multi_ex& examples)
// {
//   // create Z matrix with dimenstions d x F where d is a parameter and F is the max number of feature representation
//   Eigen::MatrixXf Z;

//   // TODO extend wildspace interactions before calling foreach

//   // To get Z we want to multiply Q.transpose() with the original A matrix (of action features)
//   // to achieve this we will iterate over the columns of Q (which are the rows of Q.transpose()) and multiply each q
//   // vector with a column in A. A column in A The inner product of Q.transpose().row (i.e. Q.column()) with
//   A.column() for (uint64_t row_index = 0; row_index < Q.cols(); row_index++) {} for (auto* ex : examples)
//   {
//     assert(!CB::ec_is_example_header(*ex));

//     for (size_t col = 0; col < _d; col++)
//     {
//       float dot_product = 0.f;
//       if (_all->weights.sparse)
//       {
//         LazyGaussianDotProduct<sparse_parameters> w(_all->weights.sparse_weights, col, _seed);
//         GD::foreach_feature<float, float, just_add_weights, LazyGaussianDotProduct<sparse_parameters>>(w,
//             _all->ignore_some_linear, _all->ignore_linear, _all->interactions, _all->extent_interactions,
//             _all->permutations, *ex, dot_product, _all->_generate_interactions_object_cache);
//       }
//       else
//       {
//         LazyGaussianDotProduct<dense_parameters> w(_all->weights.dense_weights, col, _seed);
//         GD::foreach_feature<float, float, just_add_weights, LazyGaussianDotProduct<dense_parameters>>(w,
//             _all->ignore_some_linear, _all->ignore_linear, _all->interactions, _all->extent_interactions,
//             _all->permutations, *ex, dot_product, _all->_generate_interactions_object_cache);
//       }

//       Q(row_index, col) = dot_product;
//     }
//     row_index++;
//   }
// }

template <bool is_learn>
void cb_explore_adf_large_action_space::predict_or_learn_impl(VW::LEARNER::multi_learner& base, multi_ex& examples)
{
  // Explore uniform random an epsilon fraction of the time.
  if (is_learn) { base.learn(examples); }
  else
  {
    base.predict(examples);

    auto& preds = examples[0]->pred.a_s;
    float min_ck = std::min_element(preds.begin(), preds.end(),
        [](ACTION_SCORE::action_score& a, ACTION_SCORE::action_score& b) { return a.score < b.score; })
                       ->score;

    calculate_shrink_factor(preds, min_ck);
    generate_A(examples);
    generate_Q(examples);
    QR_decomposition();
    // generate_Z(examples);
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
               .help("Large action space exploration"))
      .add(make_option("max_actions", d)
               .keep()
               .allow_override()
               .default_value(50)
               .help("Max number of actions to explore"))
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
