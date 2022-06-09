// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/epsilon_decay.h"

#include "vw/config/options.h"
#include "vw/core/global_data.h"
#include "vw/core/label_type.h"
#include "vw/core/learner.h"
#include "vw/core/memory.h"
#include "vw/core/model_utils.h"
#include "vw/core/prediction_type.h"
#include "vw/core/vw.h"

#include <utility>

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

void epsilon_decay_score::reset_stats(double alpha, double tau)
{
  VW::scored_config::reset_stats(alpha, tau);
  _lower_bound = 0.f;
}

float decayed_epsilon(uint64_t update_count) { return static_cast<float>(std::pow(update_count + 1, -1.f / 3.f)); }

void epsilon_decay_data::update_weights(VW::LEARNER::multi_learner& base, VW::multi_ex& examples)
{
  auto num_models = static_cast<int64_t>(_scored_configs.size());
  CB::cb_class logged{};
  uint64_t labelled_action = 0;
  const auto it =
      std::find_if(examples.begin(), examples.end(), [](VW::example* item) { return !item->l.cb.costs.empty(); });
  if (it != examples.end())
  {
    logged = (*it)->l.cb.costs[0];
    labelled_action = std::distance(examples.begin(), it);
  }

  const float r = 1 - (-logged.cost);
  auto& ep_fts = examples[0]->_reduction_features.template get<VW::cb_explore_adf::greedy::reduction_features>();
  // Process each model, then update the upper/lower bounds for each model
  for (int64_t i = 0; i < num_models; ++i)
  {
    if (!_constant_epsilon)
    { ep_fts.epsilon = VW::reductions::epsilon_decay::decayed_epsilon(_scored_configs[i][i].update_count); }
    if (!base.learn_returns_prediction) { base.predict(examples, _weight_indices[i]); }
    base.learn(examples, _weight_indices[i]);
    for (const auto& a_s : examples[0]->pred.a_s)
    {
      if (a_s.action == labelled_action)
      {
        const float w = (logged.probability > 0) ? a_s.score / logged.probability : 0;
        for (int64_t j = 0; j <= i; ++j) { _scored_configs[i][j].update_bounds(w, r); }
        break;
      }
    }
  }
}

// Promote model and all those lower with distance swap_dist
void epsilon_decay_data::promote_model(int64_t model_ind, int64_t swap_dist)
{
  for (; model_ind >= 0; --model_ind)
  {
    for (int64_t score_ind = 0; score_ind < model_ind + 1; ++score_ind)
    {
      _scored_configs[model_ind + swap_dist][score_ind + swap_dist] = std::move(_scored_configs[model_ind][score_ind]);
    }
    std::swap(_weight_indices[model_ind + swap_dist], _weight_indices[model_ind]);
  }
}

// Rebalance greater models to match lower shifted models
void epsilon_decay_data::rebalance_greater_models(int64_t model_ind, int64_t swap_dist, int64_t num_models)
{
  int64_t greater_model = model_ind + swap_dist + 1;
  for (int64_t curr_mod = greater_model; curr_mod < num_models; ++curr_mod)
  {
    for (int64_t score_ind = greater_model - swap_dist; score_ind > 0; --score_ind)
    { _scored_configs[curr_mod][score_ind] = std::move(_scored_configs[curr_mod][score_ind - swap_dist]); }
  }
}

// Clear values in removed weights and scores
void epsilon_decay_data::clear_weights_and_scores(int64_t swap_dist, int64_t num_models)
{
  uint64_t params_per_weight = 1;
  while (params_per_weight < static_cast<uint64_t>(num_models)) { params_per_weight *= 2; }
  for (int64_t model_ind = 0; model_ind < num_models; ++model_ind)
  {
    for (int64_t score_ind = 0;
         score_ind < std::min(static_cast<int64_t>(_scored_configs[model_ind].size()), swap_dist); ++score_ind)
    { _scored_configs[model_ind][score_ind].reset_stats(_epsilon_decay_alpha, _epsilon_decay_tau); }
  }
  for (int64_t ind = 0; ind < swap_dist; ++ind) { _weights.clear_offset(_weight_indices[ind], params_per_weight); }
}

void epsilon_decay_data::shift_model(int64_t model_ind, int64_t swap_dist, int64_t num_models)
{
  promote_model(model_ind, swap_dist);
  rebalance_greater_models(model_ind, swap_dist, num_models);
  clear_weights_and_scores(swap_dist, num_models);
}

