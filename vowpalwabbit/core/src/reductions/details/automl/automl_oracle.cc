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
    const std::string& interaction_type, const std::string& oracle_type, std::shared_ptr<VW::rand_state>& rand_state,
    config_type conf_type)
    : _interaction_type(interaction_type)
    , _oracle_type(oracle_type)
    , calc_priority(calc_priority)
    , global_lease(global_lease)
    , _impl(oracle_rand_impl(std::move(rand_state)))
{
  _conf_type = conf_type;
}
template <typename oracle_impl>
config_oracle<oracle_impl>::config_oracle(uint64_t global_lease, priority_func* calc_priority,
    const std::string& interaction_type, const std::string& oracle_type, std::shared_ptr<VW::rand_state>&,
    config_type conf_type)
    : _interaction_type(interaction_type)
    , _oracle_type(oracle_type)
    , calc_priority(calc_priority)
    , global_lease(global_lease)
    , _impl(oracle_impl())
{
  _conf_type = conf_type;
}
template <typename oracle_impl>
void config_oracle<oracle_impl>::insert_starting_configuration()
{
  assert(valid_config_size == 0);
  configs.emplace_back(set_ns_list_t(), global_lease, _conf_type);
  ++valid_config_size;
}
template <>
void config_oracle<champdupe_impl>::keep_best_two(uint64_t winner_config_index)
{
  std::swap(configs[0], configs[winner_config_index]);
  if (winner_config_index != 1) { std::swap(configs[1], configs[winner_config_index]); }

  configs[2].state = config_state::Inactive;
  // assert(configs[0].conf_type == config_type::Exclusion);
}
template <typename oracle_impl>
void config_oracle<oracle_impl>::keep_best_two(uint64_t winner_config_index)
{
  std::swap(configs[0], configs[winner_config_index]);
  if (winner_config_index != 1) { std::swap(configs[1], configs[winner_config_index]); }
  valid_config_size = 2;
}

template <typename oracle_impl>
uint64_t config_oracle<oracle_impl>::choose(std::priority_queue<std::pair<float, uint64_t>>& index_queue)
{
  uint64_t ret = index_queue.top().second;
  index_queue.pop();
  return ret;
}

interaction_vec_t ns_based_config::gen_quadratic_interactions(
    const std::map<namespace_index, uint64_t>& ns_counter, const set_ns_list_t& exclusions)
{
  interaction_vec_t interactions;
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
  return interactions;
}

interaction_vec_t ns_based_config::gen_cubic_interactions(
    const std::map<namespace_index, uint64_t>& ns_counter, const set_ns_list_t& exclusions)
{
  interaction_vec_t interactions;
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
  return interactions;
}

