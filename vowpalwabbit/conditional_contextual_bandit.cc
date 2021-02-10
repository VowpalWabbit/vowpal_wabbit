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
#include "decision_scores.h"
#include "vw_versions.h"
#include "version.h"

#include <numeric>
#include <algorithm>
#include <unordered_set>
#include <bitset>

using namespace VW::LEARNER;
using namespace VW;
using namespace VW::config;

template <typename T>
void return_v_array(v_array<T>& array, VW::v_array_pool<T>& pool)
{
  array.clear();
  pool.return_object(array);
  array = v_init<T>();
}

struct ccb
{
  vw* all;
  example* shared;
  std::vector<example*> actions, slots;
  std::vector<uint32_t> origin_index;
  CB::cb_class cb_label, default_cb_label;
  std::vector<bool> exclude_list, include_list;
  namsepace_interactions generated_interactions;
  namsepace_interactions* original_interactions;
  std::vector<CCB::label> stored_labels;
  size_t action_with_label;

  multi_ex cb_ex;

  // All of these hashes are with a hasher seeded with the below namespace hash.
  std::vector<uint64_t> slot_id_hashes;
  uint64_t id_namespace_hash;
  std::string id_namespace_str;

  size_t base_learner_stride_shift;
  bool all_slots_loss_report;

  VW::v_array_pool<CB::cb_class> cb_label_pool;
  VW::v_array_pool<ACTION_SCORE::action_score> action_score_pool;

