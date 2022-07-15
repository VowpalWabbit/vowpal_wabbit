// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/slates.h"

#include "vw/config/options.h"
#include "vw/core/action_score.h"
#include "vw/core/ccb_label.h"
#include "vw/core/decision_scores.h"
#include "vw/core/example.h"
#include "vw/core/global_data.h"
#include "vw/core/print_utils.h"
#include "vw/core/reductions/cb/cb_adf.h"
#include "vw/core/reductions/cb/cb_algs.h"
#include "vw/core/reductions/conditional_contextual_bandit.h"
#include "vw/core/setup_base.h"
#include "vw/core/shared_data.h"
#include "vw/core/slates_label.h"
#include "vw/core/vw.h"

#include <algorithm>

using namespace VW::config;

template <bool is_learn>
void VW::reductions::slates_data::learn_or_predict(VW::LEARNER::multi_learner& base, multi_ex& examples)
{
  _stashed_labels.clear();
  _stashed_labels.reserve(examples.size());
  for (auto& example : examples) { _stashed_labels.push_back(std::move(example->l.slates)); }

  const size_t num_slots = std::count_if(examples.begin(), examples.end(),
      [](const example* example) { return example->l.slates.type == VW::slates::example_type::slot; });

  float global_cost = 0.f;
  bool global_cost_found = false;
  uint32_t action_index = 0;
  size_t slot_index = 0;
  std::vector<std::vector<uint32_t>> slot_action_pools(num_slots);
  for (size_t i = 0; i < examples.size(); i++)
  {
    CCB::label ccb_label;
    CCB::default_label(ccb_label);
    const auto& slates_label = _stashed_labels[i];
    if (slates_label.type == slates::example_type::shared)
    {
      ccb_label.type = CCB::example_type::shared;
      if (slates_label.labeled)
      {
        global_cost_found = true;
        global_cost = slates_label.cost;
      }
    }
    else if (slates_label.type == slates::example_type::action)
    {
      if (slates_label.slot_id >= num_slots) { THROW("slot_id cannot be larger than or equal to the number of slots"); }
      ccb_label.type = CCB::example_type::action;
      slot_action_pools[slates_label.slot_id].push_back(action_index);
      action_index++;
    }
    else if (slates_label.type == slates::example_type::slot)
    {
      ccb_label.type = CCB::example_type::slot;
      ccb_label.explicit_included_actions.clear();
      for (const auto index : slot_action_pools[slot_index]) { ccb_label.explicit_included_actions.push_back(index); }

      if (global_cost_found)
      {
        ccb_label.outcome = new CCB::conditional_contextual_bandit_outcome();
        ccb_label.outcome->cost = global_cost;
        ccb_label.outcome->probabilities.clear();

        for (const auto& action_score : slates_label.probabilities)
        {
          // We need to convert from slate space which is zero based for
          // each slot to CCB where all action indices are in the same space.
          auto ccb_space_index = slot_action_pools[slot_index][action_score.action];
          ccb_label.outcome->probabilities.push_back({ccb_space_index, action_score.score});
        }
      }
      slot_index++;
    }

    ccb_label.weight = slates_label.weight;
    examples[i]->l.conditional_contextual_bandit = ccb_label;
  }
  VW::LEARNER::multiline_learn_or_predict<is_learn>(base, examples, examples[0]->ft_offset);

  // Need to convert decision scores to the original index space. This can be
  // done by going through the prediction for each slots and subtracting the
  // number of actions seen so far.
  uint32_t size_so_far = 0;
  for (auto& action_scores : examples[0]->pred.decision_scores)
  {
    for (auto& action_score : action_scores) { action_score.action = action_score.action - size_so_far; }
    size_so_far += static_cast<uint32_t>(action_scores.size());
  }

  for (size_t i = 0; i < examples.size(); i++) { examples[i]->l.slates = std::move(_stashed_labels[i]); }
  _stashed_labels.clear();
}

void VW::reductions::slates_data::learn(VW::LEARNER::multi_learner& base, multi_ex& examples)
{
  learn_or_predict<true>(base, examples);
}

void VW::reductions::slates_data::predict(VW::LEARNER::multi_learner& base, multi_ex& examples)
{
  learn_or_predict<false>(base, examples);
}

