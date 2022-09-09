// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "../automl_impl.h"
#include "vw/core/interactions.h"
#include "vw/core/vw.h"

namespace VW
{
namespace reductions
{
namespace automl
{
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

bool is_allowed_to_remove(const namespace_index ns)
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

std::string elements_to_string(const automl::set_ns_list_t& elements)
{
  const char* const delim = ", ";
  std::stringstream ss;
  size_t total = elements.size();
  size_t count = 0;
  ss << "{";
  for (auto const& x : elements)
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