void epsilon_decay_data::check_score_bounds()
{
  // If the lower bound of a model exceeds the upperbound of the champion, migrate the new model as
  // the new champion.
  auto num_models = static_cast<int64_t>(_scored_configs.size());
  for (int64_t i = 0; i < num_models - 1; ++i)
  {
    if (_scored_configs[i][i].get_lower_bound() > _scored_configs[num_models - 1][i].get_upper_bound())
    {
      if (_log_champ_changes)
      {
        _logger.out_info("Champion with update count: {} has changed to challenger with update count: {}",
            _scored_configs[num_models - 1][num_models - 1].update_count, _scored_configs[i][i].update_count);
      }
      shift_model(i, num_models - i - 1, num_models);
      break;
    }
  }
}

void epsilon_decay_data::check_horizon_bounds()
{
  // Check if any model counts are higher than the champion. If so, shift the model
  // back to the beginning of the list and reset its counts
  auto num_models = static_cast<int64_t>(_scored_configs.size());
  for (int64_t i = 0; i < num_models - 1; ++i)
  {
    if (_scored_configs[i][i].update_count > _min_scope &&
        _scored_configs[i][i].update_count > std::pow(_scored_configs[num_models - 1][num_models - 1].update_count,
                                                 static_cast<float>(i + 1) / num_models))
    {
      shift_model(i - 1, 1, num_models);
      break;
    }
  }
}
}  // namespace epsilon_decay
}  // namespace reductions

namespace model_utils
{
size_t read_model_field(io_buf& io, VW::reductions::epsilon_decay::epsilon_decay_score& score)
{
  size_t bytes = 0;
  bytes += read_model_field(io, reinterpret_cast<VW::scored_config&>(score));
  bytes += read_model_field(io, score._lower_bound);
  return bytes;
}

size_t write_model_field(io_buf& io, const VW::reductions::epsilon_decay::epsilon_decay_score& score,
    const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  bytes += write_model_field(io, reinterpret_cast<const VW::scored_config&>(score), upstream_name, text);
  bytes += write_model_field(io, score._lower_bound, upstream_name + "_lower_bound", text);
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

namespace
{
void predict(
    VW::reductions::epsilon_decay::epsilon_decay_data& data, VW::LEARNER::multi_learner& base, VW::multi_ex& examples)
{
  uint64_t num_models = static_cast<uint64_t>(data._scored_configs.size());
  if (!data._constant_epsilon)
  {
    auto& ep_fts = examples[0]->_reduction_features.template get<VW::cb_explore_adf::greedy::reduction_features>();
    const auto& active_score = data._scored_configs[num_models - 1][num_models - 1];
    ep_fts.epsilon = VW::reductions::epsilon_decay::decayed_epsilon(active_score.update_count);
  }
  base.predict(examples, data._weight_indices[num_models - 1]);
}

void learn(
    VW::reductions::epsilon_decay::epsilon_decay_data& data, VW::LEARNER::multi_learner& base, VW::multi_ex& examples)
{
  data.update_weights(base, examples);
  data.check_score_bounds();
  data.check_horizon_bounds();
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

}  // namespace

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
  bool _log_champ_changes = false;
  bool _constant_epsilon = false;

  option_group_definition new_options("[Reduction] Epsilon-Decaying Exploration");
  new_options
      .add(make_option("epsilon_decay", epsilon_decay_option)
               .necessary()
               .keep()
               .help("Use decay of exploration reduction")
               .experimental())
      .add(make_option("model_count", model_count)
               .keep()
               .default_value(3)
               .help("Set number of exploration models")
               .experimental())
      .add(make_option("min_scope", _min_scope)
               .keep()
               .default_value(100)
               .help("Minimum example count of model before removing")
               .experimental())
      .add(make_option("epsilon_decay_alpha", _epsilon_decay_alpha)
               .keep()
               .default_value(DEFAULT_ALPHA)
               .help("Set confidence interval for champion change")
               .experimental())
      .add(make_option("epsilon_decay_tau", _epsilon_decay_tau)
               .keep()
               .default_value(CRESSEREAD_DEFAULT_TAU)
               .help("Time constant for count decay")
               .experimental())
      .add(make_option("log_champ_changes", _log_champ_changes).keep().help("Log champ changes").experimental())
      .add(make_option("constant_epsilon", _constant_epsilon)
               .keep()
               .help("Keep epsilon constant across models")
               .experimental());

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  if (model_count < 1) { THROW("Model count must be 1 or greater"); }

  // Scale confidence interval by number of examples
  float scaled_alpha = (_epsilon_decay_alpha / model_count) / 2.0;

  auto data = VW::make_unique<VW::reductions::epsilon_decay::epsilon_decay_data>(model_count, _min_scope, scaled_alpha,
      _epsilon_decay_tau, all.weights.dense_weights, all.logger, _log_champ_changes, _constant_epsilon);

  uint64_t params_per_weight = 1;
  while (params_per_weight < model_count) { params_per_weight *= 2; }

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
