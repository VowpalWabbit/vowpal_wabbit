// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/cb/cb_actions_mask.h"

#include "vw/config/options.h"
#include "vw/core/action_score.h"
#include "vw/core/global_data.h"
#include "vw/core/learner.h"
#include "vw/core/reductions/cb/cb_adf.h"
#include "vw/core/setup_base.h"
#include "vw/core/vw.h"

void VW::reductions::cb_actions_mask::update_predictions(multi_ex& examples, size_t initial_action_size)
{
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
  auto initial_action_size = examples.size();
  if (is_learn)
  {
    base.learn(examples);

    VW::example* label_example = CB_ADF::test_adf_sequence(examples);

    if (base.learn_returns_prediction || label_example == nullptr)
    {
      data.update_predictions(examples, initial_action_size);
    }
  }
  else
  {
    base.predict(examples);
    data.update_predictions(examples, initial_action_size);
  }
}

struct options_cb_actions_mask_v1
{
  bool large_action_space_supplied;
};

std::unique_ptr<options_cb_actions_mask_v1> get_cb_actions_mask_options_instance(
    const VW::workspace&, VW::io::logger&, VW::config::options_i& options)
{
  auto cb_actions_mask_opts = VW::make_unique<options_cb_actions_mask_v1>();
  cb_actions_mask_opts->large_action_space_supplied = options.was_supplied("large_action_space");
  if (!cb_actions_mask_opts->large_action_space_supplied) { return nullptr; }
  return cb_actions_mask_opts;
}

VW::LEARNER::base_learner* VW::reductions::cb_actions_mask_setup(VW::setup_base_i& stack_builder)
{
  VW::workspace& all = *stack_builder.get_all_pointer();
  auto cb_actions_mask_opts = get_cb_actions_mask_options_instance(all, all.logger, *stack_builder.get_options());
  if (cb_actions_mask_opts == nullptr) { return nullptr; }
  auto cb_actions_mask_data = VW::make_unique<VW::reductions::cb_actions_mask>();

  auto* base = as_multiline(stack_builder.setup_base_learner());

  auto* l = VW::LEARNER::make_reduction_learner(std::move(cb_actions_mask_data), base, learn_or_predict<true>, learn_or_predict<false>,
      stack_builder.get_setupfn_name(cb_actions_mask_setup))
                .set_input_label_type(VW::label_type_t::CB)
                .set_output_label_type(VW::label_type_t::CB)
                .set_input_prediction_type(VW::prediction_type_t::ACTION_SCORES)
                .set_output_prediction_type(VW::prediction_type_t::ACTION_PROBS)
                .set_learn_returns_prediction(base->learn_returns_prediction)
                .build();
  return VW::LEARNER::make_base(*l);
}
