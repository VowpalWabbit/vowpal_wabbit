// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/cb/cb_actions_mask.h"

#include "vw/config/options.h"
#include "vw/core/action_score.h"
#include "vw/core/global_data.h"
#include "vw/core/setup_base.h"
#include "vw/core/vw.h"

void VW::reductions::cb_actions_mask::learn(VW::LEARNER::multi_learner& base, multi_ex& examples)
{
  base.learn(examples);
}

void VW::reductions::cb_actions_mask::predict(VW::LEARNER::multi_learner& base, multi_ex& examples)
{
  auto initial_action_size = examples.size();
  base.predict(examples);

  auto& preds = examples[0]->pred.a_s;
  std::vector<bool> actions_present(initial_action_size);
  for (const auto& action_score : preds) { actions_present[action_score.action] = true; }

  for (uint32_t i = 0; i < actions_present.size(); i++)
  {
    if (!actions_present[i]) { preds.push_back({i, 0.0f}); }
  }
}

template <bool is_learn>
void learn_or_predict(VW::reductions::cb_actions_mask& data, VW::LEARNER::multi_learner& base, VW::multi_ex& examples)
{
  if (is_learn) { data.learn(base, examples); }
  else
  {
    data.predict(base, examples);
  }
}

VW::LEARNER::base_learner* VW::reductions::cb_actions_mask_setup(VW::setup_base_i& stack_builder)
{
  VW::config::options_i& options = *stack_builder.get_options();
  auto data = VW::make_unique<VW::reductions::cb_actions_mask>();

  if (!options.was_supplied("large_action_space") || !options.was_supplied("full_predictions")) { return nullptr; }

  auto* base = as_multiline(stack_builder.setup_base_learner());

  auto* l = VW::LEARNER::make_reduction_learner(std::move(data), base, learn_or_predict<true>, learn_or_predict<false>,
      stack_builder.get_setupfn_name(cb_actions_mask_setup))
                .set_input_label_type(VW::label_type_t::cb)
                .set_output_label_type(VW::label_type_t::cb)
                .set_input_prediction_type(VW::prediction_type_t::action_scores)
                .set_output_prediction_type(VW::prediction_type_t::action_probs)
                .build();
  return VW::LEARNER::make_base(*l);
}
