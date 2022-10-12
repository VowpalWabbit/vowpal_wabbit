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
class A_triplet_constructor
{
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

private:
  uint64_t _weights_mask;
  uint64_t _row_index;
  std::vector<Eigen::Triplet<float>>& _triplets;
  uint64_t& _max_col;
};

bool _test_only_generate_A(VW::workspace* _all, const multi_ex& examples, std::vector<Eigen::Triplet<float>>& _triplets,
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

template <typename randomized_svd_impl, typename spanner_impl>
void cb_explore_adf_large_action_space<randomized_svd_impl, spanner_impl>::predict(
    VW::LEARNER::multi_learner& base, multi_ex& examples)
{
  predict_or_learn_impl<false>(base, examples);
}

template <typename randomized_svd_impl, typename spanner_impl>
void cb_explore_adf_large_action_space<randomized_svd_impl, spanner_impl>::learn(
    VW::LEARNER::multi_learner& base, multi_ex& examples)
{
  predict_or_learn_impl<true>(base, examples);
}

template <typename randomized_svd_impl, typename spanner_impl>
void cb_explore_adf_large_action_space<randomized_svd_impl, spanner_impl>::save_load(io_buf& io, bool read, bool text)
{
  if (io.num_files() == 0) { return; }
  std::stringstream msg;
  if (!read) { msg << "cb large action space storing example counter:  = " << _counter << "\n"; }
  bin_text_read_write_fixed_validated(io, reinterpret_cast<char*>(&_counter), sizeof(_counter), read, msg, text);
}

template <typename randomized_svd_impl, typename spanner_impl>
void cb_explore_adf_large_action_space<randomized_svd_impl, spanner_impl>::randomized_SVD(const multi_ex& examples)
{
  impl.run(examples, shrink_factors, U, _S, _V);
}

template <typename randomized_svd_impl, typename spanner_impl>
size_t cb_explore_adf_large_action_space<randomized_svd_impl, spanner_impl>::number_of_non_degenerate_singular_values()
{
  _non_degenerate_singular_values = 0;
  if (_S.size() > 0)
  {
    // sum the singular values
    auto sum_of_sv = _S.sum();

    // how many singular values represent 99% of the total sum of the singular values
    float current_sum_sv = 0;
    for (auto val : _S)
    {
      _non_degenerate_singular_values++;
      current_sum_sv += val;
      if (current_sum_sv > 0.99f * sum_of_sv) { break; }
    }
  }

  return _non_degenerate_singular_values;
}

template <typename randomized_svd_impl, typename spanner_impl>
void cb_explore_adf_large_action_space<randomized_svd_impl, spanner_impl>::update_example_prediction(
    VW::multi_ex& examples)
{
  auto& preds = examples[0]->pred.a_s;

  if (_d < preds.size())
  {
    shrink_fact_config.calculate_shrink_factor(_counter, _d, preds, shrink_factors);
    randomized_SVD(examples);

    // The U matrix is empty before learning anything.
    if (U.rows() == 0)
    {
      // Set uniform random probability for empty U.
      const float prob = 1.0f / preds.size();
      for (auto& pred : preds) { pred.score = prob; }
      return;
    }

    spanner_state.compute_spanner(U, _d, shrink_factors);

    assert(spanner_state.spanner_size() == preds.size());
  }
  else
  {
    // When the number of actions is not larger than d, all actions are selected.
    return;
  }

  // Keep only the actions in the spanner so they can be fed into the e-greedy or squarecb reductions.
  // Removed actions will be added back with zero probabilities in the cb_actions_mask reduction later
  // if the --full_predictions flag is supplied.
  auto it = preds.begin();
  while (it != preds.end())
  {
    if (!spanner_state.is_action_in_spanner(it->action)) { preds.erase(it); }
    else
    {
      it++;
    }
  }
}

template <typename randomized_svd_impl, typename spanner_impl>
template <bool is_learn>
void cb_explore_adf_large_action_space<randomized_svd_impl, spanner_impl>::predict_or_learn_impl(
    VW::LEARNER::multi_learner& base, multi_ex& examples)
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

template <typename T, typename S>
cb_explore_adf_large_action_space<T, S>::cb_explore_adf_large_action_space(uint64_t d, float gamma_scale,
    float gamma_exponent, float c, bool apply_shrink_factor, VW::workspace* all, uint64_t seed, size_t total_size,
    size_t thread_pool_size, size_t block_size, implementation_type impl_type)
    : _d(d)
    , _all(all)
    , _counter(0)
    , _seed(seed)
    , _impl_type(impl_type)
    , spanner_state(c, d)
    , shrink_fact_config(gamma_scale, gamma_exponent, apply_shrink_factor)
    , impl(all, d, _seed, total_size, thread_pool_size, block_size)
{
}

shrink_factor_config::shrink_factor_config(float gamma_scale, float gamma_exponent, bool apply_shrink_factor)
    : _gamma_scale(gamma_scale), _gamma_exponent(gamma_exponent), _apply_shrink_factor(apply_shrink_factor)
{
}

void shrink_factor_config::calculate_shrink_factor(
    size_t counter, size_t max_actions, const VW::action_scores& preds, std::vector<float>& shrink_factors)
{
  if (_apply_shrink_factor)
  {
    shrink_factors.clear();
    float min_ck = std::min_element(preds.begin(), preds.end())->score;
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

template class cb_explore_adf_large_action_space<one_pass_svd_impl, one_rank_spanner_state>;
template class cb_explore_adf_large_action_space<vanilla_rand_svd_impl, one_rank_spanner_state>;
template class cb_explore_adf_large_action_space<model_weight_rand_svd_impl, one_rank_spanner_state>;
}  // namespace cb_explore_adf
}  // namespace VW

template <typename T, typename S>
VW::LEARNER::base_learner* make_las_with_impl(VW::setup_base_i& stack_builder, VW::LEARNER::multi_learner* base,
    implementation_type& impl_type, VW::workspace& all, bool with_metrics, uint64_t d, float gamma_scale,
    float gamma_exponent, float c, bool apply_shrink_factor, size_t thread_pool_size, size_t block_size)
{
  using explore_type = cb_explore_adf_base<cb_explore_adf_large_action_space<T, S>>;

  size_t problem_multiplier = 1;

  uint64_t seed = all.get_random_state()->get_current_state() * 10.f;

  auto data = VW::make_unique<explore_type>(with_metrics, d, gamma_scale, gamma_exponent, c, apply_shrink_factor, &all,
      seed, 1 << all.num_bits, thread_pool_size, block_size, impl_type);

  auto* l = make_reduction_learner(std::move(data), base, explore_type::learn, explore_type::predict,
      stack_builder.get_setupfn_name(VW::reductions::cb_explore_adf_large_action_space_setup))
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
  bool model_weight_impl = false;
  bool use_vanilla_impl = false;
  bool full_spanner = false;
  uint64_t thread_pool_size = 0;
  uint64_t block_size = 0;

  config::option_group_definition new_options(
      "[Reduction] Experimental: Contextual Bandit Exploration with ADF with large action space");
  new_options
      .add(make_option("cb_explore_adf", cb_explore_adf_option)
               .keep()
               .necessary()
               .help("Online explore-exploit for a contextual bandit problem with multiline action dependent features"))
      .add(make_option("model_weight", model_weight_impl))
      .add(make_option("vanilla", use_vanilla_impl))
      .add(make_option("full_spanner", full_spanner))
      .add(make_option("thread_pool_size", thread_pool_size)
               .default_value(0)
               .help("number of threads in the thread pool that will be used when running with one pass svd "
                     "implementation (default svd implementation option)"))
      .add(make_option("block_size", block_size)
               .default_value(0)
               .help("number of actions in a block to be scheduled for multithreading when using one pass svd "
                     "implementation (by default, block_size = num_actions / thread_pool_size)"))
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

  if (model_weight_impl)
  {
    auto impl_type = implementation_type::model_weight_rand_svd;
    if (full_spanner)
    {
      return make_las_with_impl<model_weight_rand_svd_impl, spanner_state>(stack_builder, base, impl_type, all,
          with_metrics, d, gamma_scale, gamma_exponent, c, apply_shrink_factor, thread_pool_size, block_size);
    }
    else
    {
      return make_las_with_impl<model_weight_rand_svd_impl, one_rank_spanner_state>(stack_builder, base, impl_type, all,
          with_metrics, d, gamma_scale, gamma_exponent, c, apply_shrink_factor, thread_pool_size, block_size);
    }
  }
  else if (use_vanilla_impl)
  {
    auto impl_type = implementation_type::vanilla_rand_svd;
    if (full_spanner)
    {
      return make_las_with_impl<vanilla_rand_svd_impl, spanner_state>(stack_builder, base, impl_type, all, with_metrics,
          d, gamma_scale, gamma_exponent, c, apply_shrink_factor, thread_pool_size, block_size);
    }
    else
    {
      return make_las_with_impl<vanilla_rand_svd_impl, one_rank_spanner_state>(stack_builder, base, impl_type, all,
          with_metrics, d, gamma_scale, gamma_exponent, c, apply_shrink_factor, thread_pool_size, block_size);
    }
  }
  else
  {
    auto impl_type = implementation_type::one_pass_svd;
    if (full_spanner)
    {
      return make_las_with_impl<one_pass_svd_impl, spanner_state>(stack_builder, base, impl_type, all, with_metrics, d,
          gamma_scale, gamma_exponent, c, apply_shrink_factor, thread_pool_size, block_size);
    }
    else
    {
      return make_las_with_impl<one_pass_svd_impl, one_rank_spanner_state>(stack_builder, base, impl_type, all,
          with_metrics, d, gamma_scale, gamma_exponent, c, apply_shrink_factor, thread_pool_size, block_size);
    }
  }
}
