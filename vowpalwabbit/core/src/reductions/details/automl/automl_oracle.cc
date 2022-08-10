// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "automl_impl.h"

namespace VW
{
namespace reductions
{
namespace automl
{
// Helper function to insert new configs from oracle into map of configs as well as index_queue.
// Handles creating new config with exclusions or overwriting stale configs to avoid reallocation.
void config_oracle::insert_config(std::set<std::vector<namespace_index>>&& new_exclusions, bool allow_dups)
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
void config_oracle::do_work(
    std::vector<std::pair<aml_estimator, estimator_config>>& estimators, const uint64_t current_champ)
{
  auto& champ_interactions = estimators[current_champ].first.live_interactions;
  if (_oracle_type == "rand")
  {
    for (uint64_t i = 0; i < CONFIGS_PER_CHAMP_CHANGE; ++i)
    {
      uint64_t rand_ind = static_cast<uint64_t>(random_state->get_and_update_random() * champ_interactions.size());
      std::set<std::vector<namespace_index>> new_exclusions(
          configs[estimators[current_champ].first.config_index].exclusions);
      if (_interaction_type == "quadratic")
      {
        namespace_index ns1 = champ_interactions[rand_ind][0];
        namespace_index ns2 = champ_interactions[rand_ind][1];
        std::vector<namespace_index> idx{ns1, ns2};
        new_exclusions.insert(idx);
      }
      else if (_interaction_type == "cubic")
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
  else if (_oracle_type == "one_diff")
  {
    // Add one exclusion (for each interaction)
    for (auto& interaction : champ_interactions)
    {
      std::set<std::vector<namespace_index>> new_exclusions(
          configs[estimators[current_champ].first.config_index].exclusions);
      if (_interaction_type == "quadratic")
      {
        namespace_index ns1 = interaction[0];
        namespace_index ns2 = interaction[1];
        if (is_allowed_to_remove(ns1) && is_allowed_to_remove(ns2))
        {
          std::vector<namespace_index> idx{ns1, ns2};
          new_exclusions.insert(idx);
        }
      }
      else if (_interaction_type == "cubic")
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
  else if (_oracle_type == "champdupe")
  {
    for (uint64_t i = 0; configs.size() <= 1; ++i)
    {
      insert_config(
          std::set<std::vector<namespace_index>>(configs[estimators[current_champ].first.config_index].exclusions),
          true);
    }
  }
  else
  {
    THROW("Unknown oracle type: " << _oracle_type);
  }
}
// This function is triggered when all sets of interactions generated by the oracle have been tried and
// reached their lease. It will then add inactive configs (stored in the config manager) to the queue
// 'index_queue' which can be used to swap out live configs as they run out of lease. This functionality
// may be better within the oracle, which could generate better priorities for different configs based
// on ns_counter (which is updated as each example is processed)
bool config_oracle::repopulate_index_queue()
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

}  // namespace automl
}  // namespace reductions
}  // namespace VW