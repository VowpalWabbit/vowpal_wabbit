// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/automl/automl_impl.h"

#include "vw/common/vw_exception.h"
#include "vw/core/interactions.h"
#include "vw/core/metric_sink.h"
#include "vw/core/reductions/automl/automl_util.h"

/*
This reduction implements the ChaCha algorithm from page 5 of the following paper:
https://arxiv.org/pdf/2106.04815.pdf

There are two key differences between this implementation and the algorithm described
in the paper. First, the paper assumes all knowledge (of examples and namespaces) is
known at the start, whereas that information is gathered as we process examples in reality.
Second, this algorithm follows the pattern Schedule -> Update Champ -> Learn, whereas
the paper follows the pattern Schedule -> Learn -> Update Champ. This should not make a
functional difference. Nearly all of the variables and functions described in the
algorithm live in the config_manager struct. The following provides a translation of
terms used in the paper to this implementation:

Variables from ChaCha:
c_init -> 0: The first champ is initialized to the 0th indexed set of weights
b(budget) -> max_live_configs: The weights/configs to learn on
C -> current_champ: The weight index of the current champ
S -> configs: The entire set of challengers/configs generated so far
B(+ C) -> estimators: The live challengers (including the champ) with statistics

Functions from ChaCha:
ConfigOracle -> config_oracle(): Generates new configs based on champ
Schedule -> schedule(): Swaps out / manages live configs on each step
Schedule.Choose -> calc_priority(): Determines priority when selecting next live config
Predict/Incur Loss -> offset_learn(): Updates weights and learns on live configs
Better -> better(): Changes champ if challenger is better
Worse -> worse(): Removes challenger is much worse than champ

New Concepts:
The 'eligible_to_inactivate' flag within socred_config is an implementation of the median
function in Algorithm 2 of Chacha. Basically we set half (or some number with the
--priority_challengers flag) of the live configs as regular, and the other half as priority.
The champ is a priority config, but is used for predictions while the other configs are not.
When a priority config runs out of lease, its lease it doubled and it's able to keep running.
When a regular config runs out of lease, we first compare it's lower_bound to each of the
priority config's ips. If the lower bound is higher, we'll swap the priorities (priority
becomes regular, regular becomes priority), then double its lease and keep running. If the
lower_bound of the regular config can't beat the ips of any priority configs, then it is
swapped out for some new config on the index_queue. The idea is that some live slots should
be used for priority challengers to have more time to beat the champ, while others are used
to swap out and try new configs more consistently.
*/

