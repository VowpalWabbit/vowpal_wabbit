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
#include "vw/core/multi_model_utils.h"
#include "vw/core/prediction_type.h"
#include "vw/core/setup_base.h"
#include "vw/core/vw.h"

#include <algorithm>
#include <utility>

using namespace VW::config;
using namespace VW::LEARNER;

namespace VW
{
namespace reductions
{
namespace epsilon_decay
{
float decayed_epsilon(float init_ep, uint64_t update_count)
{
  return init_ep * static_cast<float>(std::pow(update_count + 1, -1.f / 3.f));
}

epsilon_decay_data::epsilon_decay_data(uint64_t model_count, uint64_t min_scope,
    double epsilon_decay_significance_level, dense_parameters& weights, std::string epsilon_decay_audit_str,
    bool constant_epsilon, uint32_t& feature_width, uint64_t min_champ_examples, float initial_epsilon,
    uint64_t shift_model_bounds, bool reward_as_cost, double tol_x, bool is_brentq, bool predict_only_model,
    bool challenger_epsilon, VW::io::logger logger)
    : _model_count(model_count)
    , _min_scope(min_scope)
    , _epsilon_decay_significance_level(epsilon_decay_significance_level)
    , _weights(weights)
    , _epsilon_decay_audit_str(std::move(epsilon_decay_audit_str))
    , _constant_epsilon(constant_epsilon)
    , _feature_width(feature_width)
    , _min_champ_examples(min_champ_examples)
    , _initial_epsilon(initial_epsilon)
    , _shift_model_bounds(shift_model_bounds)
    , _reward_as_cost(reward_as_cost)
    , _predict_only_model(predict_only_model)
    , _challenger_epsilon(challenger_epsilon)
    , _logger(std::move(logger))
{
  _weight_indices.resize(model_count);
  conf_seq_estimators.reserve(model_count);
  std::iota(_weight_indices.begin(), _weight_indices.end(), uint64_t{0});
  for (uint64_t i = 0; i < model_count; ++i)
  {
    conf_seq_estimators.emplace_back();
    conf_seq_estimators.back().reserve(i + 1);
    for (uint64_t j = 0; j < i + 1; ++j)
    {
      conf_seq_estimators.back().emplace_back(tol_x, is_brentq, epsilon_decay_significance_level);
    }
  }
}

void epsilon_decay_data::update_weights(float init_ep, VW::LEARNER::learner& base, VW::multi_ex& examples)
{
  auto model_count = static_cast<int64_t>(conf_seq_estimators.size());
  VW::cb_class logged{};
  uint64_t labelled_action = 0;
  const auto it =
      std::find_if(examples.begin(), examples.end(), [](VW::example* item) { return !item->l.cb.costs.empty(); });
  if (it != examples.end())
  {
    logged = (*it)->l.cb.costs[0];
    labelled_action = std::distance(examples.begin(), it);
    float r = _reward_as_cost ? logged.cost : -logged.cost;
    if (r < 0.f || r > 1.f)
    {
      _logger.err_warn(
          "epsilon_decay: reward {} is outside [0,1] and will be clamped. "
          "Ensure costs are in [0,1] when using epsilon_decay.",
          r);
      r = std::max(0.f, std::min(r, 1.f));
    }
    if (_epsilon_decay_audit_str != "")
    {
      _audit_msg << "Example: " << _global_counter << " Labelled_action: " << labelled_action
                 << " p_log: " << logged.probability << " reward: " << r << "\n";
      ++_global_counter;
    }
    auto& ep_fts = examples[0]->ex_reduction_features.template get<VW::cb_explore_adf::greedy::reduction_features>();

    VW::action_scores champ_a_s;

    // Process each model, then update the upper/lower bounds for each model
    for (int64_t model_ind = model_count - 1; model_ind >= 0; --model_ind)
    {
      if (!_constant_epsilon)
      {
        ep_fts.epsilon = VW::reductions::epsilon_decay::decayed_epsilon(
            init_ep, conf_seq_estimators[model_ind][model_ind].update_count);
      }
      if (!base.learn_returns_prediction) { base.predict(examples, _weight_indices[model_ind]); }
      base.learn(examples, _weight_indices[model_ind]);

      for (const auto& a_s : examples[0]->pred.a_s)
      {
        if (a_s.action == labelled_action)
        {
          float p_pred = a_s.score;
          if (model_ind != model_count - 1 && !_challenger_epsilon)
          {
            // Get best action
            uint32_t best_action = 0;
            float best_score = 0.f;
            for (const auto& a_s : examples[0]->pred.a_s)
            {
              if (a_s.score > best_score)
              {
                best_action = a_s.action;
                best_score = a_s.score;
              }
            }
            p_pred = (a_s.action == best_action) ? 1.f : 0.f;
          }
          float w = (logged.probability > 0) ? p_pred / logged.probability : 0;
          for (int64_t estimator_ind = 0; estimator_ind <= model_ind; ++estimator_ind)
          {
            conf_seq_estimators[model_ind][estimator_ind].update(w, r);
          }
          if (_epsilon_decay_audit_str != "")
          {
            if (model_ind != model_count - 1) { _audit_msg << "challenger[" << (model_ind + 1) << "] "; }
            else { _audit_msg << "champ "; }
            _audit_msg << "update_count: " << conf_seq_estimators[model_ind][model_ind].update_count
                       << " lb: " << conf_seq_estimators[model_ind][model_ind].lower_bound()
                       << " champ_ub: " << conf_seq_estimators[model_count - 1][model_ind].upper_bound()
                       << " p_pred: " << p_pred << "\n";
          }
          break;
        }
      }
      if (model_ind == model_count - 1) { champ_a_s = examples[0]->pred.a_s; }
    }
    examples[0]->pred.a_s = champ_a_s;
  }
}

// Promote model and all those lower with distance swap_dist
void epsilon_decay_data::promote_model(int64_t model_ind, int64_t swap_dist)
{
  for (; model_ind >= 0; --model_ind)
  {
    for (int64_t estimator_ind = 0; estimator_ind < model_ind + 1; ++estimator_ind)
    {
      conf_seq_estimators[model_ind + swap_dist][estimator_ind + swap_dist] =
          std::move(conf_seq_estimators[model_ind][estimator_ind]);
    }
    std::swap(_weight_indices[model_ind + swap_dist], _weight_indices[model_ind]);
  }
}

// Rebalance greater models to match lower shifted models
void epsilon_decay_data::rebalance_greater_models(int64_t model_ind, int64_t swap_dist, int64_t model_count)
{
  int64_t greater_model = model_ind + swap_dist + 1;
  for (int64_t curr_mod = greater_model; curr_mod < model_count; ++curr_mod)
  {
    for (int64_t estimator_ind = model_ind + 1; estimator_ind >= swap_dist; --estimator_ind)
    {
      conf_seq_estimators[curr_mod][estimator_ind] =
          std::move(conf_seq_estimators[curr_mod][estimator_ind - swap_dist]);
    }
  }
}

// Clear values in removed weights and estimators
void epsilon_decay_data::clear_weights_and_estimators(int64_t swap_dist, int64_t model_count)
{
  for (int64_t model_ind = 0; model_ind < model_count; ++model_ind)
  {
    for (int64_t estimator_ind = 0;
         estimator_ind < std::min(static_cast<int64_t>(conf_seq_estimators[model_ind].size()), swap_dist);
         ++estimator_ind)
    {
      conf_seq_estimators[model_ind][estimator_ind].reset_stats();
    }
  }
  for (int64_t ind = 0; ind < swap_dist; ++ind)
  {
    VW::reductions::multi_model::clear_innermost_offset(_weights, _weight_indices[ind], _feature_width, _model_count);
  }
}

void epsilon_decay_data::shift_model(int64_t model_ind, int64_t swap_dist, int64_t model_count)
{
  if (model_ind >= 0)
  {
    promote_model(model_ind, swap_dist);
    rebalance_greater_models(model_ind, swap_dist, model_count);
  }
  clear_weights_and_estimators(swap_dist, model_count);
}

void epsilon_decay_data::check_estimator_bounds()
{
  // If the lower bound of a model exceeds the upperbound of the champion, migrate the new model as
  // the new champion.
  auto model_count = static_cast<int64_t>(conf_seq_estimators.size());
  auto final_model_idx = model_count - 1;
  for (int64_t i = final_model_idx - 1; i >= 0; --i)
  {
    bool better = conf_seq_estimators[i][i].lower_bound() > conf_seq_estimators[final_model_idx][i].upper_bound();
    if (better && conf_seq_estimators[i][i].update_count >= _min_champ_examples)
    {
      if (_epsilon_decay_audit_str != "") { _audit_msg << "CHALLENGER[" << (i + 1) << "] promoted to CHAMP\n"; }
      shift_model(i, final_model_idx - i, model_count);
      break;
    }
  }
}

void epsilon_decay_data::check_horizon_bounds()
{
  // Check if any model counts are higher than the champion. If so, shift the model
  // back to the beginning of the list and reset its counts
  auto model_count = static_cast<int64_t>(conf_seq_estimators.size());
  auto final_model_idx = model_count - 1;
  for (int64_t i = final_model_idx - 1; i >= 0; --i)
  {
    if (conf_seq_estimators[i][i].update_count > _min_scope &&
        conf_seq_estimators[i][i].update_count >
            std::pow(conf_seq_estimators[final_model_idx][final_model_idx].update_count,
                static_cast<float>(i + 1 + _shift_model_bounds) / (model_count + _shift_model_bounds)))
    {
      shift_model(i - 1, 1, model_count);
      break;
    }
  }
}
}  // namespace epsilon_decay
}  // namespace reductions

namespace model_utils
{
size_t read_model_field(io_buf& io, VW::reductions::epsilon_decay::epsilon_decay_data& epsilon_decay)
{
  size_t bytes = 0;
  epsilon_decay.conf_seq_estimators.clear();
  epsilon_decay._weight_indices.clear();
  bytes += read_model_field(io, epsilon_decay.conf_seq_estimators);
  bytes += read_model_field(io, epsilon_decay._weight_indices);
  bytes += read_model_field(io, epsilon_decay._global_counter);
  return bytes;
}

size_t write_model_field(io_buf& io, const VW::reductions::epsilon_decay::epsilon_decay_data& epsilon_decay,
    const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  bytes += write_model_field(io, epsilon_decay.conf_seq_estimators, upstream_name + "conf_seq_estimators", text);
  bytes += write_model_field(io, epsilon_decay._weight_indices, upstream_name + "_weight_indices", text);
  bytes += write_model_field(io, epsilon_decay._global_counter, upstream_name + "_global_counter", text);
  return bytes;
}
}  // namespace model_utils
}  // namespace VW