  VW::version_struct model_file_version;
  // If the reduction has not yet seen a multi slot example, it will behave the same as if it were CB.
  // This means the interactions aren't added and the slot feature is not added.
  bool has_seen_multi_slot_example = false;
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
  for (auto* ex : examples)
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

// create empty/default cb labels
void create_cb_labels(ccb& data)
{
  data.shared->l.cb.costs = data.cb_label_pool.get_object();
  data.shared->l.cb.costs.push_back(data.default_cb_label);
  for (example* action : data.actions) { action->l.cb.costs = data.cb_label_pool.get_object(); }
  data.shared->l.cb.weight = 1.0;
}

// the polylabel (union) must be manually cleaned up
void delete_cb_labels(ccb& data)
{
  return_v_array(data.shared->l.cb.costs, data.cb_label_pool);

  for (example* action : data.actions) { return_v_array(action->l.cb.costs, data.cb_label_pool); }
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

  // correct indices: we want index relative to the original ccb multi-example, with no actions filtered
  for (auto& action_score : pred) { action_score.action = data.origin_index[action_score.action]; }

  // Exclude the chosen action from next slots.
  auto original_index_of_chosen_action = pred[0].action;
  data.exclude_list[original_index_of_chosen_action] = true;

  decision_scores.push_back(pred);
  data.shared->pred.a_s = v_init<ACTION_SCORE::action_score>();
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
    if (index == constant_namespace) { continue; }

    if (index == default_namespace)  // slot default namespace has a special namespace in shared
    { LabelDict::add_example_namespace(*shared, ccb_slot_namespace, slot->feature_space[default_namespace]); }
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
  if (id + 1 > data.slot_id_hashes.size()) { data.slot_id_hashes.resize(id + 1, 0); }

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
  // shared->indices.push_back(ccb_id_namespace);
  shared->set_namespace(ccb_id_namespace, true);

  if (audit)
  {
    auto current_index_str = "index" + std::to_string(id);
    shared->feature_space[ccb_id_namespace].space_names.push_back(
        std::make_shared<audit_strings>(data.id_namespace_str, current_index_str));
  }
}

// Since the slot id is the only thing in this namespace, the popping the value off will work correctly.
template <bool audit>
void remove_slot_id(example* shared)
{
  shared->feature_space[ccb_id_namespace].indicies.pop();
  shared->feature_space[ccb_id_namespace].values.pop();
  shared->indices.pop();

  if (audit) { shared->feature_space[ccb_id_namespace].space_names.pop_back(); }
}

void remove_slot_features(example* shared, example* slot)
{
  for (auto index : slot->indices)
  {
    // constant namespace should be ignored, as it already exists and we don't want to double it up.
    if (index == constant_namespace) { continue; }

    if (index == default_namespace)  // slot default namespace has a special namespace in shared
    { LabelDict::del_example_namespace(*shared, ccb_slot_namespace, slot->feature_space[default_namespace]); }
    else
    {
      LabelDict::del_example_namespace(*shared, index, slot->feature_space[index]);
    }
  }
}

// Generates quadratics between each namespace and the slot id as well as appends slot id to every existing interaction.
void calculate_and_insert_interactions(
    example* shared, const std::vector<example*>& actions, namsepace_interactions& generated_interactions)
{
  std::bitset<INTERACTIONS::printable_ns_size> found_namespaces;

  generated_interactions.all_example_namespaces.emplace(ccb_slot_namespace);
  generated_interactions.extra_interactions.emplace(ccb_id_namespace);

  // TODO look at flatbuffers
  // TODO leave in duplicates

  for (const auto& action : actions)
  {
    for (const auto& action_index : action->indices)
    {
      // if (action_index != constant_namespace) { generated_interactions.all_example_namespaces.emplace(action_index); }
      if (INTERACTIONS::is_printable_namespace(action_index) &&
          !found_namespaces[action_index - INTERACTIONS::printable_start])
      {
        found_namespaces[action_index - INTERACTIONS::printable_start] = true;
        generated_interactions.interactions.push_back({action_index, ccb_id_namespace});
      }
    }
  }

  for (const auto& shared_index : shared->indices)
  {
    // if (shared_index != constant_namespace) { generated_interactions.all_example_namespaces.emplace(shared_index); }
    if (INTERACTIONS::is_printable_namespace(shared_index) &&
        !found_namespaces[shared_index - INTERACTIONS::printable_start])
    {
      found_namespaces[shared_index - INTERACTIONS::printable_start] = true;
      generated_interactions.interactions.push_back({shared_index, ccb_id_namespace});
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

  // Retrieve the list of actions explicitly available for the slot (if the list is empty, then all actions are
  // possible)
  auto& explicit_includes = slot->l.conditional_contextual_bandit.explicit_included_actions;
  if (!explicit_includes.empty())
  {
    // First time seeing this, initialize the vector with falses so we can start setting each included action.
    if (data.include_list.empty()) { data.include_list.assign(data.actions.size(), false); }

    for (uint32_t included_action_id : explicit_includes) { data.include_list[included_action_id] = true; }
  }

  // set the available actions in the cb multi-example
  uint32_t index = 0;
  data.origin_index.clear();
  data.origin_index.resize(data.actions.size(), 0);
  for (size_t i = 0; i < data.actions.size(); i++)
  {
    // Filter actions that are not explicitly included. If the list is empty though, everything is included.
    if (!data.include_list.empty() && !data.include_list[i]) { continue; }

    // Filter actions chosen by previous slots
    if (data.exclude_list[i]) { continue; }

    // Select the action
    cb_ex.push_back(data.actions[i]);

    // Save the original index from the root multi-example
    data.origin_index[index++] = (uint32_t)i;

    // Remember the index of the chosen action
    if (is_learn)
    {
      if (slot_has_label && i == slot->l.conditional_contextual_bandit.outcome->probabilities[0].action)
      {
        // This is used to remove the label later.
        data.action_with_label = (uint32_t)i;
        attach_label_to_example(index, data.actions[i], slot->l.conditional_contextual_bandit.outcome, data);
      }
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
  // split shared, actions and slots
  if (!split_multi_example_and_stash_labels(examples, data)) { return; }

  if (data.slots.size() > data.actions.size())
  {
    std::stringstream msg;
    msg << "ccb_adf_explore: badly formatted example - number of actions " << data.actions.size()
        << " must be greater than the number of slots " << data.slots.size();
    THROW(msg.str())
  }

  if (is_learn)
  {
    for (auto* slot : data.slots)
    {
      if (slot->l.conditional_contextual_bandit.outcome != nullptr &&
          slot->l.conditional_contextual_bandit.outcome->probabilities.empty())
      { THROW("ccb_adf_explore: badly formatted example - missing label probability"); }
    }
  }

  data.has_seen_multi_slot_example = data.has_seen_multi_slot_example || data.slots.size() > 1;

  // If we have not seen more than one slot, we need to check if the user has supplied slot features.
  // In that case we will turn on CCB slot id/interactions.
  if (!data.has_seen_multi_slot_example)
  {
    // We decide that user defined features exist if there is at least one feature space which is not the constant
    // namespace.
    const bool user_defined_slot_features_exist =
        !data.slots.empty() && !data.slots[0]->indices.empty() && data.slots[0]->indices[0] != constant_namespace;
    data.has_seen_multi_slot_example = data.has_seen_multi_slot_example || user_defined_slot_features_exist;
  }
  const bool should_augment_with_slot_info = data.has_seen_multi_slot_example;

  // This will overwrite the labels with CB.
  create_cb_labels(data);
  auto restore_guard = VW::scope_exit([&data, &examples] {
    delete_cb_labels(data);

    // Restore ccb labels to the example objects.
    for (size_t i = 0; i < examples.size(); i++)
    {
      examples[i]->l.conditional_contextual_bandit = {data.stored_labels[i].type, data.stored_labels[i].outcome,
          data.stored_labels[i].explicit_included_actions, 0.};
    }
  });

  // this is temporary only so we can get some logging of what's going on
  try
  {
    // Reset exclusion list for this example.
    data.exclude_list.assign(data.actions.size(), false);

    auto decision_scores = examples[0]->pred.decision_scores;

    // for each slot, re-build the cb example and call cb_explore_adf
    size_t slot_id = 0;
    for (example* slot : data.slots)
    {
      if (should_augment_with_slot_info)
      {
        // Namespace crossing for slot features.
        data.generated_interactions.interactions.clear();
        std::copy(data.original_interactions->interactions.begin(), data.original_interactions->interactions.end(),
            std::back_inserter(data.generated_interactions.interactions));
        data.generated_interactions.wild_card_expansion = data.original_interactions->wild_card_expansion;
        data.generated_interactions.active_interactions = data.original_interactions->active_interactions;
        data.generated_interactions.all_example_namespaces = data.original_interactions->all_example_namespaces;
        data.generated_interactions.extra_interactions = data.original_interactions->extra_interactions;
        data.generated_interactions.leave_duplicate_interactions = data.original_interactions->leave_duplicate_interactions;
        
        calculate_and_insert_interactions(data.shared, data.actions, data.generated_interactions);
        data.shared->interactions = &data.generated_interactions;
        for (auto* ex : data.actions) { ex->interactions = &data.generated_interactions; }
      }

      data.include_list.clear();
      build_cb_example<is_learn>(data.cb_ex, slot, data);

      if (should_augment_with_slot_info)
      {
        if (data.all->audit || data.all->hash_inv) { inject_slot_id<true>(data, data.shared, slot_id); }
        else
        {
          inject_slot_id<false>(data, data.shared, slot_id);
        }
      }

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

      if (should_augment_with_slot_info)
      {
        data.shared->interactions = data.original_interactions;
        for (auto* ex : data.actions) { ex->interactions = data.original_interactions; }
      }
      remove_slot_features(data.shared, slot);

      if (should_augment_with_slot_info)
      {
        if (data.all->audit || data.all->hash_inv) { remove_slot_id<true>(data.shared); }
        else
        {
          remove_slot_id<false>(data.shared);
        }
      }

      // Put back the original shared example tag.
      std::swap(data.shared->tag, slot->tag);
      slot_id++;
      data.cb_ex.clear();
    }
    // Save the predictions
    examples[0]->pred.decision_scores = decision_scores;
  }
  catch (std::exception& e)
  {
    data.all->trace_message << "CCB got exception from base reductions: " << e.what() << std::endl;
    throw;
  }
}

std::string generate_ccb_label_printout(const std::vector<example*>& slots)
{
  size_t counter = 0;
  std::stringstream label_ss;
  std::string delim;
  for (const auto& slot : slots)
  {
    counter++;

    auto* outcome = slot->l.conditional_contextual_bandit.outcome;
    if (outcome == nullptr) { label_ss << delim << "?"; }
    else
    {
      label_ss << delim << outcome->probabilities[0].action << ":" << outcome->cost;
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

void output_example(vw& all, ccb& c, multi_ex& ec_seq)
{
  if (ec_seq.empty()) { return; }

  std::vector<example*> slots;
  size_t num_features = 0;
  float loss = 0.;

  // Should this be done for shared, action and slot?
  for (auto* ec : ec_seq)
  {
    num_features += ec->num_features;

    if (ec->l.conditional_contextual_bandit.type == CCB::example_type::slot) { slots.push_back(ec); }
  }

  // Is it hold out?
  size_t num_labelled = 0;
  auto preds = ec_seq[0]->pred.decision_scores;
  for (size_t i = 0; i < slots.size(); i++)
  {
    auto* outcome = slots[i]->l.conditional_contextual_bandit.outcome;
    if (outcome != nullptr)
    {
      num_labelled++;
      if (i == 0 || c.all_slots_loss_report)
      {
        float l = CB_ALGS::get_cost_estimate(
            outcome->probabilities[TOP_ACTION_INDEX], outcome->cost, preds[i][TOP_ACTION_INDEX].action);
        loss += l * preds[i][TOP_ACTION_INDEX].score;
      }
    }
  }

  if (num_labelled > 0 && num_labelled < slots.size())
  { std::cerr << "Warning: Unlabeled example in train set, was this intentional?\n"; }

  bool holdout_example = num_labelled > 0;
  for (const auto& example : ec_seq) { holdout_example &= example->test_only; }

  // TODO what does weight mean here?
  all.sd->update(holdout_example, num_labelled > 0, loss, ec_seq[SHARED_EX_INDEX]->weight, num_features);

  for (auto& sink : all.final_prediction_sink)
  { VW::print_decision_scores(sink.get(), ec_seq[SHARED_EX_INDEX]->pred.decision_scores); }

  VW::print_update_ccb(all, slots, preds, num_features);
}

void finish_multiline_example(vw& all, ccb& data, multi_ex& ec_seq)
{
  if (!ec_seq.empty())
  {
    output_example(all, data, ec_seq);
    CB_ADF::global_print_newline(all.final_prediction_sink);
  }

  for (auto& a_s : ec_seq[0]->pred.decision_scores) { return_v_array(a_s, data.action_score_pool); }
  ec_seq[0]->pred.decision_scores.clear();

  VW::finish_example(all, ec_seq);
}

void save_load(ccb& sm, io_buf& io, bool read, bool text)
{
  if (io.num_files() == 0) { return; }

  // We want to enter this block if either we are writing, or reading a model file after the version in which this was
  // added.
  if (!read || sm.model_file_version >= VERSION_FILE_WITH_CCB_MULTI_SLOTS_SEEN_FLAG)
  {
    std::stringstream msg;
    if (!read) { msg << "CCB: has_seen_multi_slot_example = " << sm.has_seen_multi_slot_example << "\n"; }
    bin_text_read_write_fixed_validated(
        io, (char*)&sm.has_seen_multi_slot_example, sizeof(sm.has_seen_multi_slot_example), "", read, msg, text);
  }
}

base_learner* ccb_explore_adf_setup(options_i& options, vw& all)
{
  auto data = scoped_calloc_or_throw<ccb>();
  bool ccb_explore_adf_option = false;
  bool all_slots_loss_report = false;
  option_group_definition new_options("EXPERIMENTAL: Conditional Contextual Bandit Exploration with ADF");
  new_options
      .add(make_option("ccb_explore_adf", ccb_explore_adf_option)
               .keep()
               .necessary()
               .help(
                   "EXPERIMENTAL: Do Conditional Contextual Bandit learning with multiline action dependent features."))
      .add(make_option("all_slots_loss", all_slots_loss_report).help("Report average loss from all slots"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }
  data->all_slots_loss_report = all_slots_loss_report;
  if (!options.was_supplied("cb_explore_adf"))
  {
    options.insert("cb_explore_adf", "");
    options.add_and_parse(new_options);
  }

  if (!options.was_supplied("cb_sample"))
  {
    options.insert("cb_sample", "");
    options.add_and_parse(new_options);
  }

  auto* base = as_multiline(setup_base(options, all));
  all.example_parser->lbl_parser = CCB::ccb_label_parser;

  // Stash the base learners stride_shift so we can properly add a feature later.
  data->base_learner_stride_shift = all.weights.stride_shift();

  // Extract from lower level reductions
  data->default_cb_label = {FLT_MAX, 0, -1.f, 0.f};
  data->shared = nullptr;
  data->original_interactions = &all.interactions;
  data->all = &all;
  data->model_file_version = all.model_file_ver;

  data->id_namespace_str.append("_id");
  data->id_namespace_hash = VW::hash_space(all, data->id_namespace_str);

  learner<ccb, multi_ex>& l =
      init_learner(data, base, learn_or_predict<true>, learn_or_predict<false>, 1, prediction_type_t::decision_probs);

  all.delete_prediction = ACTION_SCORE::delete_action_scores;

  l.set_finish_example(finish_multiline_example);
  l.set_save_load(save_load);
  return make_base(l);
}

bool ec_is_example_header(example const& ec) { return ec.l.conditional_contextual_bandit.type == example_type::shared; }
bool ec_is_example_unset(example const& ec) { return ec.l.conditional_contextual_bandit.type == example_type::unset; }
}  // namespace CCB
