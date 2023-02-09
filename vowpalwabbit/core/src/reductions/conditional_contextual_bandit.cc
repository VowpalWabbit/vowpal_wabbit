// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/conditional_contextual_bandit.h"

#include "vw/config/options.h"
#include "vw/core/ccb_label.h"
#include "vw/core/ccb_reduction_features.h"
#include "vw/core/constant.h"
#include "vw/core/debug_log.h"
#include "vw/core/decision_scores.h"
#include "vw/core/example.h"
#include "vw/core/global_data.h"
#include "vw/core/interactions.h"
#include "vw/core/label_dictionary.h"
#include "vw/core/model_utils.h"
#include "vw/core/multi_ex.h"
#include "vw/core/print_utils.h"
#include "vw/core/reductions/cb/cb_adf.h"
#include "vw/core/reductions/cb/cb_algs.h"
#include "vw/core/setup_base.h"
#include "vw/core/shared_data.h"
#include "vw/core/v_array_pool.h"
#include "vw/core/version.h"
#include "vw/core/vw.h"
#include "vw/core/vw_versions.h"
#include "vw/io/logger.h"

#include <algorithm>
#include <bitset>
#include <numeric>
#include <unordered_set>

#undef VW_DEBUG_LOG
#define VW_DEBUG_LOG vw_dbg::CCB

using namespace VW::LEARNER;
using namespace VW;
using namespace VW::config;
using namespace VW::reductions::ccb;

namespace
{
template <typename T>
void return_collection(VW::v_array<T>& array, VW::v_array_pool<T>& pool)
{
  array.clear();
  pool.reclaim_object(std::move(array));
  array = VW::v_array<T>{};
}

template <typename T>
void return_collection(std::vector<T>& array, VW::vector_pool<T>& pool)
{
  array.clear();
  pool.reclaim_object(std::move(array));
  array = std::vector<T>{};
}

class ccb_data
{
public:
  VW::workspace* all = nullptr;
  VW::example* shared = nullptr;
  VW::multi_ex actions, slots;
  std::vector<uint32_t> origin_index;
  VW::cb_class cb_label;
  std::vector<bool> exclude_list, include_list;
  std::vector<VW::ccb_label> stored_labels;
  size_t action_with_label = 0;

  VW::multi_ex cb_ex;

  // All of these hashes are with a hasher seeded with the below namespace hash.
  std::vector<uint64_t> slot_id_hashes;
  uint64_t id_namespace_hash = 0;
  std::string id_namespace_str;
  std::string id_namespace_audit_str;

  size_t base_learner_stride_shift = 0;
  bool all_slots_loss_report = false;
  bool no_pred = false;

  VW::vector_pool<VW::cb_class> cb_label_pool;
  VW::v_array_pool<VW::action_score> action_score_pool;

