// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "../automl_impl.h"

namespace VW
{
namespace reductions
{
namespace automl
{
template <>
config_oracle<oracle_rand_impl>::config_oracle(uint64_t global_lease, priority_func* calc_priority,
    std::map<namespace_index, uint64_t>& ns_counter, const std::string& interaction_type,
    const std::string& oracle_type, std::shared_ptr<VW::rand_state>& rand_state)
    : _interaction_type(interaction_type)
    , _oracle_type(oracle_type)
    , ns_counter(ns_counter)
    , calc_priority(calc_priority)
    , global_lease(global_lease)
    , _impl(oracle_rand_impl(std::move(rand_state)))
{
}
template <typename oracle_impl>
config_oracle<oracle_impl>::config_oracle(uint64_t global_lease, priority_func* calc_priority,
    std::map<namespace_index, uint64_t>& ns_counter, const std::string& interaction_type,
    const std::string& oracle_type, std::shared_ptr<VW::rand_state>&)
    : _interaction_type(interaction_type)
    , _oracle_type(oracle_type)
    , ns_counter(ns_counter)
    , calc_priority(calc_priority)
    , global_lease(global_lease)
    , _impl(oracle_impl())
{
}
template <typename oracle_impl>
void config_oracle<oracle_impl>::insert_qcolcol()
{
  assert(valid_config_size == 0);
  configs.emplace_back(global_lease);
  ++valid_config_size;
}

// Helper function to insert new configs from oracle into map of configs as well as index_queue.
// Handles creating new config with exclusions or overwriting stale configs to avoid reallocation.
template <typename oracle_impl>
void config_oracle<oracle_impl>::insert_config(std::set<std::vector<namespace_index>>&& new_exclusions, bool allow_dups)
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

void oracle_rand_impl::gen_exclusion_configs(
    config_oracle<oracle_rand_impl>* co, estimator_vec_t& estimators, const uint64_t current_champ)
{
  auto& champ_interactions = estimators[current_champ].first.live_interactions;
  for (uint64_t i = 0; i < CONFIGS_PER_CHAMP_CHANGE; ++i)
  {
    uint64_t rand_ind = static_cast<uint64_t>(random_state->get_and_update_random() * champ_interactions.size());
    std::set<std::vector<namespace_index>> new_exclusions(
        co->configs[estimators[current_champ].first.config_index].exclusions);
    if (co->_interaction_type == "quadratic")
    {
      namespace_index ns1 = champ_interactions[rand_ind][0];
      namespace_index ns2 = champ_interactions[rand_ind][1];
      std::vector<namespace_index> idx{ns1, ns2};
      new_exclusions.insert(idx);
    }
    else if (co->_interaction_type == "cubic")
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
    co->insert_config(std::move(new_exclusions));
  }
}
void one_diff_impl::gen_exclusion_configs(
    config_oracle<one_diff_impl>* co, estimator_vec_t& estimators, const uint64_t current_champ)
{
  auto& champ_interactions = estimators[current_champ].first.live_interactions;
  // Add one exclusion (for each interaction)
  for (auto& interaction : champ_interactions)
  {
    std::set<std::vector<namespace_index>> new_exclusions(
        co->configs[estimators[current_champ].first.config_index].exclusions);
    if (co->_interaction_type == "quadratic")
    {
      namespace_index ns1 = interaction[0];
      namespace_index ns2 = interaction[1];
      if (is_allowed_to_remove(ns1) && is_allowed_to_remove(ns2))
      {
        std::vector<namespace_index> idx{ns1, ns2};
        new_exclusions.insert(idx);
      }
    }
    else if (co->_interaction_type == "cubic")
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
    co->insert_config(std::move(new_exclusions));
  }
  // Remove one exclusion (for each exclusion)
  for (auto& ns_pair : co->configs[estimators[current_champ].first.config_index].exclusions)
  {
    auto new_exclusions = co->configs[estimators[current_champ].first.config_index].exclusions;
    new_exclusions.erase(ns_pair);
    co->insert_config(std::move(new_exclusions));
  }
}
void champdupe_impl::gen_exclusion_configs(
    config_oracle<champdupe_impl>* co, estimator_vec_t& estimators, const uint64_t current_champ)
{
  for (uint64_t i = 0; co->configs.size() <= 2; ++i)
  {
    co->insert_config(
        std::set<std::vector<namespace_index>>(co->configs[estimators[current_champ].first.config_index].exclusions),
        true);
  }
}

// This will generate configs based on the current champ. These configs will be
// stored as 'exclusions.' The current design is to look at the interactions of
// the current champ and remove one interaction for each new config. The number
// of configs to generate per champ is hard-coded to 5 at the moment.
// TODO: Add logic to avoid duplicate configs (could be very costly)
template <typename oracle_impl>
void config_oracle<oracle_impl>::gen_exclusion_configs(estimator_vec_t& estimators, const uint64_t current_champ)
{
  _impl.gen_exclusion_configs(this, estimators, current_champ);
}

// This function is triggered when all sets of interactions generated by the oracle have been tried and
// reached their lease. It will then add inactive configs (stored in the config manager) to the queue
// 'index_queue' which can be used to swap out live configs as they run out of lease. This functionality
// may be better within the oracle, which could generate better priorities for different configs based
// on ns_counter (which is updated as each example is processed)
template <typename oracle_impl>
bool config_oracle<oracle_impl>::repopulate_index_queue()
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

template struct config_oracle<oracle_rand_impl>;
template struct config_oracle<one_diff_impl>;
template struct config_oracle<champdupe_impl>;

}  // namespace automl
}  // namespace reductions
}  // namespace VW