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

namespace VW
{
namespace reductions
{
namespace epsilon_decay
{
float decayed_epsilon(uint64_t update_count) { return static_cast<float>(std::pow(update_count + 1, -1.f / 3.f)); }

epsilon_decay_data::epsilon_decay_data(uint64_t model_count, uint64_t min_scope,
    double epsilon_decay_significance_level, double epsilon_decay_estimator_decay, dense_parameters& weights,
    std::string epsilon_decay_audit_str, bool constant_epsilon, uint32_t& wpp, bool lb_trick,
    uint64_t min_champ_examples)
    : _min_scope(min_scope)
    , _epsilon_decay_significance_level(epsilon_decay_significance_level)
    , _epsilon_decay_estimator_decay(epsilon_decay_estimator_decay)
    , _weights(weights)
    , _epsilon_decay_audit_str(std::move(epsilon_decay_audit_str))
    , _constant_epsilon(constant_epsilon)
    , _wpp(wpp)
    , _lb_trick(lb_trick)
    , _min_champ_examples(min_champ_examples)
{
  _weight_indices.resize(model_count);
  _estimator_configs.reserve(model_count);
  std::iota(_weight_indices.begin(), _weight_indices.end(), 0);
  for (uint64_t i = 0; i < model_count; ++i)
  {
    _estimator_configs.emplace_back();
    _estimator_configs.back().reserve(i + 1);
    for (uint64_t j = 0; j < i + 1; ++j)
    { _estimator_configs.back().emplace_back(epsilon_decay_significance_level, epsilon_decay_estimator_decay); }
  }
}

void epsilon_decay_data::update_weights(VW::LEARNER::multi_learner& base, VW::multi_ex& examples)
{
  auto model_count = static_cast<int64_t>(_estimator_configs.size());
  CB::cb_class logged{};
  uint64_t labelled_action = 0;
  const auto it =
      std::find_if(examples.begin(), examples.end(), [](VW::example* item) { return !item->l.cb.costs.empty(); });
  if (it != examples.end())
  {
    logged = (*it)->l.cb.costs[0];
    labelled_action = std::distance(examples.begin(), it);
    const float r = -logged.cost;
    if (_epsilon_decay_audit_str != "")
    {
      _audit_msg << "Example: " << _global_counter << " Labelled_action: " << labelled_action
                 << " p_log: " << logged.probability << " reward: " << r << "\n";
      ++_global_counter;
    }
    auto& ep_fts = examples[0]->_reduction_features.template get<VW::cb_explore_adf::greedy::reduction_features>();
    // Process each model, then update the upper/lower bounds for each model
    for (int64_t i = 0; i < model_count; ++i)
    {
      if (!_constant_epsilon)
      { ep_fts.epsilon = VW::reductions::epsilon_decay::decayed_epsilon(_estimator_configs[i][i].update_count); }
      if (!base.learn_returns_prediction) { base.predict(examples, _weight_indices[i]); }
      base.learn(examples, _weight_indices[i]);
      for (const auto& a_s : examples[0]->pred.a_s)
      {
        if (a_s.action == labelled_action)
        {
          const float w = (logged.probability > 0) ? a_s.score / logged.probability : 0;
          for (int64_t j = 0; j <= i; ++j)
          {
            if (_lb_trick && i == model_count - 1) { _estimator_configs[i][j].update(w, 1 - r); }
            else
            {
              _estimator_configs[i][j].update(w, r);
            }
          }
          if (_epsilon_decay_audit_str != "")
          {
            if (i == model_count - 1) { _audit_msg << "champ "; }
            else
            {
              _audit_msg << "challenger[" << (i + 1) << "] ";
            }
            _audit_msg << "update_count: " << _estimator_configs[i][i].update_count
                       << " lb: " << _estimator_configs[i][i].lower_bound()
                       << " ub: " << _estimator_configs[i][i].upper_bound() << " p_pred: " << a_s.score << "\n";
          }
          break;
        }
      }
    }
  }
}

// Promote model and all those lower with distance swap_dist
void epsilon_decay_data::promote_model(int64_t model_ind, int64_t swap_dist)
{
  for (; model_ind >= 0; --model_ind)
  {
    for (int64_t estimator_ind = 0; estimator_ind < model_ind + 1; ++estimator_ind)
    {
      _estimator_configs[model_ind + swap_dist][estimator_ind + swap_dist] =
          std::move(_estimator_configs[model_ind][estimator_ind]);
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
      _estimator_configs[curr_mod][estimator_ind] = std::move(_estimator_configs[curr_mod][estimator_ind - swap_dist]);
    }
  }
}

// Clear values in removed weights and estimators
void epsilon_decay_data::clear_weights_and_estimators(int64_t swap_dist, int64_t model_count)
{
  for (int64_t model_ind = 0; model_ind < model_count; ++model_ind)
  {
    for (int64_t estimator_ind = 0;
         estimator_ind < std::min(static_cast<int64_t>(_estimator_configs[model_ind].size()), swap_dist);
         ++estimator_ind)
    {
      _estimator_configs[model_ind][estimator_ind].reset_stats(
          _epsilon_decay_significance_level, _epsilon_decay_estimator_decay);
    }
  }
  for (int64_t ind = 0; ind < swap_dist; ++ind) { _weights.clear_offset(_weight_indices[ind], _wpp); }
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
  auto model_count = static_cast<int64_t>(_estimator_configs.size());
  auto final_model_idx = model_count - 1;
  for (int64_t i = 0; i < final_model_idx; ++i)
  {
    bool better = _lb_trick
        ? _estimator_configs[i][i].lower_bound() > (1.f - _estimator_configs[final_model_idx][i].lower_bound())
        : _estimator_configs[i][i].lower_bound() > _estimator_configs[final_model_idx][i].upper_bound();
    if (better && _estimator_configs[i][i].update_count >= _min_champ_examples)
    {
      if (_epsilon_decay_audit_str != "") { _audit_msg << "CHALLENGER[" << (i + 1) << "] promoted to CHAMP\n"; }
      shift_model(i, final_model_idx - i, model_count);
      if (_lb_trick)
      {
        for (int64_t i = 0; i < model_count; ++i)
        {
          for (int64_t j = 0; j <= i; ++j) { _estimator_configs[i][j].reset_stats(); }
        }
      }
      break;
    }
  }
}

void epsilon_decay_data::check_horizon_bounds()
{
  // Check if any model counts are higher than the champion. If so, shift the model
  // back to the beginning of the list and reset its counts
  auto model_count = static_cast<int64_t>(_estimator_configs.size());
  auto final_model_idx = model_count - 1;
  for (int64_t i = 0; i < final_model_idx; ++i)
  {
    if (_estimator_configs[i][i].update_count > _min_scope &&
        _estimator_configs[i][i].update_count >
            std::pow(_estimator_configs[final_model_idx][final_model_idx].update_count,
                static_cast<float>(i + 1) / model_count))
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
size_t read_model_field(io_buf& io, VW::reductions::epsilon_decay::epsilon_decay_estimator& estimator)
{
  size_t bytes = 0;
  bytes += read_model_field(io, reinterpret_cast<VW::estimator_config&>(estimator));
  return bytes;
}

size_t write_model_field(io_buf& io, const VW::reductions::epsilon_decay::epsilon_decay_estimator& estimator,
    const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  bytes += write_model_field(io, reinterpret_cast<const VW::estimator_config&>(estimator), upstream_name, text);
  return bytes;
}

size_t read_model_field(io_buf& io, VW::reductions::epsilon_decay::epsilon_decay_data& epsilon_decay)
{
  size_t bytes = 0;
  epsilon_decay._estimator_configs.clear();
  bytes += read_model_field(io, epsilon_decay._estimator_configs);
  bytes += read_model_field(io, epsilon_decay._global_counter);
  return bytes;
}

size_t write_model_field(io_buf& io, const VW::reductions::epsilon_decay::epsilon_decay_data& epsilon_decay,
    const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  bytes += write_model_field(io, epsilon_decay._estimator_configs, upstream_name + "_estimator_configs", text);
  bytes += write_model_field(io, epsilon_decay._global_counter, upstream_name + "_global_counter", text);
  return bytes;
}
}  // namespace model_utils
}  // namespace VW

namespace
{
void predict(
    VW::reductions::epsilon_decay::epsilon_decay_data& data, VW::LEARNER::multi_learner& base, VW::multi_ex& examples)
{
  uint64_t final_model_idx = static_cast<uint64_t>(data._estimator_configs.size()) - 1;
  if (!data._constant_epsilon)
  {
    auto& ep_fts = examples[0]->_reduction_features.template get<VW::cb_explore_adf::greedy::reduction_features>();
    const auto& active_estimator = data._estimator_configs[final_model_idx][final_model_idx];
    ep_fts.epsilon = VW::reductions::epsilon_decay::decayed_epsilon(active_estimator.update_count);
  }
  base.predict(examples, data._weight_indices[final_model_idx]);
}

void learn(
    VW::reductions::epsilon_decay::epsilon_decay_data& data, VW::LEARNER::multi_learner& base, VW::multi_ex& examples)
{
  data.update_weights(base, examples);
  data.check_estimator_bounds();
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

void finish(VW::reductions::epsilon_decay::epsilon_decay_data& data)
{
  if (data._epsilon_decay_audit_str != "")
  {
    io_buf buf;
    buf.add_file(VW::io::open_file_writer(data._epsilon_decay_audit_str));
    bin_text_write(buf, nullptr, 0, data._audit_msg, true);
    buf.flush();
    buf.close_file();
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
  float _epsilon_decay_significance_level;
  float _epsilon_decay_estimator_decay;
  std::string _epsilon_decay_audit_str;
  bool _constant_epsilon = false;
  bool _lb_trick = false;
  bool _fixed_significance_level = false;
  uint64_t _min_champ_examples;

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
      .add(make_option("epsilon_decay_significance_level", _epsilon_decay_significance_level)
               .keep()
               .default_value(DEFAULT_ALPHA)
               .help("Set significance level for champion change")
               .experimental())
      .add(make_option("epsilon_decay_estimator_decay", _epsilon_decay_estimator_decay)
               .keep()
               .default_value(CRESSEREAD_DEFAULT_TAU)
               .help("Time constant for count decay")
               .experimental())
      .add(make_option("epsilon_decay_audit", _epsilon_decay_audit_str)
               .default_value("")
               .help("Epsilon decay audit file name")
               .experimental())
      .add(make_option("constant_epsilon", _constant_epsilon)
               .keep()
               .help("Keep epsilon constant across models")
               .experimental())
      .add(make_option("lb_trick", _lb_trick)
               .default_value(false)
               .help("Use 1-lower_bound as upper_bound for estimator")
               .experimental())
      .add(make_option("fixed_significance_level", _fixed_significance_level)
               .keep()
               .help("Use fixed significance level as opposed to scaling by model count (bonferroni correction)")
               .experimental())
      .add(make_option("min_champ_examples", _min_champ_examples)
               .default_value(0)
               .keep()
               .help("Minimum number of examples for any challenger to become champion")
               .experimental());

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  if (model_count < 1) { THROW("Model count must be 1 or greater"); }

  if (!_fixed_significance_level) { _epsilon_decay_significance_level /= model_count; }

  auto data = VW::make_unique<VW::reductions::epsilon_decay::epsilon_decay_data>(model_count, _min_scope,
      _epsilon_decay_significance_level, _epsilon_decay_estimator_decay, all.weights.dense_weights,
      _epsilon_decay_audit_str, _constant_epsilon, all.wpp, _lb_trick, _min_champ_examples);

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
                        .set_params_per_weight(model_count)
                        .set_output_prediction_type(base_learner->get_output_prediction_type())
                        .set_save_load(save_load_epsilon_decay)
                        .set_finish(::finish)
                        .build();

    return VW::LEARNER::make_base(*learner);
  }
  else
  {
    // not implemented
    THROW("--epsilon_decay is not supported for single line learners");
  }
}