// TODO this abstraction may not really work as this function now doesn't have access to the global cost...
std::string VW::reductions::generate_slates_label_printout(const std::vector<example*>& slots)
{
  size_t counter = 0;
  std::stringstream label_ss;
  std::string delim;
  for (const auto& slot : slots)
  {
    counter++;
    const auto& label = slot->l.slates;
    if (label.labeled) { label_ss << delim << label.probabilities[0].action; }
    else
    {
      label_ss << delim << "?";
    }

    delim = ",";

    // Stop after 2...
    if (counter > 1 && slots.size() > 2)
    {
      label_ss << delim << "...";
      break;
    }
  }

  return label_ss.str();
}
namespace
{
// PseudoInverse estimator for slate recommendation. The following implements
// the case for a Cartesian product when the logging policy is a product
// distribution. This can be seen in example 4 of the paper.
// https://arxiv.org/abs/1605.04812
float get_estimate(
    const ACTION_SCORE::action_scores& label_probs, float cost, const VW::decision_scores_t& prediction_probs)
{
  assert(!label_probs.empty());
  assert(!prediction_probs.empty());
  assert(label_probs.size() == prediction_probs.size());

  float p_over_ps = 0.f;
  const size_t number_of_slots = label_probs.size();
  for (size_t slot_index = 0; slot_index < number_of_slots; slot_index++)
  { p_over_ps += (prediction_probs[slot_index][0].score / label_probs[slot_index].score); }
  p_over_ps -= (number_of_slots - 1);

  return cost * p_over_ps;
}

void output_example(VW::workspace& all, const VW::reductions::slates_data& /*c*/, const VW::multi_ex& ec_seq)
{
  VW::multi_ex slots;
  size_t num_features = 0;
  float loss = 0.;
  bool is_labelled = ec_seq[SHARED_EX_INDEX]->l.slates.labeled;
  float cost = is_labelled ? ec_seq[SHARED_EX_INDEX]->l.slates.cost : 0.f;
  v_array<ACTION_SCORE::action_score> label_probs;

  for (auto* ec : ec_seq)
  {
    num_features += ec->get_num_features();

    if (ec->l.slates.type == VW::slates::example_type::slot)
    {
      slots.push_back(ec);
      if (is_labelled)
      {
        const auto& this_example_label_probs = ec->l.slates.probabilities;
        if (this_example_label_probs.empty()) { THROW("Probabilities missing for labeled example"); }
        // Only care about top action for each slot.
        label_probs.push_back(this_example_label_probs[0]);
      }
    }
  }

  // Calculate the estimate for this example based on the pseudo inverse estimator.
  const auto& predictions = ec_seq[0]->pred.decision_scores;
  if (is_labelled) { loss = get_estimate(label_probs, cost, predictions); }
  label_probs.clear();

  bool holdout_example = is_labelled;
  if (holdout_example != false)
  {
    for (const auto& example : ec_seq) { holdout_example &= example->test_only; }
  }

  all.sd->update(holdout_example, is_labelled, loss, ec_seq[SHARED_EX_INDEX]->weight, num_features);

  for (auto& sink : all.final_prediction_sink)
  { VW::print_decision_scores(sink.get(), ec_seq[SHARED_EX_INDEX]->pred.decision_scores, all.logger); }

  VW::print_update_slates(all, slots, predictions, num_features);
}

void finish_multiline_example(VW::workspace& all, VW::reductions::slates_data& data, VW::multi_ex& ec_seq)
{
  if (!ec_seq.empty())
  {
    output_example(all, data, ec_seq);
    VW::details::global_print_newline(all.final_prediction_sink, all.logger);
    for (auto& action_scores : ec_seq[0]->pred.decision_scores) { action_scores.clear(); }
    ec_seq[0]->pred.decision_scores.clear();
  }

  VW::finish_example(all, ec_seq);
}

template <bool is_learn>
void learn_or_predict(VW::reductions::slates_data& data, VW::LEARNER::multi_learner& base, VW::multi_ex& examples)
{
  if (is_learn) { data.learn(base, examples); }
  else
  {
    data.predict(base, examples);
  }
}
}  // namespace

VW::LEARNER::base_learner* VW::reductions::slates_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();
  auto data = VW::make_unique<slates_data>();
  bool slates_option = false;
  option_group_definition new_options("[Reduction] Slates");
  new_options.add(make_option("slates", slates_option).keep().necessary().help("Enable slates reduction"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  if (!options.was_supplied("ccb_explore_adf"))
  {
    options.insert("ccb_explore_adf", "");
    options.add_and_parse(new_options);
  }

  auto* base = as_multiline(stack_builder.setup_base_learner());
  all.example_parser->lbl_parser = VW::slates::slates_label_parser;
  auto* l = VW::LEARNER::make_reduction_learner(std::move(data), base, learn_or_predict<true>, learn_or_predict<false>,
      stack_builder.get_setupfn_name(slates_setup))
                .set_learn_returns_prediction(base->learn_returns_prediction)
                .set_input_prediction_type(VW::prediction_type_t::decision_probs)
                .set_output_prediction_type(VW::prediction_type_t::decision_probs)
                .set_input_label_type(VW::label_type_t::slates)
                .set_output_label_type(VW::label_type_t::ccb)
                .set_finish_example(finish_multiline_example)
                .build();
  return VW::LEARNER::make_base(*l);
}
