// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "conditional_contextual_bandit.h"
#include "reductions.h"
#include "example.h"
#include "global_data.h"
#include "vw.h"
#include "interactions.h"
#include "label_dictionary.h"
#include "cb_adf.h"
#include "cb_algs.h"
#include "constant.h"
#include "v_array_pool.h"

#include <numeric>
#include <algorithm>
#include <unordered_set>
#include <bitset>

using namespace LEARNER;
using namespace VW;
using namespace VW::config;

template <typename T>
void return_v_array(v_array<T>& array, VW::v_array_pool<T>& pool)
{
  array.clear();
  pool.return_object(array);
}

struct ccb
{
  vw* all;
  example* shared;
  std::vector<example*> actions, slots;
  std::vector<uint32_t> origin_index;
  CB::cb_class cb_label, default_cb_label;
  std::vector<bool> exclude_list, include_list;
  std::vector<std::string> generated_interactions;
  std::vector<std::string>* original_interactions;
  std::vector<CCB::label> stored_labels;
  size_t action_with_label;

  multi_ex cb_ex;

  // All of these hashes are with a hasher seeded with the below namespace hash.
  std::vector<uint64_t> slot_id_hashes;
  uint64_t id_namespace_hash;
  std::string id_namespace_str;

  size_t base_learner_stride_shift;

  VW::v_array_pool<CB::cb_class> cb_label_pool;
  VW::v_array_pool<ACTION_SCORE::action_score> action_score_pool;
};