  VW::version_struct model_file_version;
  // If the reduction has not yet seen a multi slot example, it will behave the same as if it were CB.
  // This means the interactions aren't added and the slot feature is not added.
  bool has_seen_multi_slot_example = false;
  // Introduction has_seen_multi_slot_example was breaking change in terms of model format.
  // This flag is required for loading cb models (which do not have has_seen_multi_slot_example flag) into ccb_data
  // reduction
  bool is_ccb_input_model = false;
};

void clear_all(ccb_data& data)
{
  // data.include_list and data.exclude_list aren't cleared here but are assigned in the predict/learn function
  data.shared = nullptr;
  data.actions.clear();
  data.slots.clear();
  data.action_with_label = 0;
  data.stored_labels.clear();
}

// split the slots, the actions and the shared example from the multiline example
bool split_multi_example_and_stash_labels(const VW::multi_ex& examples, ccb_data& data)
{
  for (auto* ex : examples)
  {
    switch (ex->l.conditional_contextual_bandit.type)
    {
      case VW::ccb_example_type::SHARED:
        data.shared = ex;
        break;
      case VW::ccb_example_type::ACTION:
        data.actions.push_back(ex);
        break;
      case VW::ccb_example_type::SLOT:
        data.slots.push_back(ex);
        break;
      default:
        data.all->logger.out_error("ccb_adf_explore: badly formatted example - invalid example type");
        return false;
    }

    // Stash the CCB labels before rewriting them.
    data.stored_labels.push_back(std::move(ex->l.conditional_contextual_bandit));
  }

  return true;
}

// create empty/default cb labels
void create_cb_labels(ccb_data& data)
{
  data.cb_label_pool.acquire_object(data.shared->l.cb.costs);
  data.shared->l.cb.costs.push_back(VW::cb_class{});
  for (VW::example* action : data.actions) { data.cb_label_pool.acquire_object(action->l.cb.costs); }
  data.shared->l.cb.weight = 1.0;
}

// the polylabel (union) must be manually cleaned up
void delete_cb_labels(ccb_data& data)
{
  return_collection(data.shared->l.cb.costs, data.cb_label_pool);
  data.shared->l.cb.costs.clear();

  for (VW::example* action : data.actions)
  {
    return_collection(action->l.cb.costs, data.cb_label_pool);
    action->l.cb.costs.clear();
  }
}

void attach_label_to_example(
    uint32_t action_index_one_based, VW::example* example, const VW::ccb_outcome* outcome, ccb_data& data)
{
  // save the cb label
  // Action is unused in cb
  data.cb_label.action = action_index_one_based;
  data.cb_label.probability = outcome->probabilities[0].score;
  data.cb_label.cost = outcome->cost;

  example->l.cb.costs.push_back(data.cb_label);
}

// This is used for outputting predictions for a slot. It will exclude the chosen action for labeled examples,
// otherwise it will exclude the action with the highest prediction.
void save_action_scores_and_exclude_top_action(ccb_data& data, decision_scores_t& decision_scores)
{
  auto& pred = data.shared->pred.a_s;

  // correct indices: we want index relative to the original ccb multi-example, with no actions filtered
  for (auto& action_score : pred) { action_score.action = data.origin_index[action_score.action]; }

  // Exclude the chosen action from next slots.
  const auto original_index_of_chosen_action = pred[0].action;
  data.exclude_list[original_index_of_chosen_action] = true;

  decision_scores.emplace_back(std::move(pred));
  data.shared->pred.a_s.clear();
}

// This is used to exclude the chosen action for a slot for a labeled example where no_predict is enabled.
void exclude_chosen_action(ccb_data& data, const VW::multi_ex& examples)
{
  int32_t action_index = -1;
  for (size_t i = 0; i < examples.size(); i++)
  {
    const VW::cb_label& ld = examples[i]->l.cb;
    if (ld.costs.size() == 1 && ld.costs[0].cost != FLT_MAX)
    {
      action_index = static_cast<int32_t>(i) - 1; /* un-1-index for shared */
      break;
    }
  }
  if (action_index == -1)
  {
    data.all->logger.err_warn("Unlabeled example used for learning only. Skipping over.");
    return;
  }
  data.exclude_list[action_index] = true;
}

void clear_pred_and_label(ccb_data& data)
{
  // Don't need to return to pool, as that will be done when the example is output.

  // This just needs to be cleared as it is reused.
  data.actions[data.action_with_label]->l.cb.costs.clear();
}

// true if there exists at least 1 action in the cb multi-example
bool has_action(VW::multi_ex& cb_ex) { return !cb_ex.empty(); }

// This function intentionally does not handle increasing the num_features of the example because
// the output_example function has special logic to ensure the number of features is correctly calculated.
// Copy anything in default namespace for slot to ccb_slot_namespace in shared
// Copy other slot namespaces to shared
void inject_slot_features(VW::example* shared, VW::example* slot)
{
  for (auto index : slot->indices)
  {
    // constant namespace should be ignored, as it already exists and we don't want to double it up.
    if (index == VW::details::CONSTANT_NAMESPACE) { continue; }

    if (index == VW::details::DEFAULT_NAMESPACE)  // slot default namespace has a special namespace in shared
    {
      VW::details::append_example_namespace(
          *shared, VW::details::CCB_SLOT_NAMESPACE, slot->feature_space[VW::details::DEFAULT_NAMESPACE]);
    }
    else { VW::details::append_example_namespace(*shared, index, slot->feature_space[index]); }
  }
}

template <bool audit>
void inject_slot_id(ccb_data& data, VW::example* shared, size_t id)
{
  // id is zero based, so the vector must be of size id + 1
  if (id + 1 > data.slot_id_hashes.size()) { data.slot_id_hashes.resize(id + 1, 0); }

  uint64_t index;
  if (data.slot_id_hashes[id] == 0)
  {
    const auto current_index_str = "index" + std::to_string(id);
    index = VW::hash_feature(*data.all, current_index_str, data.id_namespace_hash);

    // To maintain indices consistent with what the parser does we must scale.
    index *= static_cast<uint64_t>(data.all->wpp) << data.base_learner_stride_shift;
    data.slot_id_hashes[id] = index;
  }
  else { index = data.slot_id_hashes[id]; }

  shared->feature_space[VW::details::CCB_ID_NAMESPACE].push_back(1., index, VW::details::CCB_ID_NAMESPACE);
  shared->indices.push_back(VW::details::CCB_ID_NAMESPACE);
  if (id == 0) { shared->num_features++; }

  if (audit)
  {
    auto current_index_str = "index" + std::to_string(id);
    shared->feature_space[VW::details::CCB_ID_NAMESPACE].space_names.emplace_back(
        data.id_namespace_audit_str, current_index_str);
  }
}

// Since the slot id is the only thing in this namespace, the popping the value off will work correctly.
template <bool audit>
void remove_slot_id(VW::example* shared)
{
  shared->feature_space[VW::details::CCB_ID_NAMESPACE].clear();
  shared->indices.pop_back();
}

void remove_slot_features(VW::example* shared, VW::example* slot)
{
  for (auto index : slot->indices)
  {
    // constant namespace should be ignored, as it already exists and we don't want to double it up.
    if (index == VW::details::CONSTANT_NAMESPACE) { continue; }

    if (index == VW::details::DEFAULT_NAMESPACE)  // slot default namespace has a special namespace in shared
    {
      VW::details::truncate_example_namespace(
          *shared, VW::details::CCB_SLOT_NAMESPACE, slot->feature_space[VW::details::DEFAULT_NAMESPACE]);
    }
    else { VW::details::truncate_example_namespace(*shared, index, slot->feature_space[index]); }
  }
}

// build a cb example from the ccb example
template <bool is_learn>
void build_cb_example(VW::multi_ex& cb_ex, VW::example* slot, const VW::ccb_label& ccb_label, ccb_data& data)
{
  const bool slot_has_label = ccb_label.outcome != nullptr;

  // Merge the slot features with the shared example and set it in the cb multi-example
  // TODO is it important for total_sum_feat_sq and num_features to be correct at this point?
  inject_slot_features(data.shared, slot);
  cb_ex.push_back(data.shared);

  // Retrieve the list of actions explicitly available for the slot (if the list is empty, then all actions are
  // possible)
  const auto& explicit_includes = ccb_label.explicit_included_actions;
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
    data.origin_index[index++] = static_cast<uint32_t>(i);

    // Remember the index of the chosen action
    if (is_learn)
    {
      if (slot_has_label && i == ccb_label.outcome->probabilities[0].action)
      {
        // This is used to remove the label later.
        data.action_with_label = static_cast<uint32_t>(i);
        attach_label_to_example(index, data.actions[i], ccb_label.outcome, data);
      }
    }
  }