namespace VW
{
namespace reductions
{
namespace automl
{
void aml_estimator::persist(
    metric_sink& metrics, const std::string& suffix, bool verbose, const std::string& interaction_type)
{
  VW::estimator_config::persist(metrics, suffix);
  metrics.set_uint("conf_idx" + suffix, config_index);
  if (verbose)
  {
    metrics.set_string(
        "interactions" + suffix, VW::reductions::interaction_vec_t_to_string(live_interactions, interaction_type));
  }
}

// config_manager is a state machine (config_manager_state) 'time' moves forward after a call into one_step()
// this can also be interpreted as a pre-learn() hook since it gets called by a learn() right before calling
// into its own base_learner.learn(). see learn_automl(...)
interaction_config_manager::interaction_config_manager(uint64_t global_lease, uint64_t max_live_configs,
    std::shared_ptr<VW::rand_state> rand_state, uint64_t priority_challengers, std::string interaction_type,
    std::string oracle_type, dense_parameters& weights, priority_func* calc_priority, double automl_significance_level,
    double automl_estimator_decay, VW::io::logger* logger, uint32_t& wpp, bool lb_trick)
    : global_lease(global_lease)
    , max_live_configs(max_live_configs)
    , random_state(std::move(rand_state))
    , priority_challengers(priority_challengers)
    , interaction_type(std::move(interaction_type))
    , oracle_type(std::move(oracle_type))
    , weights(weights)
    , calc_priority(calc_priority)
    , automl_significance_level(automl_significance_level)
    , automl_estimator_decay(automl_estimator_decay)
    , logger(logger)
    , wpp(wpp)
    , lb_trick(lb_trick)
{
  configs.emplace_back(global_lease);
  configs[0].state = VW::reductions::automl::config_state::Live;
  estimators.emplace_back(std::make_pair(aml_estimator(automl_significance_level, automl_estimator_decay),
      estimator_config(automl_significance_level, automl_estimator_decay)));
  ++valid_config_size;
}

// This code is primarily borrowed from expand_quadratics_wildcard_interactions in
// interactions.cc. It will generate interactions with -q :: and exclude namespaces
// from the corresponding live_slot. This function can be swapped out depending on
// preference of how to generate interactions from a given set of exclusions.
// Transforms exclusions -> interactions expected by VW.

void interaction_config_manager::gen_interactions(uint64_t live_slot)
{
  if (interaction_type == "quadratic")
  {
    auto& exclusions = configs[estimators[live_slot].first.config_index].exclusions;
    auto& interactions = estimators[live_slot].first.live_interactions;
    if (!interactions.empty()) { interactions.clear(); }
    for (auto it = ns_counter.begin(); it != ns_counter.end(); ++it)
    {
      auto idx1 = (*it).first;
      for (auto jt = it; jt != ns_counter.end(); ++jt)
      {
        auto idx2 = (*jt).first;
        std::vector<namespace_index> idx{idx1, idx2};
        if (exclusions.find(idx) == exclusions.end()) { interactions.push_back({idx1, idx2}); }
      }
    }
  }
  else if (interaction_type == "cubic")
  {
    auto& exclusions = configs[estimators[live_slot].first.config_index].exclusions;
    auto& interactions = estimators[live_slot].first.live_interactions;
    if (!interactions.empty()) { interactions.clear(); }
    for (auto it = ns_counter.begin(); it != ns_counter.end(); ++it)
    {
      auto idx1 = (*it).first;
      for (auto jt = it; jt != ns_counter.end(); ++jt)
      {
        auto idx2 = (*jt).first;
        for (auto kt = jt; kt != ns_counter.end(); ++kt)
        {
          auto idx3 = (*kt).first;
          std::vector<namespace_index> idx{idx1, idx2, idx3};
          if (exclusions.find(idx) == exclusions.end()) { interactions.push_back({idx1, idx2, idx3}); }
        }
      }
    }
  }
  else
  {
    THROW("Unknown interaction type.");
  }
}

void interaction_config_manager::clear_non_champ_weights()
{
  for (int64_t current_slot_index = 1; static_cast<size_t>(current_slot_index) < estimators.size();
       ++current_slot_index)
  { weights.clear_offset(current_slot_index, wpp); }
}

// This function will process an incoming multi_ex, update the namespace_counter,
// log if new namespaces are encountered, and regenerate interactions based on
// newly seen namespaces.
void interaction_config_manager::pre_process(const multi_ex& ecs)
{
  // Count all namepsace seen in current example
  bool new_ns_seen = false;
  for (const example* ex : ecs)
  {
    for (const auto& ns : ex->indices)
    {
      if (!INTERACTIONS::is_interaction_ns(ns)) { continue; }
      ns_counter[ns]++;
      if (ns_counter[ns] == 1) { new_ns_seen = true; }
    }
  }

  // Regenerate interactions if new namespaces are seen
  if (new_ns_seen)
  {
    for (uint64_t live_slot = 0; live_slot < estimators.size(); ++live_slot) { gen_interactions(live_slot); }
  }
}
// Helper function to insert new configs from oracle into map of configs as well as index_queue.
// Handles creating new config with exclusions or overwriting stale configs to avoid reallocation.
void interaction_config_manager::insert_config(std::set<std::vector<namespace_index>>&& new_exclusions, bool allow_dups)
{
  // First check if config already exists
  if (!allow_dups)
  {
    for (size_t i = 0; i < configs.size(); ++i)
    {
      if (configs[i].exclusions == new_exclusions)
      {
        if (i < valid_config_size) { return; }
        else
        {
          configs[valid_config_size].exclusions = std::move(configs[i].exclusions);
          configs[valid_config_size].lease = global_lease;
          configs[valid_config_size].state = VW::reductions::automl::config_state::New;
        }
      }
    }
  }

  // Note that configs are never actually cleared, but valid_config_size is set to 0 instead to denote that
  // configs have become stale. Here we try to write over stale configs with new configs, and if no stale
  // configs exist we'll generate a new one.
  if (valid_config_size < configs.size())
  {
    configs[valid_config_size].exclusions = std::move(new_exclusions);
    configs[valid_config_size].lease = global_lease;
    configs[valid_config_size].state = VW::reductions::automl::config_state::New;
  }
  else
  {
    configs.emplace_back(global_lease);
    configs[valid_config_size].exclusions = std::move(new_exclusions);
  }
  float priority = (*calc_priority)(configs[valid_config_size], ns_counter);
  index_queue.push(std::make_pair(priority, valid_config_size));
  ++valid_config_size;
}

// This will generate configs based on the current champ. These configs will be
// stored as 'exclusions.' The current design is to look at the interactions of
// the current champ and remove one interaction for each new config. The number
// of configs to generate per champ is hard-coded to 5 at the moment.
// TODO: Add logic to avoid duplicate configs (could be very costly)
void interaction_config_manager::config_oracle()
{
  auto& champ_interactions = estimators[current_champ].first.live_interactions;
  if (oracle_type == "rand")
  {
    for (uint64_t i = 0; i < CONFIGS_PER_CHAMP_CHANGE; ++i)
    {
      uint64_t rand_ind = static_cast<uint64_t>(random_state->get_and_update_random() * champ_interactions.size());
      std::set<std::vector<namespace_index>> new_exclusions(
          configs[estimators[current_champ].first.config_index].exclusions);
      if (interaction_type == "quadratic")
      {
        namespace_index ns1 = champ_interactions[rand_ind][0];
        namespace_index ns2 = champ_interactions[rand_ind][1];
        std::vector<namespace_index> idx{ns1, ns2};
        new_exclusions.insert(idx);
      }
      else if (interaction_type == "cubic")
      {
        namespace_index ns1 = champ_interactions[rand_ind][0];
        namespace_index ns2 = champ_interactions[rand_ind][1];
        namespace_index ns3 = champ_interactions[rand_ind][2];
        std::vector<namespace_index> idx{ns1, ns2, ns3};
        new_exclusions.insert(idx);
      }
      else
      {
        THROW("Unknown interaction type.");
      }
      insert_config(std::move(new_exclusions));
    }
  }
  /*
   * Example of one_diff oracle:
   * Say we have namespaces {a,b,c}, so all quadratic interactions are {aa, ab, ac, bb, bc, cc}. If the champ has
   * an exclusion set {aa, ab}, then the champs interactions would be {ac, bb, bc, cc}. We want to generate all
   * configs with a distance of 1 from the champ, meaning all sets with one less exclusion, and all sets with one
   * more exclusion. So the first part looks through the champs interaction set, and creates new configs which add
   * one of those to the current exclusion set (eg it will generate {aa, ab, ac}, {aa, ab, bb}, {aa, ab, bc},
   * {aa, ab, cc}). Then the second part will create all exclusion sets which remove one (eg {aa} and {ab}).
   */
  else if (oracle_type == "one_diff")
  {
    // Add one exclusion (for each interaction)
    for (auto& interaction : champ_interactions)
    {
      std::set<std::vector<namespace_index>> new_exclusions(
          configs[estimators[current_champ].first.config_index].exclusions);
      if (interaction_type == "quadratic")
      {
        namespace_index ns1 = interaction[0];
        namespace_index ns2 = interaction[1];
        std::vector<namespace_index> idx{ns1, ns2};
        new_exclusions.insert(idx);
      }
      else if (interaction_type == "cubic")
      {
        namespace_index ns1 = interaction[0];
        namespace_index ns2 = interaction[1];
        namespace_index ns3 = interaction[2];
        std::vector<namespace_index> idx{ns1, ns2, ns3};
        new_exclusions.insert(idx);
      }
      else
      {
        THROW("Unknown interaction type.");
      }
      insert_config(std::move(new_exclusions));
    }
    // Remove one exclusion (for each exclusion)
    for (auto& ns_pair : configs[estimators[current_champ].first.config_index].exclusions)
    {
      auto new_exclusions = configs[estimators[current_champ].first.config_index].exclusions;
      new_exclusions.erase(ns_pair);
      insert_config(std::move(new_exclusions));
    }
  }
  else if (oracle_type == "champdupe")
  {
    for (uint64_t i = 0; i < max_live_configs; ++i)
    {
      insert_config(
          std::set<std::vector<namespace_index>>(configs[estimators[current_champ].first.config_index].exclusions),
          true);
    }
  }
  else
  {
    THROW("Unknown oracle type.");
  }
}
// This function is triggered when all sets of interactions generated by the oracle have been tried and
// reached their lease. It will then add inactive configs (stored in the config manager) to the queue
// 'index_queue' which can be used to swap out live configs as they run out of lease. This functionality
// may be better within the oracle, which could generate better priorities for different configs based
// on ns_counter (which is updated as each example is processed)
bool interaction_config_manager::repopulate_index_queue()
{
  for (size_t i = 0; i < valid_config_size; ++i)
  {
    // Only re-add if not removed and not live
    if (configs[i].state == VW::reductions::automl::config_state::New ||
        configs[i].state == VW::reductions::automl::config_state::Inactive)
    {
      float priority = (*calc_priority)(configs[i], ns_counter);
      index_queue.push(std::make_pair(priority, i));
    }
  }
  return !index_queue.empty();
}

bool interaction_config_manager::swap_eligible_to_inactivate(uint64_t live_slot)
{
  for (uint64_t other_live_slot = 0; other_live_slot < estimators.size(); ++other_live_slot)
  {
    bool better = lb_trick
        ? estimators[live_slot].first.lower_bound() > (1.f - estimators[other_live_slot].first.lower_bound())
        : estimators[live_slot].first.lower_bound() > estimators[other_live_slot].first.upper_bound();
    if (!estimators[other_live_slot].first.eligible_to_inactivate && other_live_slot != current_champ && better)
    {
      estimators[live_slot].first.eligible_to_inactivate = false;
      estimators[other_live_slot].first.eligible_to_inactivate = true;
      return true;
    }
  }
  return false;
}

// This function defines the logic update the live configs' leases, and to swap out
// new configs when the lease runs out.
void interaction_config_manager::schedule()
{
  for (uint64_t live_slot = 0; live_slot < max_live_configs; ++live_slot)
  {
    bool need_new_estimator = estimators.size() <= live_slot;
    /*
    Scheduling a new live config is necessary in 3 cases:
    1. We have not reached the maximum number of live configs yet
    2. The current live config has been removed due to Chacha's worse function
    3. A config has reached its lease
    */
    if (need_new_estimator ||
        configs[estimators[live_slot].first.config_index].state == VW::reductions::automl::config_state::Removed ||
        estimators[live_slot].first.update_count >= configs[estimators[live_slot].first.config_index].lease)
    {
      // Double the lease check swap for eligible_to_inactivate configs
      if (!need_new_estimator &&
          configs[estimators[live_slot].first.config_index].state == VW::reductions::automl::config_state::Live)
      {
        configs[estimators[live_slot].first.config_index].lease *= 2;
        if (!estimators[live_slot].first.eligible_to_inactivate || swap_eligible_to_inactivate(live_slot)) { continue; }
      }
      // Skip over removed configs in index queue, and do nothing we we run out of eligible configs
      while (!index_queue.empty() &&
          configs[index_queue.top().second].state == VW::reductions::automl::config_state::Removed)
      { index_queue.pop(); }
      if (index_queue.empty() && !repopulate_index_queue()) { continue; }
      // Allocate new estimator if we haven't reached maximum yet
      if (need_new_estimator)
      {
        estimators.emplace_back(std::make_pair(aml_estimator(automl_significance_level, automl_estimator_decay),
            estimator_config(automl_significance_level, automl_estimator_decay)));
        if (live_slot > priority_challengers) { estimators.back().first.eligible_to_inactivate = true; }
      }
      // Only inactivate current config if lease is reached
      if (!need_new_estimator &&
          configs[estimators[live_slot].first.config_index].state == VW::reductions::automl::config_state::Live)
      { configs[estimators[live_slot].first.config_index].state = VW::reductions::automl::config_state::Inactive; }
      // Set all features of new live config
      estimators[live_slot].first.reset_stats(automl_significance_level, automl_estimator_decay);
      estimators[live_slot].second.reset_stats(automl_significance_level, automl_estimator_decay);
      uint64_t new_live_config_index = choose();
      estimators[live_slot].first.config_index = new_live_config_index;
      configs[new_live_config_index].state = VW::reductions::automl::config_state::Live;
      weights.move_offsets(current_champ, live_slot, wpp);
      // Regenerate interactions each time an exclusion is swapped in
      gen_interactions(live_slot);
      // We may also want to 0 out weights here? Currently keep all same in live_slot position
    }
  }
}

bool interaction_config_manager::better(uint64_t live_slot)
{
  return lb_trick ? estimators[live_slot].first.lower_bound() > (1.f - estimators[live_slot].second.lower_bound())
                  : estimators[live_slot].first.lower_bound() > estimators[live_slot].second.upper_bound();
}

bool interaction_config_manager::worse(uint64_t)
{
  // Dummy return false
  return false;
}

uint64_t interaction_config_manager::choose()
{
  uint64_t ret = index_queue.top().second;
  index_queue.pop();
  return ret;
}

void interaction_config_manager::update_champ()
{
  bool champ_change = false;
  uint64_t old_champ_slot = current_champ;
  uint64_t winning_challenger_slot = 0;

  // compare lowerbound of any challenger to the ips of the champ, and switch whenever when the LB beats the champ
  for (uint64_t live_slot = 0; live_slot < estimators.size(); ++live_slot)
  {
    if (live_slot == current_champ) { continue; }
    // If challenger is better ('better function from Chacha')
    if (better(live_slot))
    {
      champ_change = true;
      winning_challenger_slot = live_slot;
    }
    else if (worse(live_slot))  // If challenger is worse ('worse function from Chacha')
    {
      configs[estimators[live_slot].first.config_index].state = VW::reductions::automl::config_state::Removed;
    }
  }
  if (champ_change)
  {
    /*
     * Note here that the wining challenger (and its weights) will be moved into slot 0, and the old
     * champion will move into slot 1. All other weights are no longer relevant and will later take on the
     * champ's weights. The current champion will always be in slot 0, so if the winning challenger is in
     * slot 3 with 5 live models, the following operations will occur:
     * w0 w1 w2 w3 w4
     * w3 w1 w2 w0 w4 // w3 are the weights for the winning challenger (new champion) and placed in slot 0
     * w3 w0 w2 w0 w4 // w0 are the old champ's weights and place in slot 1, other weights are irrelevant
     */
    weights.move_offsets(winning_challenger_slot, old_champ_slot, wpp, true);
    if (winning_challenger_slot != 1) { weights.move_offsets(winning_challenger_slot, 1, wpp, false); }

    this->total_champ_switches++;
    while (!index_queue.empty()) { index_queue.pop(); };
    estimators[winning_challenger_slot].first.eligible_to_inactivate = false;
    if (priority_challengers > 1) { estimators[old_champ_slot].first.eligible_to_inactivate = false; }
    exclusion_config new_champ_config = configs[estimators[winning_challenger_slot].first.config_index];
    exclusion_config old_champ_config = configs[estimators[old_champ_slot].first.config_index];
    configs[0] = std::move(new_champ_config);
    configs[1] = std::move(old_champ_config);
    estimators[winning_challenger_slot].first.config_index = 0;
    estimators[old_champ_slot].first.config_index = 1;
    auto champ_estimator = std::move(estimators[winning_challenger_slot]);
    auto old_champ_estimator = std::move(estimators[old_champ_slot]);
    estimators.clear();
    estimators.push_back(std::move(champ_estimator));
    estimators.push_back(std::move(old_champ_estimator));
    current_champ = 0;
    valid_config_size = 2;

    /*
     * These operations rearrange the scoring data to sync up the new champion and old champion. Assume the first
     * challenger (chal_1) is taking over the champ (old_champ). The estimators would have this configuration before a
     * champ change: slot_0: <old_champ_dummy, old_champ_dummy_track_champ> slot_1: <chal_1, chal_1_track_champ> Note
     * that the estimators <old_champ_dummy, old_champ_dummy_track_champ> are not updated since the champion does not
     * need to track itself. After the champ change, the same estimators will look like this: slot_0: <chal_1,
     * chal_1_track_champ> slot_1: <old_champ_dummy, old_champ_dummy_track_champ> We then want to move the statistics
     * from chal_1_track_champ to old_champ_dummy and chal_1 to old_champ_dummy_track_champ since they both share the
     * same horizons, but have swapped which is champion and which is challenger. While we want to swap these
     * statistics, we don't want to alter the other state such as interactions and config_index.
     */
    estimators[1].first = aml_estimator(std::move(estimators[0].second), estimators[1].first.config_index,
        estimators[1].first.eligible_to_inactivate, estimators[1].first.live_interactions);
    estimators[1].second = estimators[0].first;
    if (lb_trick)
    {
      estimators[1].first.reset_stats();
      estimators[1].second.reset_stats();
    }
    config_oracle();
  }
}

void interaction_config_manager::persist(metric_sink& metrics, bool verbose)
{
  metrics.set_uint("test_county", total_learn_count);
  metrics.set_uint("current_champ", current_champ);
  for (uint64_t live_slot = 0; live_slot < estimators.size(); ++live_slot)
  {
    estimators[live_slot].first.persist(metrics, "_amls_" + std::to_string(live_slot), verbose, interaction_type);
    estimators[live_slot].second.persist(metrics, "_sc_" + std::to_string(live_slot));
    if (verbose)
    {
      auto& exclusions = configs[estimators[live_slot].first.config_index].exclusions;
      metrics.set_string("exclusionc_" + std::to_string(live_slot), VW::reductions::exclusions_to_string(exclusions));
    }
  }
  metrics.set_uint("total_champ_switches", total_champ_switches);
}

// This sets up example with correct ineractions vector
void interaction_config_manager::apply_config(example* ec, uint64_t live_slot)
{
  if (ec == nullptr) { return; }
  if (live_slot < max_live_configs) { ec->interactions = &(estimators[live_slot].first.live_interactions); }
  else
  {
    THROW("fatal (automl): trying to apply a config higher than max configs allowed");
  }
}

void interaction_config_manager::do_learning(multi_learner& base, multi_ex& ec, uint64_t live_slot)
{
  // TODO: what to do if that slot is switched with a new config?
  std::swap(*_gd_normalized, per_live_model_state_double[live_slot * 2]);
  std::swap(*_gd_total_weight, per_live_model_state_double[live_slot * 2 + 1]);
  std::swap(*_cb_adf_event_sum, per_live_model_state_uint64[live_slot * 2]);
  std::swap(*_cb_adf_action_sum, per_live_model_state_uint64[live_slot * 2 + 1]);
  for (example* ex : ec) { apply_config(ex, live_slot); }
  if (!base.learn_returns_prediction) { base.predict(ec, live_slot); }
  base.learn(ec, live_slot);
  std::swap(*_gd_normalized, per_live_model_state_double[live_slot * 2]);
  std::swap(*_gd_total_weight, per_live_model_state_double[live_slot * 2 + 1]);
  std::swap(*_cb_adf_event_sum, per_live_model_state_uint64[live_slot * 2]);
  std::swap(*_cb_adf_action_sum, per_live_model_state_uint64[live_slot * 2 + 1]);
}

}  // namespace automl
}  // namespace reductions
}  // namespace VW