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
    : _c(c)
    , _shrink_factor_config(gamma_scale, gamma_exponent, apply_shrink_factor)
    , _all(all)
    , _counter(0)
    , _impl_type(impl_type)
    , _d(d)
    , _seed(all->get_random_state()->get_current_state() * 10.f)
    , _aatop_impl(all)
    , _vanilla_rand_svd_impl(all, d, _seed)
    , _model_weight_rand_svd_impl(all, d, _seed)
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

void cb_explore_adf_large_action_space::_populate_all_testing_components()
{
  _set_testing_components = true;
  _vanilla_rand_svd_impl._set_testing_components = true;
  _model_weight_rand_svd_impl._set_testing_components = true;
}

void cb_explore_adf_large_action_space::_set_rank(uint64_t rank)
{
  _d = rank;
  _vanilla_rand_svd_impl._d = rank;
  _model_weight_rand_svd_impl._d = rank;
  _action_indices.resize(_d);
}

void generate_Z(const multi_ex& examples, Eigen::MatrixXf& Z, Eigen::MatrixXf& B, uint64_t d, uint64_t seed)
{
  // create Z matrix with dimenstions Kxd where K = examples.size()
  // Z = B * P where P is a dxd gaussian matrix

  uint64_t num_actions = examples[0]->pred.a_s.size();
  Z.resize(num_actions, d);
  Z.setZero();

  for (Eigen::Index row = 0; row < B.rows(); row++)
  {
    for (uint64_t col = 0; col < d; col++)
    {
      for (uint64_t inner_index = 0; inner_index < d; inner_index++)
      {
        auto combined_index = inner_index + col + seed;
        auto dot_prod_prod = B(row, inner_index) * merand48_boxmuller(combined_index);
        Z(row, col) += dot_prod_prod;
      }
    }
  }
  VW::gram_schmidt(Z);
}

void shrink_factor_config::calculate_shrink_factor(
    size_t counter, size_t max_actions, const ACTION_SCORE::action_scores& preds, std::vector<float>& shrink_factors)
{
  if (_apply_shrink_factor)
  {
    shrink_factors.clear();
    float min_ck = std::min_element(preds.begin(), preds.end(), VW::action_score_compare_lt)->score;
    float gamma = _gamma_scale * static_cast<float>(std::pow(counter, _gamma_exponent));
    for (size_t i = 0; i < preds.size(); i++)
    {
      shrink_factors.push_back(std::sqrt(1 + max_actions + gamma / (4.0f * max_actions) * (preds[i].score - min_ck)));
    }
  }
  else
  {
    shrink_factors.resize(preds.size(), 1.f);
  }
}


void cb_explore_adf_large_action_space::randomized_SVD(const multi_ex& examples)
{
  if (_impl_type == implementation_type::aatop)
  {
    _aatop_impl.generate_AAtop(examples, shrink_factors);
    // TODO run svd here
  }
  else if (_impl_type == implementation_type::vanilla_rand_svd)
  {
    _vanilla_rand_svd_impl.run(examples, shrink_factors);
    // TODO: remove this overwrite of U after aatop becomes independent
    // spanner expects this U to be the one being operated on
    U = _vanilla_rand_svd_impl.U;
    if (_set_testing_components)
    {
      _V = _vanilla_rand_svd_impl._V;
      _S = _vanilla_rand_svd_impl._S;
    }
  }
  else if (_impl_type == implementation_type::model_weight_rand_svd)
  {
    _model_weight_rand_svd_impl.run(examples, shrink_factors);
    // TODO: remove this overwrite of U after aatop becomes independent
    // spanner expects this U to be the one being operated on
    U = _model_weight_rand_svd_impl.U;
    if (_set_testing_components)
    {
      _V = _model_weight_rand_svd_impl._V;
      _S = _model_weight_rand_svd_impl._S;
    }
  }
}

// spanner
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

// spanner
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
    _shrink_factor_config.calculate_shrink_factor(_counter, _d, preds, shrink_factors);
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
  if (model_weight_impl) { impl_type = implementation_type::model_weight_rand_svd; }

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