namespace
{
void predict(
    VW::reductions::epsilon_decay::epsilon_decay_data& data, VW::LEARNER::learner& base, VW::multi_ex& examples)
{
  uint64_t final_model_idx = static_cast<uint64_t>(data.conf_seq_estimators.size()) - 1;
  if (!data._constant_epsilon)
  {
    auto& ep_fts = examples[0]->ex_reduction_features.template get<VW::cb_explore_adf::greedy::reduction_features>();
    const auto& active_estimator = data.conf_seq_estimators[final_model_idx][final_model_idx];
    ep_fts.epsilon =
        VW::reductions::epsilon_decay::decayed_epsilon(data._initial_epsilon, active_estimator.update_count);
  }
  base.predict(examples, data._weight_indices[final_model_idx]);
}

void learn(VW::reductions::epsilon_decay::epsilon_decay_data& data, VW::LEARNER::learner& base, VW::multi_ex& examples)
{
  data.update_weights(data._initial_epsilon, base, examples);
  data.check_estimator_bounds();
  data.check_horizon_bounds();
}

void save_load_epsilon_decay(
    VW::reductions::epsilon_decay::epsilon_decay_data& epsilon_decay, VW::io_buf& io, bool read, bool text)
{
  if (io.num_files() == 0) { return; }
  if (read) { VW::model_utils::read_model_field(io, epsilon_decay); }
  else if (!epsilon_decay._predict_only_model)
  {
    VW::model_utils::write_model_field(io, epsilon_decay, "_epsilon_decay", text);
  }
}

void finish(VW::reductions::epsilon_decay::epsilon_decay_data& data)
{
  if (data._epsilon_decay_audit_str != "")
  {
    VW::io_buf buf;
    buf.add_file(VW::io::open_file_writer(data._epsilon_decay_audit_str));
    VW::details::bin_text_write(buf, nullptr, 0, data._audit_msg, true);
    buf.flush();
    buf.close_file();
  }
}

void pre_save_load_epsilon_decay(VW::workspace& all, VW::reductions::epsilon_decay::epsilon_decay_data& data)
{
  options_i& options = *all.options;
  if (!data._predict_only_model) { return; }

  // Adjust champ weights to new single-model space
  VW::reductions::multi_model::reduce_innermost_model_weights(
      data._weights, data._weight_indices[data.conf_seq_estimators.size() - 1], data._feature_width, data._model_count);

  for (auto& group : options.get_all_option_group_definitions())
  {
    if (group.m_name == "[Reduction] Epsilon-Decaying Exploration Options")
    {
      for (auto& opt : group.m_options) { opt->m_keep = false; }
    }
  }

  all.initial_weights_config.num_bits =
      all.initial_weights_config.num_bits - static_cast<uint32_t>(std::log2(data._feature_width));
  options.get_typed_option<uint32_t>("bit_precision").value(all.initial_weights_config.num_bits);
}

}  // namespace

