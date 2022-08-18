// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "../automl_impl.h"
#include "vw/core/reductions/conditional_contextual_bandit.h"

namespace VW
{
namespace reductions
{
namespace automl
{
template <>
config_oracle<oracle_rand_impl>::config_oracle(uint64_t global_lease, priority_func* calc_priority,
    const std::string& interaction_type, const std::string& oracle_type, std::shared_ptr<VW::rand_state>& rand_state)
    : _interaction_type(interaction_type)
    , _oracle_type(oracle_type)
    , calc_priority(calc_priority)
    , global_lease(global_lease)
    , _impl(oracle_rand_impl(std::move(rand_state)))
{
}
template <typename oracle_impl>
config_oracle<oracle_impl>::config_oracle(uint64_t global_lease, priority_func* calc_priority,
    const std::string& interaction_type, const std::string& oracle_type, std::shared_ptr<VW::rand_state>&)
    : _interaction_type(interaction_type)
    , _oracle_type(oracle_type)
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

template <typename oracle_impl>
uint64_t config_oracle<oracle_impl>::choose(std::priority_queue<std::pair<float, uint64_t>>& index_queue)
{
  uint64_t ret = index_queue.top().second;
  index_queue.pop();
  return ret;
}

// This code is primarily borrowed from expand_quadratics_wildcard_interactions in
// interactions.cc. It will generate interactions with -q :: and exclude namespaces
// from the corresponding live_slot. This function can be swapped out depending on
// preference of how to generate interactions from a given set of exclusions.
// Transforms exclusions -> interactions expected by VW.
void exclusion_config::apply_config_to_interactions(const bool ccb_on,
    const std::map<namespace_index, uint64_t>& ns_counter, const std::string& interaction_type,
    const exclusion_config& config, interaction_vec_t& interactions)
{
  if (interaction_type == "quadratic")
  {
    if (!interactions.empty()) { interactions.clear(); }
    for (auto it = ns_counter.begin(); it != ns_counter.end(); ++it)
    {
      auto idx1 = (*it).first;
      for (auto jt = it; jt != ns_counter.end(); ++jt)
      {
        auto idx2 = (*jt).first;
        std::vector<namespace_index> idx{idx1, idx2};
        if (config.exclusions.find(idx) == config.exclusions.end()) { interactions.push_back({idx1, idx2}); }
      }
    }
  }
  else if (interaction_type == "cubic")
  {
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
          if (config.exclusions.find(idx) == config.exclusions.end()) { interactions.push_back({idx1, idx2, idx3}); }
        }
      }
    }
  }
  else
  {
    THROW("Unknown interaction type.");
  }

  if (ccb_on)
  {
    std::vector<std::vector<extent_term>> empty;
    ccb::insert_ccb_interactions(interactions, empty);
  }
}

// Helper function to insert new configs from oracle into map of configs as well as index_queue.
// Handles creating new config with exclusions or overwriting stale configs to avoid reallocation.
template <typename oracle_impl>
void config_oracle<oracle_impl>::insert_config(
    set_ns_list_t&& new_exclusions, const std::map<namespace_index, uint64_t>& ns_counter, bool allow_dups)
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
void oracle_rand_impl::gen_exclusion_config_at(const std::string& interaction_type,
    const interaction_vec_t& champ_interactions, const size_t, set_ns_list_t& new_exclusions)
{
  uint64_t rand_ind = static_cast<uint64_t>(random_state->get_and_update_random() * champ_interactions.size());
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
}
void one_diff_impl::gen_exclusion_config_at(const std::string& interaction_type,
    const interaction_vec_t& champ_interactions, const size_t num, set_ns_list_t::iterator& exclusion,
    set_ns_list_t& new_exclusions)
{
  // Add one exclusion (for each interaction)
  if (num < champ_interactions.size())
  {
    auto& interaction = champ_interactions[num];
    if (interaction_type == "quadratic")
    {
      namespace_index ns1 = interaction[0];
      namespace_index ns2 = interaction[1];
      if (is_allowed_to_remove(ns1) && is_allowed_to_remove(ns2))
      {
        std::vector<namespace_index> idx{ns1, ns2};
        new_exclusions.insert(idx);
      }
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
  }
  else
  {
    // Remove one exclusion (for each exclusion)
    new_exclusions.erase(*exclusion);
    ++exclusion;
  }
}

template <>
void config_oracle<one_diff_impl>::gen_exclusion_configs(
    const interaction_vec_t& champ_interactions, const std::map<namespace_index, uint64_t>& ns_counter)
{
  // we need this to stay constant bc insert might resize configs vector
  auto champ_excl = std::move(configs[0].exclusions);
  auto exclusion_it = champ_excl.begin();

  for (auto it = _impl.begin(); it < _impl.end(champ_interactions, champ_excl); ++it)
  {
    auto copy_champ = champ_excl;
    _impl.gen_exclusion_config_at(_interaction_type, champ_interactions, *it, exclusion_it, copy_champ);
    insert_config(std::move(copy_champ), ns_counter);
  }

  configs[0].exclusions = std::move(champ_excl);
}

template <>
void config_oracle<champdupe_impl>::gen_exclusion_configs(
    const interaction_vec_t&, const std::map<namespace_index, uint64_t>& ns_counter)
{
  for (auto it = _impl.begin(); it < _impl.end(); ++it)
  {
    auto copy_champ = configs[0].exclusions;
    insert_config(std::move(copy_champ), ns_counter, true);
  }
}

template <typename oracle_impl>
void config_oracle<oracle_impl>::gen_exclusion_configs(
    const interaction_vec_t& champ_interactions, const std::map<namespace_index, uint64_t>& ns_counter)
{
  for (auto it = _impl.begin(); it < _impl.end(); ++it)
  {
    auto copy_champ = configs[0].exclusions;
    _impl.gen_exclusion_config_at(_interaction_type, champ_interactions, *it, copy_champ);
    insert_config(std::move(copy_champ), ns_counter);
  }
}

// This function is triggered when all sets of interactions generated by the oracle have been tried and
// reached their lease. It will then add inactive configs (stored in the config manager) to the queue
// 'index_queue' which can be used to swap out live configs as they run out of lease. This functionality
// may be better within the oracle, which could generate better priorities for different configs based
// on ns_counter (which is updated as each example is processed)
template <typename oracle_impl>
bool config_oracle<oracle_impl>::repopulate_index_queue(const std::map<namespace_index, uint64_t>& ns_counter)
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