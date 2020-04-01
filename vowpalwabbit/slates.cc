// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "slates.h"
#include "example.h"
#include "slates_label.h"
#include "reductions.h"
#include "global_data.h"

#include "vw.h"
#include <algorithm>
#include "cb_algs.h"
#include "cb_adf.h"
#include "conditional_contextual_bandit.h"
#include "decision_scores.h"
#include "action_score.h"
#include "ccb_label.h"

using namespace VW::config;

namespace slates
{
template <bool is_learn>
void slates_data::learn_or_predict(LEARNER::multi_learner& base, multi_ex& examples)
{
  _stashed_labels.clear();
  _stashed_labels.reserve(examples.size());
  for (auto& example : examples)
  {
    _stashed_labels.push_back(std::move(example->l.slates));
  }

  const size_t num_slots = std::count_if(examples.begin(), examples.end(),
      [](const example* example) { return example->l.conditional_contextual_bandit.type == CCB::example_type::slot; });

  float global_cost = 0.f;
  bool global_cost_found = false;
  uint32_t action_index = 0;
  size_t slot_index = 0;
  std::vector<std::vector<uint32_t>> slot_action_pools(num_slots);
  for (size_t i = 0; i < examples.size(); i++)
  {
    CCB::label ccb_label;
    memset(&ccb_label, 0, sizeof(ccb_label));
    CCB::ccb_label_parser.default_label(&ccb_label);
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
      if (slates_label.slot_id >= num_slots)
      {
        THROW("slot_id cannot be larger than or equal to the number of slots");
      }
      ccb_label.type = CCB::example_type::action;
      slot_action_pools[slates_label.slot_id].push_back(action_index);
      action_index++;
    }
    else if (slates_label.type == slates::example_type::slot)
    {
      ccb_label.type = CCB::example_type::slot;
      if (global_cost_found)
      {
        ccb_label.outcome = new CCB::conditional_contextual_bandit_outcome();
        ccb_label.outcome->cost = global_cost;
        ccb_label.outcome->probabilities = v_init<ACTION_SCORE::action_score>();
        ccb_label.explicit_included_actions = v_init<uint32_t>();
        for (const auto index : slot_action_pools[slot_index])
        {
          ccb_label.explicit_included_actions.push_back(index);
        }
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
  LEARNER::multiline_learn_or_predict<is_learn>(base, examples, examples[0]->ft_offset);

  // Need to convert decision scores to the original index space. This can be
  // done by going through the prediction for each slots and subtracting the
  // number of actions seen so far.
  uint32_t size_so_far = 0;
  for (auto& action_scores : examples[0]->pred.decision_scores)
  {
    for (size_t i = 0; i < action_scores.size(); i++)
    {
      action_scores[i].action = action_scores[i].action - size_so_far;
    }
    size_so_far += static_cast<uint32_t>(action_scores.size());
  }

  for (size_t i = 0; i < examples.size(); i++)
  {
    CCB::ccb_label_parser.delete_label(&examples[i]->l.conditional_contextual_bandit);
    examples[i]->l.slates = std::move(_stashed_labels[i]);
  }
  _stashed_labels.clear();
}

void slates_data::learn(LEARNER::multi_learner& base, multi_ex& examples)
{
  learn_or_predict<true>(base, examples);
}

void slates_data::predict(LEARNER::multi_learner& base, multi_ex& examples)
{
  learn_or_predict<false>(base, examples);
}

// TODO this abstraction may not really work as this function now doens't have access to the global cost...
std::string generate_slates_label_printout(const std::vector<example*>& slots)
{
  size_t counter = 0;
  std::stringstream label_ss;
  std::string delim;
  for (const auto& slot : slots)
  {
    counter++;
    const auto& label = slot->l.slates;
    if (label.labeled )
    {
      label_ss << delim << label.probabilities[0].action;
    }
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

void output_example(vw& all, slates_data& /*c*/, multi_ex& ec_seq)
{
  std::vector<example*> slots;
  size_t num_features = 0;
  float loss = 0.;
  bool is_labelled = ec_seq[SHARED_EX_INDEX]->l.slates.labeled;
  float cost = is_labelled ? ec_seq[SHARED_EX_INDEX]->l.slates.cost : 0.f;

  for (auto ec : ec_seq)
  {
    num_features += ec->num_features;

    if (ec->l.slates.type == slates::example_type::slot)
    {
      slots.push_back(ec);
    }
  }

  auto predictions = ec_seq[0]->pred.decision_scores;
  for (size_t slot_index = 0; slot_index < slots.size(); slot_index++)
  {
    const auto& label_probabilties = slots[slot_index]->l.slates.probabilities;
    if (is_labelled)
    {
      if (label_probabilties.empty())
      {
        THROW("Probabilities missing for labeled example");
      }

      float l = CB_ALGS::get_cost_estimate(
          label_probabilties[TOP_ACTION_INDEX], cost, predictions[slot_index][TOP_ACTION_INDEX].action);
      loss += l * predictions[slot_index][TOP_ACTION_INDEX].score;
    }
  }

  bool holdout_example = is_labelled;
  if (holdout_example != false)
  {
    for (const auto& example : ec_seq)
    {
      holdout_example &= example->test_only;
    }
  }

  all.sd->update(holdout_example, is_labelled, loss, ec_seq[SHARED_EX_INDEX]->weight, num_features);

  for (auto sink : all.final_prediction_sink)
  {
    VW::print_decision_scores(sink, ec_seq[SHARED_EX_INDEX]->pred.decision_scores);
  }

  VW::print_update_slates(all, slots, predictions, num_features);
}

void finish_multiline_example(vw& all, slates_data& data, multi_ex& ec_seq)
{
  if (!ec_seq.empty())
  {
    output_example(all, data, ec_seq);
    CB_ADF::global_print_newline(all.final_prediction_sink);
  }

  VW::finish_example(all, ec_seq);
}

template <bool is_learn>
void learn_or_predict(slates_data& data, LEARNER::multi_learner& base, multi_ex& examples)
{
  if (is_learn)
  {
    data.learn(base, examples);
  }
  else
  {
    data.predict(base, examples);
  }
}

LEARNER::base_learner* slates_setup(options_i& options, vw& all)
{
  auto data = scoped_calloc_or_throw<slates_data>();
  bool slates_option = false;
  option_group_definition new_options("Slates");
  new_options.add(make_option("slates", slates_option).keep().help("EXPERIMENTAL"));
  options.add_and_parse(new_options);

  if (!slates_option)
  {
    return nullptr;
  }

  if (!options.was_supplied("ccb_explore_adf"))
  {
    options.insert("ccb_explore_adf", "");
    options.add_and_parse(new_options);
  }

  auto base = as_multiline(setup_base(options, all));
  all.p->lp = slates_label_parser;
  all.label_type = label_type_t::slates;
  all.delete_prediction = VW::delete_decision_scores;
  auto& l = LEARNER::init_learner(
      data, base, learn_or_predict<true>, learn_or_predict<false>, 1, prediction_type_t::decision_probs);
  l.set_finish_example(finish_multiline_example);
  return LEARNER::make_base(l);
}
}  // namespace slates