// This code is primarily borrowed from expand_quadratics_wildcard_interactions in
// interactions.cc. It will generate interactions with -q :: and exclude namespaces
// from the corresponding live_slot. This function can be swapped out depending on
// preference of how to generate interactions from a given set of namespaces lists.
// Transforms config -> interactions expected by VW.
void ns_based_config::apply_config_to_interactions(const bool ccb_on,
    const std::map<namespace_index, uint64_t>& ns_counter, const std::string& interaction_type,
    const ns_based_config& config, interaction_vec_t& interactions)
{
  if (config.conf_type == config_type::Exclusion)
  {
    if (interaction_type == "quadratic")
    {
      if (!interactions.empty()) { interactions.clear(); }
      interactions = gen_quadratic_interactions(ns_counter, config.elements);
    }
    else if (interaction_type == "cubic")
    {
      if (!interactions.empty()) { interactions.clear(); }
      interactions = gen_cubic_interactions(ns_counter, config.elements);
    }
    else
    {
      THROW("Unknown interaction type.");
    }
  }
  else if (config.conf_type == config_type::Interaction)
  {
    if (!interactions.empty()) { interactions.clear(); }
    interactions.reserve(config.elements.size());
    interactions.assign(config.elements.begin(), config.elements.end());
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
void config_oracle<oracle_impl>::insert_config(set_ns_list_t&& new_elements,
    const std::map<namespace_index, uint64_t>& ns_counter, VW::reductions::automl::config_type conf_type,
    bool allow_dups)
{
  // First check if config already exists
  if (!allow_dups)
  {
    for (size_t i = 0; i < configs.size(); ++i)
    {
      if (configs[i].elements == new_elements)
      {
        if (i < valid_config_size) { return; }
        else
        {
          configs[valid_config_size].reset(std::move(configs[i].elements), global_lease, conf_type);
          // TODO: do we have to push here to index_quue?
        }
      }
    }
  }

  // Note that configs are never actually cleared, but valid_config_size is set to 0 instead to denote that
  // configs have become stale. Here we try to write over stale configs with new configs, and if no stale
  // configs exist we'll generate a new one.
  if (valid_config_size < configs.size())
  { configs[valid_config_size].reset(std::move(new_elements), global_lease, conf_type); }
  else
  {
    configs.emplace_back(std::move(new_elements), global_lease, conf_type);
  }

  float priority = (*calc_priority)(configs[valid_config_size], ns_counter);
  index_queue.push(std::make_pair(priority, valid_config_size));
  ++valid_config_size;
}

// This will generate configs based on the current champ. These configs will be
// stored as a set of NS lists. The current design is to look at the interactions of
// the current champ and remove one interaction for each new config. The number
// of configs to generate per champ is hard-coded to 5 at the moment.
void oracle_rand_impl::gen_ns_groupings_at(const std::string& interaction_type,
    const interaction_vec_t& champ_interactions, const size_t, set_ns_list_t& new_elements, config_type)
{
  uint64_t rand_ind = static_cast<uint64_t>(random_state->get_and_update_random() * champ_interactions.size());
  if (interaction_type == "quadratic")
  {
    namespace_index ns1 = champ_interactions[rand_ind][0];
    namespace_index ns2 = champ_interactions[rand_ind][1];
    std::vector<namespace_index> idx{ns1, ns2};
    new_elements.insert(idx);
  }
  else if (interaction_type == "cubic")
  {
    namespace_index ns1 = champ_interactions[rand_ind][0];
    namespace_index ns2 = champ_interactions[rand_ind][1];
    namespace_index ns3 = champ_interactions[rand_ind][2];
    std::vector<namespace_index> idx{ns1, ns2, ns3};
    new_elements.insert(idx);
  }
  else
  {
    THROW("Unknown interaction type.");
  }
}
void one_diff_impl::gen_ns_groupings_at(const std::string& interaction_type,
    const interaction_vec_t& champ_interactions, const size_t num, set_ns_list_t::iterator& exclusion,
    set_ns_list_t& new_elements)
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
        new_elements.insert(idx);
      }
    }
    else if (interaction_type == "cubic")
    {
      namespace_index ns1 = interaction[0];
      namespace_index ns2 = interaction[1];
      namespace_index ns3 = interaction[2];
      if (is_allowed_to_remove(ns1) && is_allowed_to_remove(ns2) && is_allowed_to_remove(ns3))
      {
        std::vector<namespace_index> idx{ns1, ns2, ns3};
        new_elements.insert(idx);
      }
    }
    else
    {
      THROW("Unknown interaction type.");
    }
  }
  else
  {
    // Remove one exclusion (for each exclusion)
    new_elements.erase(*exclusion);
    ++exclusion;
  }
}

template <>
void config_oracle<one_diff_impl>::gen_configs(
    const interaction_vec_t& champ_interactions, const std::map<namespace_index, uint64_t>& ns_counter)
{
  // we need this to stay constant bc insert might resize configs vector
  auto champ_excl = std::move(configs[0].elements);
  auto exclusion_it = champ_excl.begin();

  for (auto it = _impl.begin(); it < _impl.end(champ_interactions, champ_excl); ++it)
  {
    auto copy_champ = champ_excl;
    _impl.gen_ns_groupings_at(_interaction_type, champ_interactions, *it, exclusion_it, copy_champ);
    insert_config(std::move(copy_champ), ns_counter, _conf_type);
  }

  configs[0].elements = std::move(champ_excl);
}

