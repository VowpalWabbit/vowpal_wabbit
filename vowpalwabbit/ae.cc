// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "ae.h"
#include "global_data.h"
#include "learner.h"
#include "memory.h"
#include "options.h"
#include "vw.h"
#include "label_type.h"
#include "prediction_type.h"
#include "scored_config.h"

#include <string>
#include <algorithm>
#include <cmath>

using namespace VW::config;
using namespace VW::LEARNER;

// namespace logger = VW::io::logger;

namespace VW
{
namespace ae
{

struct ae_score : scored_config
{
  ae_score() : _lower_bound(0), _model_idx(0) {}
  ae_score(uint64_t model_idx) : _lower_bound(0), _model_idx(model_idx) {}
  float get_upper_bound() const { return this->current_ips(); }
  float get_lower_bound() const { return _lower_bound;  }
  uint64_t get_model_idx() const { return _model_idx; } 
  void update_bounds(float w, float r);

  float _lower_bound;
  uint64_t _model_idx;
};

struct ae_data
{
  ae_data(float _epsilon, uint64_t num_configs) : scored_configs(num_configs)
  {
    initial_epsilon = _epsilon;
    for (uint64_t i = 0; i < num_configs; ++i) {
      scored_configs[i]._model_idx = i;
    }
  }
  float initial_epsilon;
  VW::workspace* all = nullptr;  // statistics, loss
  std::vector<uint64_t> update_counts;
  std::vector<ae_score> scored_configs;
};

void ae_score::update_bounds(float w, float r)
{
  update(w, r);

  // update the lower bound
  distributionally_robust::ScoredDual sd = this->chisq.recompute_duals();
  _lower_bound = static_cast<float>(sd.first);
  ++update_count;
}

template <class ForwardIt>
ForwardIt swap_models(ForwardIt first, ForwardIt n_first, ForwardIt end)
{
  for (; first != end; ++first, ++n_first) { std::iter_swap(first, n_first); }
  return n_first;
}

template <class ForwardIt>
void reset_models(ForwardIt first, ForwardIt end, VW::workspace* workspace, uint64_t model_count)
{
  for (; first != end; ++first) {
    first->reset_stats();
    workspace->weights.dense_weights.clear_offset(first->get_model_idx(), model_count);
  }
}

float decayed_epsilon(float initial_epsilon, uint64_t update_count)
{
  // Return some function of initial epsilon and update count (cube root?)
  _UNUSED(update_count);
  return initial_epsilon;
}

void predict(ae_data& data, VW::LEARNER::multi_learner& base, multi_ex& examples)
{
  auto& ep_fts = examples[0]->_reduction_features.template get<VW::cb_explore_adf::greedy::reduction_features>();
  auto active_iter = data.scored_configs.end() - 1;
  ep_fts.epsilon = decayed_epsilon(data.initial_epsilon, (*active_iter).update_count);
  base.predict(examples, active_iter->get_model_idx());
}

void learn(ae_data& data, VW::LEARNER::multi_learner& base, multi_ex& ec)
{
  CB::cb_class logged{};
  uint64_t labelled_action = 0;
  const auto it = std::find_if(ec.begin(), ec.end(), [](example* item) { return !item->l.cb.costs.empty(); });
  if (it != ec.end())
  {
    logged = (*it)->l.cb.costs[0];
    labelled_action = std::distance(ec.begin(), it);
  }

  // Process each model, then update the upper/lower bounds for each model
  for (auto config_iter = data.scored_configs.begin(); config_iter != data.scored_configs.end(); ++config_iter)
  {
      // Update the scoring of all configs
      // Only call if learn calls predict is set
      if (!base.learn_returns_prediction) { base.predict(ec, config_iter->get_model_idx()); }
      base.learn(ec, config_iter->get_model_idx());

      const uint32_t chosen_action = ec[0]->pred.a_s[0].action;
      const float w = logged.probability > 0 ? 1 / logged.probability : 0;
      const float r = -logged.cost;
      config_iter->update_bounds((chosen_action == labelled_action) ? 0 : w, r);
  }

  // If the lower bound of a model exceeds the upperbound of the champion, migrate the new model as
  // the new champion.
  auto champion_iter = data.scored_configs.rbegin();
  auto end_iter = data.scored_configs.rend();
  uint64_t model_count = data.scored_configs.size();
  for (auto candidate_iter = champion_iter + 1; candidate_iter != end_iter; ++candidate_iter)
  {
    if (candidate_iter->get_lower_bound() > champion_iter->get_upper_bound())
    {
      auto n_iter = swap_models(candidate_iter, champion_iter, end_iter);
      reset_models(n_iter, end_iter, data.all, model_count);
      break;
    }
  }

  // check if any model counts are higher than the champion. If so, shift the model
  // back to the beginning of the list and reset its counts
  auto model_idx = model_count - 1;
  for (auto candidate_iter = champion_iter + 1; candidate_iter != end_iter; ++candidate_iter, --model_idx){
    if (candidate_iter->update_count > (pow(champion_iter->update_count, model_idx / model_count))) {
      auto n_iter = swap_models(candidate_iter+1, candidate_iter, end_iter);
      reset_models(n_iter, end_iter, data.all, model_count);
      break;
    }
  }
}


void finish_example(VW::workspace& all, ae_data& data, multi_ex& ec)
{
  VW::finish_example(all, ec);
  _UNUSED(data);
}


VW::LEARNER::base_learner* ae_setup(VW::setup_base_i& stack_builder)
{
  VW::config::options_i& options = *stack_builder.get_options();
  std::string arg;
  uint64_t model_count;
  float epsilon;

  option_group_definition new_options("Aged Exploration");
  new_options
    .add(make_option("agedexp", model_count)
      .necessary()
      .keep()
      .default_value(3)
      .help("Set number of exploration models"))
    .add(make_option("epsilon", epsilon)
      .keep()
      .allow_override()
      .default_value(0.05f)
      .help("Epsilon-greedy exploration initial value"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }
  auto data = VW::make_unique<ae_data>(epsilon, model_count);
  data->all = stack_builder.get_all_pointer();

  // make sure we setup the rest of the stack with cleared interactions
  // to make sure there are not subtle bugs
  auto* base_learner = stack_builder.setup_base_learner();
  if (base_learner->is_multiline())
  {
    auto* learner = VW::LEARNER::make_reduction_learner(std::move(data), VW::LEARNER::as_multiline(base_learner), learn,
        predict, stack_builder.get_setupfn_name(ae_setup))
                        .set_params_per_weight(model_count)
                        .set_output_prediction_type(base_learner->get_output_prediction_type())
                        .set_finish_example(finish_example)
                        .build();

    return VW::LEARNER::make_base(*learner);
  }
  else
  {
    // not implemented yet
    THROW("fatal: aged learner not supported for single line learners yet");
  }

}

}  // namespace ae
}  // namespace VW
