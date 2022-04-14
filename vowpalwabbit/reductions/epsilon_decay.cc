// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "epsilon_decay.h"

#include "global_data.h"
#include "label_type.h"
#include "learner.h"
#include "memory.h"
#include "model_utils.h"
#include "prediction_type.h"
#include "vw.h"
#include "vw/config/options.h"

#include <algorithm>
#include <cmath>
#include <string>

using namespace VW::config;
using namespace VW::LEARNER;

// namespace logger = VW::io::logger;

namespace VW
{
namespace reductions
{
namespace epsilon_decay
{
void epsilon_decay_score::update_bounds(float w, float r)
{
  update(w, r);

  // update the lower bound
  distributionally_robust::ScoredDual sd = this->chisq.recompute_duals();
  _lower_bound = static_cast<float>(sd.first);
}

template <class ForwardIt>
ForwardIt swap_models(ForwardIt first, ForwardIt n_first, ForwardIt end)
{
  for (; first != end; ++first, ++n_first) { std::iter_swap(first, n_first); }
  return n_first;
}

template <class ForwardIt>
void reset_models(ForwardIt first, ForwardIt end, parameters& _weights, double _epsilon_decay_alpha,
    double _epsilon_decay_tau, uint64_t model_count)
{
  uint64_t params_per_weight = 1;
  while (params_per_weight < model_count) { params_per_weight *= 2; }
  for (; first != end; ++first)
  {
    first->reset_stats(_epsilon_decay_alpha, _epsilon_decay_tau);
    _weights.dense_weights.clear_offset(first->get_model_idx(), params_per_weight);
  }
}

float decayed_epsilon(uint64_t update_count) { return static_cast<float>(std::pow(update_count + 1, -1.f / 3.f)); }

}  // namespace epsilon_decay
}  // namespace reductions

namespace model_utils
{
size_t read_model_field(io_buf& io, VW::reductions::epsilon_decay::epsilon_decay_score& score)
{
  size_t bytes = 0;
  bytes += read_model_field(io, reinterpret_cast<VW::scored_config&>(score));
  bytes += read_model_field(io, score._lower_bound);
  bytes += read_model_field(io, score._model_idx);
  return bytes;
}

size_t write_model_field(io_buf& io, const VW::reductions::epsilon_decay::epsilon_decay_score& score,
    const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  bytes += write_model_field(io, reinterpret_cast<const VW::scored_config&>(score), upstream_name, text);
  bytes += write_model_field(io, score._lower_bound, upstream_name + "_lower_bound", text);
  bytes += write_model_field(io, score._model_idx, upstream_name + "_model_idx", text);
  return bytes;
}

size_t read_model_field(io_buf& io, VW::reductions::epsilon_decay::epsilon_decay_data& epsilon_decay)
{
  size_t bytes = 0;
  epsilon_decay._scored_configs.clear();
  bytes += read_model_field(io, epsilon_decay._scored_configs);
  return bytes;
}

size_t write_model_field(io_buf& io, const VW::reductions::epsilon_decay::epsilon_decay_data& epsilon_decay,
    const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  bytes += write_model_field(io, epsilon_decay._scored_configs, upstream_name + "_scored_configs", text);
  return bytes;
}
}  // namespace model_utils
}  // namespace VW

void predict(
    VW::reductions::epsilon_decay::epsilon_decay_data& data, VW::LEARNER::multi_learner& base, multi_ex& examples)
{
  auto& ep_fts = examples[0]->_reduction_features.template get<VW::cb_explore_adf::greedy::reduction_features>();
  auto active_iter = data._scored_configs.end() - 1;
  ep_fts.epsilon = VW::reductions::epsilon_decay::decayed_epsilon(active_iter->update_count);
  base.predict(examples, active_iter->get_model_idx());
}