void one_diff_inclusion_impl::gen_ns_groupings_at(const std::string& interaction_type,
    const interaction_vec_t& all_interactions, const size_t num, set_ns_list_t& copy_champ)
{
  // Element does not exist, so add it
  if (copy_champ.find(all_interactions[num]) == copy_champ.end())
  {
    auto& interaction = all_interactions[num];
    if (interaction_type == "quadratic")
    {
      namespace_index ns1 = interaction[0];
      namespace_index ns2 = interaction[1];
      if (is_allowed_to_remove(ns1) && is_allowed_to_remove(ns2))
      {
        std::vector<namespace_index> idx{ns1, ns2};
        copy_champ.insert(idx);
      }
    }
    else if (interaction_type == "cubic")
    {
      namespace_index ns1 = interaction[0];
      namespace_index ns2 = interaction[1];
      namespace_index ns3 = interaction[2];
      if (is_allowed_to_remove(ns1) && is_allowed_to_remove(ns2) && is_allowed_to_remove(ns3))
      {
        std::vector<namespace_index> idx{ns1, ns2, ns3};
        copy_champ.insert(idx);
      }
    }
  }
  else  // Element does exist, so remove it
  {
    copy_champ.erase(all_interactions[num]);
  }
}

template <>
void config_oracle<one_diff_inclusion_impl>::gen_configs(
    const interaction_vec_t&, const std::map<namespace_index, uint64_t>& ns_counter)
{
  // we need this to stay constant bc insert might resize configs vector
  auto champ_incl = std::move(configs[0].elements);
  interaction_vec_t all_interactions = (_interaction_type == "quadratic")
      ? ns_based_config::gen_quadratic_interactions(ns_counter, {})
      : ns_based_config::gen_cubic_interactions(ns_counter, {});

  for (auto it = _impl.begin(); it < _impl.end(all_interactions); ++it)
  {
    auto copy_champ = champ_incl;
    _impl.gen_ns_groupings_at(_interaction_type, all_interactions, *it, copy_champ);
    insert_config(std::move(copy_champ), ns_counter, _conf_type);
  }

  configs[0].elements = std::move(champ_incl);
}

template <>
void config_oracle<champdupe_impl>::gen_configs(
    const interaction_vec_t&, const std::map<namespace_index, uint64_t>& ns_counter)
{
  if (configs.size() == 1)
  {
    auto current = 0;
    for (auto it = _impl.begin(); it < _impl.end(); ++it, ++current)
    {
      auto copy_champ = configs[0].elements;
      if (current % 2) { insert_config(std::move(copy_champ), ns_counter, _conf_type, true); }
      else
      {
        set_ns_list_t empty;
        insert_config(std::move(empty), ns_counter, config_type::Interaction, true);
      }
    }
  }
}

template <typename oracle_impl>
void config_oracle<oracle_impl>::gen_configs(
    const interaction_vec_t& champ_interactions, const std::map<namespace_index, uint64_t>& ns_counter)
{
  for (auto it = _impl.begin(); it < _impl.end(); ++it)
  {
    auto copy_champ = configs[0].elements;
    _impl.gen_ns_groupings_at(_interaction_type, champ_interactions, *it, copy_champ, _conf_type);
    insert_config(std::move(copy_champ), ns_counter, _conf_type);
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

template class config_oracle<oracle_rand_impl>;
template class config_oracle<one_diff_impl>;
template class config_oracle<champdupe_impl>;
template class config_oracle<one_diff_inclusion_impl>;

}  // namespace automl
}  // namespace reductions
}  // namespace VW