namespace CCB
{
static constexpr uint32_t SHARED_EX_INDEX = 0;
static constexpr uint32_t TOP_ACTION_INDEX = 0;

void clear_all(ccb& data)
{
  // data.include_list and data.exclude_list aren't cleared here but are assigned in the predict/learn function
  data.shared = nullptr;
  data.actions.clear();
  data.slots.clear();
  data.action_with_label = 0;
  data.stored_labels.clear();
}

// split the slots, the actions and the shared example from the multiline example
bool split_multi_example_and_stash_labels(const multi_ex& examples, ccb& data)
{
  for (auto ex : examples)
  {
    switch (ex->l.conditional_contextual_bandit.type)
    {
      case example_type::shared:
        data.shared = ex;
        break;
      case example_type::action:
        data.actions.push_back(ex);
        break;
      case example_type::slot:
        data.slots.push_back(ex);
        break;
      default:
        std::cout << "ccb_adf_explore: badly formatted example - invalid example type";
        return false;
    }

    // Stash the CCB labels before rewriting them.
    data.stored_labels.push_back({ex->l.conditional_contextual_bandit.type, ex->l.conditional_contextual_bandit.outcome,
        ex->l.conditional_contextual_bandit.explicit_included_actions, 0.});
  }

  return true;
}

template <bool is_learn>
bool sanity_checks(ccb& data)
{
  if (data.slots.size() > data.actions.size())
  {
    std::cerr << "ccb_adf_explore: badly formatted example - number of actions " << data.actions.size()
              << " must be greater than the number of slots " << data.slots.size();
    return false;
  }

  if (is_learn)
  {
    for (auto slot : data.slots)
    {
      if (slot->l.conditional_contextual_bandit.outcome != nullptr &&
          slot->l.conditional_contextual_bandit.outcome->probabilities.size() == 0)
      {
        std::cerr << "ccb_adf_explore: badly formatted example - missing label probability";
        return false;
      }
    }
  }
  return true;
}

// create empty/default cb labels
void create_cb_labels(ccb& data)
{
  data.shared->l.cb.costs = data.cb_label_pool.get_object();
  data.shared->l.cb.costs.push_back(data.default_cb_label);
  for (example* action : data.actions)
  {
    action->l.cb.costs = data.cb_label_pool.get_object();
  }
  data.shared->l.cb.weight = 1.0;
}

// the polylabel (union) must be manually cleaned up
void delete_cb_labels(ccb& data)
{
  return_v_array(data.shared->l.cb.costs, data.cb_label_pool);

  for (example* action : data.actions)
  {
    return_v_array(action->l.cb.costs, data.cb_label_pool);
  }
}

void attach_label_to_example(
    uint32_t action_index_one_based, example* example, conditional_contextual_bandit_outcome* outcome, ccb& data)
{
  // save the cb label
  // Action is unused in cb
  data.cb_label.action = action_index_one_based;
  data.cb_label.probability = outcome->probabilities[0].score;
  data.cb_label.cost = outcome->cost;

  example->l.cb.costs.push_back(data.cb_label);
}

void save_action_scores(ccb& data, decision_scores_t& decision_scores)
{
  auto& pred = data.shared->pred.a_s;
  decision_scores.push_back(pred);

  // correct indices: we want index relative to the original ccb multi-example, with no actions filtered
  for (auto& action_score : pred)
  {
    action_score.action = data.origin_index[action_score.action];
  }

  // Exclude the chosen action from next slots.
  auto original_index_of_chosen_action = pred[0].action;
  data.exclude_list[original_index_of_chosen_action] = true;
}

void clear_pred_and_label(ccb& data)
{
  // Don't need to return to pool, as that will be done when the example is output.

  // This just needs to be cleared as it is reused.
  data.actions[data.action_with_label]->l.cb.costs.clear();
}

// true if there exists at least 1 action in the cb multi-example
bool has_action(multi_ex& cb_ex) { return !cb_ex.empty(); }

// This function intentionally does not handle increasing the num_features of the example because
// the output_example function has special logic to ensure the number of feaures is correctly calculated.
// Copy anything in default namespace for slot to ccb_slot_namespace in shared
// Copy other slot namespaces to shared
void inject_slot_features(example* shared, example* slot)
{
  for (auto index : slot->indices)
  {
    // constant namespace should be ignored, as it already exists and we don't want to double it up.
    if (index == constant_namespace)
    {
      continue;
    }
    else if (index == default_namespace)  // slot default namespace has a special namespace in shared
    {
      LabelDict::add_example_namespace(*shared, ccb_slot_namespace, slot->feature_space[default_namespace]);
    }
    else
    {
      LabelDict::add_example_namespace(*shared, index, slot->feature_space[index]);
    }
  }
}

template <bool audit>
void inject_slot_id(ccb& data, example* shared, size_t id)
{
  // id is zero based, so the vector must be of size id + 1
  if (id + 1 > data.slot_id_hashes.size())
  {
    data.slot_id_hashes.resize(id + 1, 0);
  }

  uint64_t index;
  if (data.slot_id_hashes[id] == 0)
  {
    auto current_index_str = "index" + std::to_string(id);
    index = VW::hash_feature(*data.all, current_index_str, data.id_namespace_hash);

    // To maintain indicies consistent with what the parser does we must scale.
    index *= static_cast<uint64_t>(data.all->wpp) << data.base_learner_stride_shift;
    data.slot_id_hashes[id] = index;
  }
  else
  {
    index = data.slot_id_hashes[id];
  }

  shared->feature_space[ccb_id_namespace].push_back(1., index);
  shared->indices.push_back(ccb_id_namespace);

  if (audit)
  {
    auto current_index_str = "index" + std::to_string(id);
    shared->feature_space[ccb_id_namespace].space_names.push_back(
        audit_strings_ptr(new audit_strings(data.id_namespace_str, current_index_str)));
  }
}

// Since the slot id is the only thing in this namespace, the popping the value off will work correctly.
template <bool audit>
void remove_slot_id(example* shared)
{
  shared->feature_space[ccb_id_namespace].indicies.pop();
  shared->feature_space[ccb_id_namespace].values.pop();
  shared->indices.pop();

  if (audit)
  {
    shared->feature_space[ccb_id_namespace].space_names.pop();
  }
}

void remove_slot_features(example* shared, example* slot)
{
  for (auto index : slot->indices)
  {
    // constant namespace should be ignored, as it already exists and we don't want to double it up.
    if (index == constant_namespace)
    {
      continue;
    }

    else if (index == default_namespace)  // slot default namespace has a special namespace in shared
    {
      LabelDict::del_example_namespace(*shared, ccb_slot_namespace, slot->feature_space[32]);
    }
    else
    {
      LabelDict::del_example_namespace(*shared, index, slot->feature_space[index]);
    }
  }
}

// Generates quadratics between each namespace and the slot id as well as appends slot id to every existing interaction.
void calculate_and_insert_interactions(
    example* shared, std::vector<example*> actions, std::vector<std::string>& generated_interactions)
{
  std::bitset<INTERACTIONS::printable_ns_size> found_namespaces;

  const auto original_size = generated_interactions.size();
  for (size_t i = 0; i < original_size; i++)
  {
    auto interaction_copy = generated_interactions[i];
    interaction_copy.push_back((char)ccb_id_namespace);
    generated_interactions.push_back(interaction_copy);
  }

  for (const auto& action : actions)
  {
    for (const auto& action_index : action->indices)
    {
      if (INTERACTIONS::is_printable_namespace(action_index) &&
          !found_namespaces[action_index - INTERACTIONS::printable_start])
      {
        found_namespaces[action_index - INTERACTIONS::printable_start] = true;
        generated_interactions.push_back({(char)action_index, (char)ccb_id_namespace});
      }
    }
  }

  for (const auto& shared_index : shared->indices)
  {
    if (INTERACTIONS::is_printable_namespace(shared_index) &&
        !found_namespaces[shared_index - INTERACTIONS::printable_start])
    {
      found_namespaces[shared_index - INTERACTIONS::printable_start] = true;
      generated_interactions.push_back({(char)shared_index, (char)ccb_id_namespace});
    }
  }
}

// build a cb example from the ccb example
template <bool is_learn>
void build_cb_example(multi_ex& cb_ex, example* slot, ccb& data)
{
  bool slot_has_label = slot->l.conditional_contextual_bandit.outcome != nullptr;

  // Merge the slot features with the shared example and set it in the cb multi-example
  // TODO is it imporant for total_sum_feat_sq and num_features to be correct at this point?
  inject_slot_features(data.shared, slot);
  cb_ex.push_back(data.shared);

  // Retrieve the action index whitelist (if the list is empty, then all actions are white-listed)
  auto& explicit_includes = slot->l.conditional_contextual_bandit.explicit_included_actions;
  if (explicit_includes.size() != 0)
  {
    // First time seeing this, initialize the vector with falses so we can start setting each included action.
    if (data.include_list.size() == 0)
    {
      data.include_list.assign(data.actions.size(), false);
    }

    for (uint32_t included_action_id : explicit_includes)
    {
      data.include_list[included_action_id] = true;
    }
  }

  // set the available actions in the cb multi-example
  uint32_t index = 0;
  data.origin_index.clear();
  data.origin_index.resize(data.actions.size(), 0);
  for (size_t i = 0; i < data.actions.size(); i++)
  {
    // Filter actions that are not explicitly included. If the list is empty though, everything is included.
    if (!data.include_list.empty() && !data.include_list[i])
      continue;

    // Filter actions chosen by previous slots
    if (data.exclude_list[i])
      continue;

    // Select the action
    cb_ex.push_back(data.actions[i]);

    // Save the original index from the root multi-example
    data.origin_index[index++] = (uint32_t)i;

    // Remember the index of the chosen action
    if (is_learn && slot_has_label && i == slot->l.conditional_contextual_bandit.outcome->probabilities[0].action)
    {
      // This is used to remove the label later.
      data.action_with_label = (uint32_t)i;
      attach_label_to_example(index, data.actions[i], slot->l.conditional_contextual_bandit.outcome, data);
    }
  }

  // Must provide a prediction that cb can write into, this will be saved into the decision scores object later.
  data.shared->pred.a_s = data.action_score_pool.get_object();

  // Tag can be used for specifying the sampling seed per slot. For it to be used it must be inserted into the shared
  // example.
  std::swap(data.shared->tag, slot->tag);
}

// iterate over slots contained in the multi-example, and for each slot, build a cb example and perform a
// cb_explore_adf call.
template <bool is_learn>
void learn_or_predict(ccb& data, multi_learner& base, multi_ex& examples)
{
  clear_all(data);
  if (!split_multi_example_and_stash_labels(examples, data))  // split shared, actions and slots
    return;

#ifndef NDEBUG
  if (!sanity_checks<is_learn>(data))
    return;
#endif

  // This will overwrite the labels with CB.
  create_cb_labels(data);

  // Reset exclusion list for this example.
  data.exclude_list.assign(data.actions.size(), false);

  auto decision_scores = examples[0]->pred.decision_scores;

  // for each slot, re-build the cb example and call cb_explore_adf
  size_t slot_id = 0;
  for (example* slot : data.slots)
  {
    // Namespace crossing for slot features.
    data.generated_interactions.clear();
    std::copy(data.original_interactions->begin(), data.original_interactions->end(),
        std::back_inserter(data.generated_interactions));
    calculate_and_insert_interactions(data.shared, data.actions, data.generated_interactions);
    data.shared->interactions = &data.generated_interactions;
    for (auto ex : data.actions)
    {
      ex->interactions = &data.generated_interactions;
    }

    data.include_list.clear();
    build_cb_example<is_learn>(data.cb_ex, slot, data);

    if (data.all->audit)
      inject_slot_id<true>(data, data.shared, slot_id);
    else
      inject_slot_id<false>(data, data.shared, slot_id);

    if (has_action(data.cb_ex))
    {
      // the cb example contains at least 1 action
      multiline_learn_or_predict<is_learn>(base, data.cb_ex, examples[0]->ft_offset);
      save_action_scores(data, decision_scores);
      clear_pred_and_label(data);
    }
    else
    {
      // the cb example contains no action => cannot decide
      decision_scores.push_back(data.action_score_pool.get_object());
    }

    data.shared->interactions = data.original_interactions;
    for (auto ex : data.actions)
    {
      ex->interactions = data.original_interactions;
    }
    remove_slot_features(data.shared, slot);

    if (data.all->audit)
      remove_slot_id<true>(data.shared);
    else
      remove_slot_id<false>(data.shared);

    // Put back the original shared example tag.
    std::swap(data.shared->tag, slot->tag);
    slot_id++;
    data.cb_ex.clear();
  }

  delete_cb_labels(data);

  // Restore ccb labels to the example objects.
  for (size_t i = 0; i < examples.size(); i++)
  {
    examples[i]->l.conditional_contextual_bandit = {
        data.stored_labels[i].type, data.stored_labels[i].outcome, data.stored_labels[i].explicit_included_actions, 0.};
  }

  // Save the predictions
  examples[0]->pred.decision_scores = decision_scores;
}

void print_decision_scores(int f, decision_scores_t& decision_scores)
{
  if (f >= 0)
  {
    std::stringstream ss;
    for (auto slot : decision_scores)
    {
      std::string delimiter = "";
      for (auto action_score : slot)
      {
        ss << delimiter << action_score.action << ':' << action_score.score;
        delimiter = ",";
      }
      ss << '\n';
    }
    ssize_t len = ss.str().size();
    ssize_t t = io_buf::write_file_or_socket(f, ss.str().c_str(), (unsigned int)len);
    if (t != len)
      std::cerr << "write error: " << strerror(errno) << std::endl;
  }
}

void print_update(vw& all, std::vector<example*>& slots, decision_scores_t& decision_scores, size_t num_features)
{
  if (all.sd->weighted_examples() >= all.sd->dump_interval && !all.quiet && !all.bfgs)
  {
    std::string label_str = "";
    std::string delim = "";
    int counter = 0;
    for (auto slot : slots)
    {
      counter++;

      auto outcome = slot->l.conditional_contextual_bandit.outcome;
      if (outcome == nullptr)
      {
        label_str += delim;
        label_str += "?";
      }
      else
      {
        label_str += delim;
        label_str += std::to_string(outcome->probabilities[0].action);
        label_str += ":";
        label_str += std::to_string(outcome->cost);
      }

      delim = ",";

      // Stop after 2...
      if (counter > 1 && slots.size() > 2)
      {
        label_str += delim;
        label_str += "...";
        break;
      }
    }
    std::ostringstream label_buf;
    label_buf << std::setw(all.sd->col_current_label) << std::right << std::setfill(' ') << label_str;

    std::string pred_str = "";
    delim = "";
    counter = 0;
    for (auto slot : decision_scores)
    {
      counter++;
      pred_str += delim;
      pred_str += std::to_string(slot[0].action);
      delim = ",";

      // Stop after 3...
      if (counter > 2)
      {
        pred_str += delim;
        pred_str += "...";
        break;
      }
    }
    std::ostringstream pred_buf;
    pred_buf << std::setw(all.sd->col_current_predict) << std::right << std::setfill(' ') << pred_str;

    all.sd->print_update(all.holdout_set_off, all.current_pass, label_buf.str(), pred_buf.str(), num_features,
        all.progress_add, all.progress_arg);
  }
}

void output_example(vw& all, ccb& /*c*/, multi_ex& ec_seq)
{
  if (ec_seq.size() <= 0)
    return;

  std::vector<example*> slots;
  size_t num_features = 0;
  float loss = 0.;

  // Should this be done for shared, action and slot?
  for (auto ec : ec_seq)
  {
    num_features += ec->num_features;

    if (ec->l.conditional_contextual_bandit.type == CCB::example_type::slot)
    {
      slots.push_back(ec);
    }
  }

  // Is it hold out?
  size_t num_labelled = 0;
  auto preds = ec_seq[0]->pred.decision_scores;
  for (size_t i = 0; i < slots.size(); i++)
  {
    auto outcome = slots[i]->l.conditional_contextual_bandit.outcome;
    if (outcome != nullptr)
    {
      num_labelled++;
      float l = CB_ALGS::get_cost_estimate(
          outcome->probabilities[TOP_ACTION_INDEX], outcome->cost, preds[i][TOP_ACTION_INDEX].action);
      loss += l * preds[i][TOP_ACTION_INDEX].score;
    }
  }

  if (num_labelled > 0 && num_labelled < slots.size())
  {
    std::cerr << "Warning: Unlabeled example in train set, was this intentional?\n";
  }

  bool holdout_example = num_labelled > 0;
  for (size_t i = 0; i < ec_seq.size(); i++) holdout_example &= ec_seq[i]->test_only;

  // TODO what does weight mean here?
  all.sd->update(holdout_example, num_labelled > 0, loss, ec_seq[SHARED_EX_INDEX]->weight, num_features);

  for (auto sink : all.final_prediction_sink)
    print_decision_scores(sink, ec_seq[SHARED_EX_INDEX]->pred.decision_scores);

  CCB::print_update(all, slots, preds, num_features);
}

void finish_multiline_example(vw& all, ccb& data, multi_ex& ec_seq)
{
  if (ec_seq.size() > 0)
  {
    output_example(all, data, ec_seq);
    CB_ADF::global_print_newline(all.final_prediction_sink);
  }

  for (auto& a_s : ec_seq[0]->pred.decision_scores)
  {
    return_v_array(a_s, data.action_score_pool);
  }
  ec_seq[0]->pred.decision_scores.clear();

  VW::finish_example(all, ec_seq);
}

base_learner* ccb_explore_adf_setup(options_i& options, vw& all)
{
  auto data = scoped_calloc_or_throw<ccb>();
  bool ccb_explore_adf_option = false;
  bool slate = false;
  option_group_definition new_options(
      "EXPERIMENTAL: Conditional Contextual Bandit Exploration with Action Dependent Features");
  new_options.add(
      make_option("ccb_explore_adf", ccb_explore_adf_option)
          .keep()
          .help("EXPERIMENTAL: Do Conditional Contextual Bandit learning with multiline action dependent features."));
  new_options.add(make_option("slate", slate).keep().help("EXPERIMENTAL - MAY CHANGE: Enable slate mode in CCB."));
  options.add_and_parse(new_options);

  if (!ccb_explore_adf_option)
    return nullptr;

  if (!options.was_supplied("cb_explore_adf"))
  {
    options.insert("cb_explore_adf", "");
    options.add_and_parse(new_options);
  }

  if (options.was_supplied("cb_sample") && slate)
  {
    THROW("--slate and --cb_sample cannot be supplied together");
  }

  if (!options.was_supplied("cb_sample") && !slate)
  {
    options.insert("cb_sample", "");
    options.add_and_parse(new_options);
  }

  auto base = as_multiline(setup_base(options, all));
  all.p->lp = CCB::ccb_label_parser;
  all.label_type = label_type_t::ccb;

  // Stash the base learners stride_shift so we can properly add a feature later.
  data->base_learner_stride_shift = all.weights.stride_shift();

  // Extract from lower level reductions
  data->default_cb_label = {FLT_MAX, 0, -1.f, 0.f};
  data->shared = nullptr;
  data->original_interactions = &all.interactions;
  data->all = &all;

  data->id_namespace_str.push_back((char)ccb_id_namespace);
  data->id_namespace_str.append("_id");
  data->id_namespace_hash = VW::hash_space(all, data->id_namespace_str);

  learner<ccb, multi_ex>& l =
      init_learner(data, base, learn_or_predict<true>, learn_or_predict<false>, 1, prediction_type_t::decision_probs);

  all.delete_prediction = ACTION_SCORE::delete_action_scores;

  l.set_finish_example(finish_multiline_example);
  return make_base(l);
}

bool ec_is_example_header(example const& ec) { return ec.l.conditional_contextual_bandit.type == example_type::shared; }
}  // namespace CCB