std::shared_ptr<VW::LEARNER::learner> VW::reductions::epsilon_decay_setup(VW::setup_base_i& stack_builder)
{
  VW::config::options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();

  std::string arg;
  bool epsilon_decay_option;
  uint64_t model_count;
  uint64_t min_scope;
  float epsilon_decay_significance_level;
  std::string epsilon_decay_audit_str;
  bool constant_epsilon = false;
  uint64_t bonferroni_denominator;
  uint64_t min_champ_examples;
  float initial_epsilon;
  uint64_t shift_model_bounds;
  bool reward_as_cost;
  float tol_x;
  std::string opt_func = "bisect";
  bool challenger_epsilon = false;

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
      .add(make_option("min_scope", min_scope)
               .keep()
               .default_value(100)
               .help("Minimum example count of model before removing")
               .experimental())
      .add(make_option("epsilon_decay_significance_level", epsilon_decay_significance_level)
               .keep()
               .default_value(VW::details::CS_ROBUST_DEFAULT_ALPHA)
               .help("Set significance level for champion change")
               .experimental())
      .add(make_option("epsilon_decay_audit", epsilon_decay_audit_str)
               .default_value("")
               .help("Epsilon decay audit file name")
               .experimental())
      .add(make_option("constant_epsilon", constant_epsilon)
               .keep()
               .help("Keep epsilon constant across models")
               .experimental())
      .add(make_option("bonferroni_denominator", bonferroni_denominator)
               .default_value(0)
               .keep()
               .help("Factor to scale down significance level by, defaults to model count (bonferroni correction)")
               .experimental())
      .add(make_option("min_champ_examples", min_champ_examples)
               .default_value(0)
               .keep()
               .help("Minimum number of examples for any challenger to become champion")
               .experimental())
      .add(make_option("initial_epsilon", initial_epsilon)
               .default_value(1.0)
               .keep()
               .help("Initial epsilon value")
               .experimental())
      .add(make_option("shift_model_bounds", shift_model_bounds)
               .default_value(0)
               .keep()
               .help("Shift maximum update_count for model i from champ_update_count^(i / num_models) to "
                     "champ_update_count^((i + shift) / (num_models + shift))")
               .experimental())
      .add(make_option("reward_as_cost", reward_as_cost)
               .keep()
               .help("Treat rewards as cost (do not negate sign)")
               .experimental())
      .add(make_option("tol_x", tol_x)
               .default_value(1e-6f)
               .keep()
               .help("Tolerance for estimation optimization")
               .experimental())
      .add(make_option("opt_func", opt_func)
               .default_value("bisect")
               .keep()
               .one_of({"bisect", "brentq"})
               .help("Optimization function for estimation")
               .experimental())
      .add(make_option("challenger_epsilon", challenger_epsilon)
               .keep()
               .help("Use exploration for challenger model predictions")
               .experimental());

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  if (model_count < 1) { THROW("Model count must be 1 or greater"); }

  if (bonferroni_denominator == 0) { bonferroni_denominator = model_count; }
  epsilon_decay_significance_level /= bonferroni_denominator;

  bool predict_only_model = options.was_supplied("predict_only_model");
  bool is_brentq = opt_func == "brentq";

  auto data = VW::make_unique<VW::reductions::epsilon_decay::epsilon_decay_data>(model_count, min_scope,
      epsilon_decay_significance_level, all.weights.dense_weights, epsilon_decay_audit_str, constant_epsilon,
      all.reduction_state.total_feature_width, min_champ_examples, initial_epsilon, shift_model_bounds, reward_as_cost,
      tol_x, is_brentq, predict_only_model, challenger_epsilon, all.logger);

  // make sure we setup the rest of the stack with cleared interactions
  // to make sure there are not subtle bugs
  auto base = stack_builder.setup_base_learner(model_count);

  if (base->is_multiline())
  {
    auto l = VW::LEARNER::make_reduction_learner(std::move(data), VW::LEARNER::require_multiline(base), learn, predict,
        stack_builder.get_setupfn_name(epsilon_decay_setup))
                 .set_input_label_type(VW::label_type_t::CB)
                 .set_output_label_type(VW::label_type_t::CB)
                 .set_input_prediction_type(VW::prediction_type_t::ACTION_PROBS)
                 .set_output_prediction_type(VW::prediction_type_t::ACTION_PROBS)
                 .set_feature_width(model_count)
                 .set_save_load(save_load_epsilon_decay)
                 .set_finish(::finish)
                 .set_pre_save_load(pre_save_load_epsilon_decay)
                 .build();

    return l;
  }
  else
  {
    // not implemented
    THROW("--epsilon_decay is not supported for single line learners");
  }
}