void learn(
    VW::reductions::epsilon_decay::epsilon_decay_data& data, VW::LEARNER::multi_learner& base, multi_ex& examples)
{
  CB::cb_class logged{};
  uint64_t labelled_action = 0;
  const auto it =
      std::find_if(examples.begin(), examples.end(), [](example* item) { return !item->l.cb.costs.empty(); });
  if (it != examples.end())
  {
    logged = (*it)->l.cb.costs[0];
    labelled_action = std::distance(examples.begin(), it);
  }

  const float r = -logged.cost;
  // Process each model, then update the upper/lower bounds for each model
  for (auto config_iter = data._scored_configs.begin(); config_iter != data._scored_configs.end(); ++config_iter)
  {
    // Update the scoring of all configs
    // Only call if learn calls predict is set
    auto& ep_fts = examples[0]->_reduction_features.template get<VW::cb_explore_adf::greedy::reduction_features>();
    ep_fts.epsilon = VW::reductions::epsilon_decay::decayed_epsilon(config_iter->update_count);
    if (!base.learn_returns_prediction) { base.predict(examples, config_iter->get_model_idx()); }
    base.learn(examples, config_iter->get_model_idx());
    for (const auto& a_s : examples[0]->pred.a_s)
    {
      if (a_s.action == labelled_action)
      {
        const float w = (logged.probability > 0) ? a_s.score / logged.probability : 0;
        config_iter->update_bounds(w, r);
        break;
      }
    }
  }

  // If the lower bound of a model exceeds the upperbound of the champion, migrate the new model as
  // the new champion.
  auto champion_iter = data._scored_configs.rbegin();
  auto end_iter = data._scored_configs.rend();
  uint64_t model_count = data._scored_configs.size();
  for (auto candidate_iter = champion_iter + 1; candidate_iter != end_iter; ++candidate_iter)
  {
    if (candidate_iter->get_lower_bound() > champion_iter->get_upper_bound())
    {
      auto n_iter = swap_models(candidate_iter, champion_iter, end_iter);
      reset_models(n_iter, end_iter, data._weights, data._epsilon_decay_alpha, data._epsilon_decay_tau, model_count);
      break;
    }
  }

  // check if any model counts are higher than the champion. If so, shift the model
  // back to the beginning of the list and reset its counts
  auto model_idx = model_count - 1;
  for (auto candidate_iter = champion_iter + 1; candidate_iter != end_iter; ++candidate_iter, --model_idx)
  {
    if (candidate_iter->update_count > data._min_scope &&
        candidate_iter->update_count >
            (std::pow(champion_iter->update_count, static_cast<float>(model_idx) / model_count)))
    {
      auto n_iter = swap_models(candidate_iter + 1, candidate_iter, end_iter);
      reset_models(n_iter, end_iter, data._weights, data._epsilon_decay_alpha, data._epsilon_decay_tau, model_count);
      break;
    }
  }
}

void save_load_epsilon_decay(
    VW::reductions::epsilon_decay::epsilon_decay_data& epsilon_decay, io_buf& io, bool read, bool text)
{
  if (io.num_files() == 0) { return; }
  if (read) { VW::model_utils::read_model_field(io, epsilon_decay); }
  else
  {
    VW::model_utils::write_model_field(io, epsilon_decay, "_epsilon_decay", text);
  }
}

VW::LEARNER::base_learner* VW::reductions::epsilon_decay_setup(VW::setup_base_i& stack_builder)
{
  VW::config::options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();

  std::string arg;
  bool epsilon_decay_option;
  uint64_t model_count;
  uint64_t _min_scope;
  float _epsilon_decay_alpha;
  float _epsilon_decay_tau;

  option_group_definition new_options("Epsilon-Decaying Exploration");
  new_options
      .add(make_option("epsilon_decay", epsilon_decay_option)
               .necessary()
               .keep()
               .help("Use decay of exploration reduction"))
      .add(make_option("model_count", model_count).keep().default_value(3).help("Set number of exploration models"))
      .add(make_option("min_scope", _min_scope)
               .keep()
               .default_value(100)
               .help("Minimum example count of model before removing"))
      .add(make_option("epsilon_decay_alpha", _epsilon_decay_alpha)
               .keep()
               .default_value(DEFAULT_ALPHA)
               .help("Set confidence interval for champion change"))
      .add(make_option("epsilon_decay_tau", _epsilon_decay_tau)
               .keep()
               .default_value(DEFAULT_TAU)
               .help("Time constant for count decay"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  // Update model count to be 2^n
  uint64_t params_per_weight = 1;
  while (params_per_weight < model_count) { params_per_weight *= 2; }

  // Scale confidence interval by number of examples
  float scaled_alpha = _epsilon_decay_alpha / model_count;

  auto data = VW::make_unique<VW::reductions::epsilon_decay::epsilon_decay_data>(
      model_count, _min_scope, scaled_alpha, _epsilon_decay_tau, all.weights);

  // make sure we setup the rest of the stack with cleared interactions
  // to make sure there are not subtle bugs
  auto* base_learner = stack_builder.setup_base_learner();
  if (base_learner->is_multiline())
  {
    auto* learner = VW::LEARNER::make_reduction_learner(std::move(data), VW::LEARNER::as_multiline(base_learner), learn,
        predict, stack_builder.get_setupfn_name(epsilon_decay_setup))
                        .set_input_label_type(VW::label_type_t::cb)
                        .set_output_label_type(VW::label_type_t::cb)
                        .set_input_prediction_type(VW::prediction_type_t::action_scores)
                        .set_output_prediction_type(VW::prediction_type_t::action_scores)
                        .set_params_per_weight(params_per_weight)
                        .set_output_prediction_type(base_learner->get_output_prediction_type())
                        .set_save_load(save_load_epsilon_decay)
                        .build();

    return VW::LEARNER::make_base(*learner);
  }
  else
  {
    // not implemented
    THROW("--epsilon_decay is not supported for single line learners");
  }
}