  // Must provide a prediction that cb can write into, this will be saved into the decision scores object later.
  data.action_score_pool.acquire_object(data.shared->pred.a_s);

  // Tag can be used for specifying the sampling seed per slot. For it to be used it must be inserted into the shared
  // example.
  std::swap(data.shared->tag, slot->tag);
}

VW_WARNING_STATE_PUSH
VW_WARNING_DISABLE_UNUSED_INTERNAL_DECLARATION
std::string ccb_decision_to_string(const ccb_data& data)
{
  std::ostringstream out_stream;
  auto& pred = data.shared->pred.a_s;
  // correct indices: we want index relative to the original ccb multi-example,
  // with no actions filtered
  out_stream << "a_s [";
  for (const auto& action_score : pred) { out_stream << action_score.action << ":" << action_score.score << ", "; }
  out_stream << "] ";

  out_stream << "excl [";
  for (auto excl : data.exclude_list) { out_stream << excl << ","; }
  out_stream << "] ";

  return out_stream.str();
}
VW_WARNING_STATE_POP

// iterate over slots contained in the multi-example, and for each slot, build a cb example and perform a
// cb_explore_adf call.
template <bool is_learn>
void learn_or_predict(ccb_data& data, learner& base, VW::multi_ex& examples)
{
  clear_all(data);
  // split shared, actions and slots
  if (!split_multi_example_and_stash_labels(examples, data)) { return; }
  auto restore_labels_guard = VW::scope_exit(
      [&data, &examples]
      {
        // Restore ccb labels to the example objects.
        for (size_t i = 0; i < examples.size(); i++)
        {
          examples[i]->l.conditional_contextual_bandit = std::move(data.stored_labels[i]);
        }
      });

  if (data.slots.size() > data.actions.size())
  {
    std::stringstream msg;
    msg << "ccb_adf_explore: badly formatted example - number of actions " << data.actions.size()
        << " must be greater than or equal to the number of slots " << data.slots.size();
    THROW(msg.str())
  }

  if (is_learn)
  {
    for (auto* slot : data.slots)
    {
      if (slot->l.conditional_contextual_bandit.outcome != nullptr &&
          slot->l.conditional_contextual_bandit.outcome->probabilities.empty())
      {
        THROW("ccb_adf_explore: badly formatted example - missing label probability")
      }
    }
  }

  auto previously_should_augment_with_slot_info = data.has_seen_multi_slot_example;
  data.has_seen_multi_slot_example = data.has_seen_multi_slot_example || data.slots.size() > 1;

  // If we have not seen more than one slot, we need to check if the user has supplied slot features.
  // In that case we will turn on CCB slot id/interactions.
  if (!data.has_seen_multi_slot_example)
  {
    // We decide that user defined features exist if there is at least one feature space which is not the constant
    // namespace.
    const bool user_defined_slot_features_exist = !data.slots.empty() && !data.slots[0]->indices.empty() &&
        data.slots[0]->indices[0] != VW::details::CONSTANT_NAMESPACE;
    data.has_seen_multi_slot_example = data.has_seen_multi_slot_example || user_defined_slot_features_exist;
  }
  const bool should_augment_with_slot_info = data.has_seen_multi_slot_example;

  // Even though the interactions reduction caches things when we move into CCB
  // mode a new namespace is added (VW::details::CCB_ID_NAMESPACE) and so we can be confident
  // that the cache will be invalidated.
  if (!previously_should_augment_with_slot_info && should_augment_with_slot_info)
  {
    insert_ccb_interactions(data.all->interactions, data.all->extent_interactions);
  }

  // This will overwrite the labels with CB.
  create_cb_labels(data);
  auto delete_cb_labels_guard = VW::scope_exit([&data] { delete_cb_labels(data); });

  // this is temporary only so we can get some logging of what's going on
  try
  {
    // Reset exclusion list for this example.
    data.exclude_list.assign(data.actions.size(), false);

    auto& decision_scores = examples[0]->pred.decision_scores;

    // for each slot, re-build the cb example and call cb_explore_adf
    size_t slot_id = 0;
    for (VW::example* slot : data.slots)
    {
      // shared, action, action, slot
      data.include_list.clear();
      assert(1 /* shared */ + data.actions.size() + slot_id < data.stored_labels.size());
      build_cb_example<is_learn>(
          data.cb_ex, slot, data.stored_labels[1 /* shared */ + data.actions.size() + slot_id], data);

      if (should_augment_with_slot_info)
      {
        if (data.all->audit || data.all->hash_inv) { inject_slot_id<true>(data, data.shared, slot_id); }
        else { inject_slot_id<false>(data, data.shared, slot_id); }
      }

      // the cb example contains at least 1 action
      if (has_action(data.cb_ex))
      {
        // Notes:  Prediction is needed for output purposes. i.e.
        // save_action_scores needs it.
        // This is will be used to a) display prediction, b) output to a predict
        // file c) progressive loss calcs
        //
        // Strictly speaking, predict is not needed to learn.  The only reason
        // for doing this here
        // instead of letting the framework call predict before learn is to
        // avoid extra work in example manipulation.
        //
        // The right thing to do here is to detect library mode and not have to
        // call predict if prediction is
        // not needed for learn.  This will be part of a future PR
        if (!is_learn) { multiline_learn_or_predict<false>(base, data.cb_ex, examples[0]->ft_offset); }
        else { multiline_learn_or_predict<true>(base, data.cb_ex, examples[0]->ft_offset); }

        if (!data.no_pred) { save_action_scores_and_exclude_top_action(data, decision_scores); }
        else { exclude_chosen_action(data, examples); }

        VW_DBG(examples) << "ccb "
                         << "slot:" << slot_id << " " << ccb_decision_to_string(data) << std::endl;
        for (const auto& ex : data.cb_ex)
        {
          if (VW::ec_is_example_header_cb(*ex)) { slot->num_features = (data.cb_ex.size() - 1) * ex->num_features; }
          else
          {
            slot->num_features += ex->num_features;
            slot->num_features_from_interactions += ex->num_features_from_interactions;
            slot->num_features -= ex->feature_space[VW::details::CONSTANT_NAMESPACE].size();
          }
        }
        clear_pred_and_label(data);
      }
      else
      {
        // the cb example contains no action => cannot decide
        decision_scores.emplace_back();
        data.action_score_pool.acquire_object(*(decision_scores.end() - 1));
      }

      remove_slot_features(data.shared, slot);

      if (should_augment_with_slot_info)
      {
        if (data.all->audit || data.all->hash_inv) { remove_slot_id<true>(data.shared); }
        else { remove_slot_id<false>(data.shared); }
      }

      // Put back the original shared example tag.
      std::swap(data.shared->tag, slot->tag);
      slot_id++;
      data.cb_ex.clear();
    }
  }
  catch (std::exception& e)
  {
    data.all->logger.err_error("CCB got exception from base reductions: {}", e.what());
    throw;
  }
}

