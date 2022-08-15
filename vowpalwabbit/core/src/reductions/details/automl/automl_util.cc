// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "../automl_impl.h"
#include "vw/core/interactions.h"
#include "vw/core/reductions/conditional_contextual_bandit.h"
#include "vw/core/vw.h"

namespace VW
{
namespace reductions
{
namespace automl
{
// This code is primarily borrowed from expand_quadratics_wildcard_interactions in
// interactions.cc. It will generate interactions with -q :: and exclude namespaces
// from the corresponding live_slot. This function can be swapped out depending on
// preference of how to generate interactions from a given set of exclusions.
// Transforms exclusions -> interactions expected by VW.
void gen_interactions_from_exclusions(bool ccb_on, std::map<namespace_index, uint64_t>& ns_counter,
    std::string& interaction_type, std::vector<exclusion_config>& configs, estimator_vec_t& estimators,
    uint64_t live_slot)
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

  if (ccb_on)
  {
    std::vector<std::vector<extent_term>> empty;
    auto& interactions = estimators[live_slot].first.live_interactions;
    ccb::insert_ccb_interactions(interactions, empty);
  }
}

bool worse()
{
  // Dummy return false
  return false;
}

// This sets up example with correct ineractions vector
void apply_config(example* ec, interaction_vec_t* live_interactions)
{
  if (ec == nullptr) { return; }
  ec->interactions = live_interactions;
}

// This function will process an incoming multi_ex, update the namespace_counter,
// log if new namespaces are encountered, and regenerate interactions based on
// newly seen namespaces.
bool count_namespaces(const multi_ex& ecs, std::map<namespace_index, uint64_t>& ns_counter)
{
  // Count all namepsace seen in current example
  bool new_ns_seen = false;
  for (const example* ex : ecs)
  {
    for (const auto& ns : ex->indices)
    {
      if (!INTERACTIONS::is_interaction_ns(ns)) { continue; }
      if (!is_allowed_to_remove(ns)) { continue; }
      ns_counter[ns]++;
      if (ns_counter[ns] == 1) { new_ns_seen = true; }
    }
  }

  return new_ns_seen;
}

bool is_allowed_to_remove(const unsigned char ns)
{
  if (ns == ccb_slot_namespace || ns == ccb_id_namespace) { return false; }
  return true;
}

void clear_non_champ_weights(dense_parameters& weights, uint32_t total, uint32_t& wpp)
{
  for (int64_t current_slot_index = 1; static_cast<size_t>(current_slot_index) < total; ++current_slot_index)
  { weights.clear_offset(current_slot_index, wpp); }
}
}  // namespace automl

namespace util
{
// fail if incompatible reductions got setup
// todo: audit if they reference global all interactions
void fail_if_enabled(VW::workspace& all, const std::set<std::string>& not_compat)
{
  std::vector<std::string> enabled_reductions;
  if (all.l != nullptr) { all.l->get_enabled_reductions(enabled_reductions); }

  for (const auto& reduction : enabled_reductions)
  {
    if (not_compat.count(reduction) > 0) THROW("automl does not yet support this reduction: " + reduction);
  }
}

std::string ns_to_str(unsigned char ns)
{
  if (ns == constant_namespace) { return "[constant]"; }
  else if (ns == ccb_slot_namespace)
  {
    return "[ccbslot]";
  }
  else if (ns == ccb_id_namespace)
  {
    return "[ccbid]";
  }
  else if (ns == wildcard_namespace)
  {
    return "[wild]";
  }
  else if (ns == default_namespace)
  {
    return "[default]";
  }
  else
  {
    return std::string(1, ns);
  }
}

std::string interaction_vec_t_to_string(
    const std::vector<std::vector<namespace_index>>& interactions, const std::string& interaction_type)
{
  std::stringstream ss;
  for (const std::vector<VW::namespace_index>& v : interactions)
  {
    interaction_type == "quadratic" ? ss << "-q " : ss << "-cubic ";
    for (VW::namespace_index c : v) { ss << ns_to_str(c); }
    ss << " ";
  }
  return ss.str();
}

std::string exclusions_to_string(const std::set<std::vector<VW::namespace_index>>& exclusions)
{
  const char* const delim = ", ";
  std::stringstream ss;
  size_t total = exclusions.size();
  size_t count = 0;
  ss << "{";
  for (auto const& x : exclusions)
  {
    ss << "[";
    if (!x.empty())
    {
      auto i = x.begin(), x_last = std::prev(x.end());
      for (; i != x_last; ++i) { ss << "\"" << ns_to_str(*i) << "\"" << delim; }
      ss << "\"" << ns_to_str(*x_last) << "\"";
    }
    count += 1;
    ss << "]";
    if (count < total) { ss << delim; }
  }
  ss << "}";
  return ss.str();
}
}  // namespace util
}  // namespace reductions
}  // namespace VW