void update_stats_ccb(const VW::workspace& /* all */, shared_data& sd, const ccb_data& data, const VW::multi_ex& ec_seq,
    VW::io::logger& logger)
{
  if (!ec_seq.empty() && !data.no_pred)
  {
    size_t num_features = 0;
    for (const auto* ec : data.slots) { num_features += ec->get_num_features(); }

    // Is it hold out?
    size_t num_labeled = 0;
    const auto& preds = ec_seq[0]->pred.decision_scores;
    float loss = 0.;
    for (size_t i = 0; i < data.slots.size(); i++)
    {
      auto* outcome = data.slots[i]->l.conditional_contextual_bandit.outcome;
      if (outcome != nullptr)
      {
        num_labeled++;
        if (i == 0 || data.all_slots_loss_report)
        {
          const float l = VW::get_cost_estimate(outcome->probabilities[VW::details::TOP_ACTION_INDEX], outcome->cost,
              preds[i][VW::details::TOP_ACTION_INDEX].action);
          loss += l * preds[i][VW::details::TOP_ACTION_INDEX].score * ec_seq[VW::details::SHARED_EX_INDEX]->weight;
        }
      }
    }

    if (num_labeled > 0 && num_labeled < data.slots.size())
    {
      logger.err_warn("Unlabeled example in train set, was this intentional?");
    }

    bool holdout_example = num_labeled > 0;
    for (const auto* example : ec_seq) { holdout_example &= example->test_only; }

    // TODO what does weight mean here?
    sd.update(holdout_example, num_labeled > 0, loss, ec_seq[VW::details::SHARED_EX_INDEX]->weight, num_features);
  }
}

void output_example_prediction_ccb(
    VW::workspace& all, const ccb_data& data, const VW::multi_ex& ec_seq, VW::io::logger& /* unused */)
{
  if (!ec_seq.empty() && !data.no_pred)
  {
    // Print predictions
    for (auto& sink : all.final_prediction_sink)
    {
      VW::print_decision_scores(sink.get(), ec_seq[VW::details::SHARED_EX_INDEX]->pred.decision_scores, all.logger);
    }
    VW::details::global_print_newline(all.final_prediction_sink, all.logger);
  }
}

void print_update_ccb(VW::workspace& all, shared_data& /* sd */, const ccb_data& data, const VW::multi_ex& ec_seq,
    VW::io::logger& /* unused */)
{
  const bool should_print_driver_update =
      all.sd->weighted_examples() >= all.sd->dump_interval && !all.quiet && !all.bfgs;

  if (should_print_driver_update && !ec_seq.empty() && !data.no_pred)
  {
    // Print progress
    size_t num_features = 0;
    for (auto* ec : data.slots) { num_features += ec->get_num_features(); }

    VW::print_update_ccb(all, data.slots, ec_seq[VW::details::SHARED_EX_INDEX]->pred.decision_scores, num_features);
  }
}

void cleanup_example_ccb(ccb_data& data, VW::multi_ex& ec_seq)
{
  if (!data.no_pred)
  {
    for (auto& a_s : ec_seq[VW::details::SHARED_EX_INDEX]->pred.decision_scores)
    {
      return_collection(a_s, data.action_score_pool);
    }
    ec_seq[VW::details::SHARED_EX_INDEX]->pred.decision_scores.clear();
  }
}

void save_load(ccb_data& sm, VW::io_buf& io, bool read, bool text)
{
  if (io.num_files() == 0) { return; }

  // We need to check if reading a model file after the version in which this was added.
  if (read &&
      (sm.model_file_version >= VW::version_definitions::VERSION_FILE_WITH_CCB_MULTI_SLOTS_SEEN_FLAG &&
          sm.is_ccb_input_model))
  {
    VW::model_utils::read_model_field(io, sm.has_seen_multi_slot_example);
  }
  else if (!read)
  {
    VW::model_utils::write_model_field(io, sm.has_seen_multi_slot_example, "CCB: has_seen_multi_slot_example", text);
  }

  if (read && sm.has_seen_multi_slot_example)
  {
    insert_ccb_interactions(sm.all->interactions, sm.all->extent_interactions);
  }
}
}  // namespace
std::shared_ptr<VW::LEARNER::learner> VW::reductions::ccb_explore_adf_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();
  auto data = VW::make_unique<ccb_data>();
  bool ccb_explore_adf_option = false;
  bool all_slots_loss_report = false;
  std::string type_string = "mtr";

  data->is_ccb_input_model = all.is_ccb_input_model;

  option_group_definition new_options("[Reduction] Conditional Contextual Bandit Exploration with ADF");
  new_options
      .add(make_option("ccb_explore_adf", ccb_explore_adf_option)
               .keep()
               .necessary()
               .help("Do Conditional Contextual Bandit learning with multiline action dependent features"))
      .add(make_option("all_slots_loss", all_slots_loss_report).help("Report average loss from all slots"))
      .add(make_option("no_predict", data->no_pred).help("Do not do a prediction when training"))
      .add(make_option("cb_type", type_string)
               .keep()
               .default_value("mtr")
               .one_of({"ips", "dm", "dr", "mtr", "sm"})
               .help("Contextual bandit method to use"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  // Ensure serialization of this option in all cases.
  if (!options.was_supplied("cb_type"))
  {
    options.insert("cb_type", type_string);
    options.add_and_parse(new_options);
  }

  data->all_slots_loss_report = all_slots_loss_report;
  if (!options.was_supplied("cb_explore_adf"))
  {
    options.insert("cb_explore_adf", "");
    options.add_and_parse(new_options);
  }

  if (options.was_supplied("no_predict") && options.was_supplied("p"))
  {
    THROW("Error: Cannot use flags --no_predict and -p simultaneously");
  }

  if (options.was_supplied("no_predict") && type_string != "mtr")
  {
    THROW("Error: --no_predict flag can only be used with default cb_type mtr");
  }

  if (!options.was_supplied("cb_sample") && !data->no_pred)
  {
    options.insert("cb_sample", "");
    options.add_and_parse(new_options);
  }

  auto base = require_multiline(stack_builder.setup_base_learner());
  all.example_parser->lbl_parser = VW::ccb_label_parser_global;

  // Stash the base learners stride_shift so we can properly add a feature
  // later.
  data->base_learner_stride_shift = all.weights.stride_shift();

  // Extract from lower level reductions
  data->shared = nullptr;
  data->all = &all;
  data->model_file_version = all.model_file_ver;

  data->id_namespace_str = "_id";
  data->id_namespace_audit_str = "_ccb_slot_index";
  data->id_namespace_hash = VW::hash_space(all, data->id_namespace_str);

  auto l = make_reduction_learner(std::move(data), base, learn_or_predict<true>, learn_or_predict<false>,
      stack_builder.get_setupfn_name(ccb_explore_adf_setup))
               .set_learn_returns_prediction(true)
               .set_input_prediction_type(VW::prediction_type_t::ACTION_PROBS)
               .set_output_prediction_type(VW::prediction_type_t::DECISION_PROBS)
               .set_input_label_type(VW::label_type_t::CCB)
               .set_output_label_type(VW::label_type_t::CB)
               .set_output_example_prediction(output_example_prediction_ccb)
               .set_print_update(::print_update_ccb)
               .set_update_stats(update_stats_ccb)
               .set_cleanup_example(cleanup_example_ccb)
               .set_save_load(save_load)
               .build();
  return l;
}

// CCB adds the following interactions:
//   1. Every existing interaction + VW::details::CCB_ID_NAMESPACE
//   2. wildcard_namespace + VW::details::CCB_ID_NAMESPACE
void VW::reductions::ccb::insert_ccb_interactions(std::vector<std::vector<VW::namespace_index>>& interactions_to_add_to,
    std::vector<std::vector<extent_term>>& extent_interactions_to_add_to)
{
  const auto reserve_size = interactions_to_add_to.size() * 2;
  std::vector<std::vector<VW::namespace_index>> new_interactions;
  new_interactions.reserve(reserve_size);
  for (const auto& inter : interactions_to_add_to)
  {
    new_interactions.push_back(inter);
    new_interactions.back().push_back(static_cast<VW::namespace_index>(VW::details::CCB_ID_NAMESPACE));
    new_interactions.push_back(inter);
  }
  interactions_to_add_to.reserve(interactions_to_add_to.size() + new_interactions.size() + 2);
  std::move(new_interactions.begin(), new_interactions.end(), std::back_inserter(interactions_to_add_to));
  interactions_to_add_to.push_back({VW::details::WILDCARD_NAMESPACE, VW::details::CCB_ID_NAMESPACE});

  std::vector<std::vector<extent_term>> new_extent_interactions;
  new_extent_interactions.reserve(new_extent_interactions.size() * 2);
  for (const auto& inter : extent_interactions_to_add_to)
  {
    new_extent_interactions.push_back(inter);
    new_extent_interactions.back().emplace_back(VW::details::CCB_ID_NAMESPACE, VW::details::CCB_ID_NAMESPACE);
    new_extent_interactions.push_back(inter);
  }
  extent_interactions_to_add_to.reserve(extent_interactions_to_add_to.size() + new_extent_interactions.size() + 2);
  std::move(new_extent_interactions.begin(), new_extent_interactions.end(),
      std::back_inserter(extent_interactions_to_add_to));
  extent_interactions_to_add_to.push_back(
      {std::make_pair(VW::details::WILDCARD_NAMESPACE, VW::details::WILDCARD_NAMESPACE),
          std::make_pair(VW::details::CCB_ID_NAMESPACE, VW::details::CCB_ID_NAMESPACE)});
}

bool VW::reductions::ccb::ec_is_example_header(VW::example const& ec)
{
  return ec.l.conditional_contextual_bandit.type == VW::ccb_example_type::SHARED;
}
bool VW::reductions::ccb::ec_is_example_unset(VW::example const& ec)
{
  return ec.l.conditional_contextual_bandit.type == VW::ccb_example_type::UNSET;
}

std::string VW::reductions::ccb::generate_ccb_label_printout(const VW::multi_ex& slots)
{
  size_t counter = 0;
  std::stringstream label_ss;
  std::string delim;
  for (const auto& slot : slots)
  {
    counter++;

    auto* outcome = slot->l.conditional_contextual_bandit.outcome;
    if (outcome == nullptr) { label_ss << delim << "?"; }
    else { label_ss << delim << outcome->probabilities[0].action << ":" << outcome->cost; }

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
