// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/automl_impl.h"
#include "vw/core/interactions.h"
#include "vw/core/multi_model_utils.h"
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
      if (!VW::is_interaction_ns(ns)) { continue; }
      if (!is_allowed_to_remove(ns)) { continue; }
      ns_counter[ns]++;
      if (ns_counter[ns] == 1) { new_ns_seen = true; }
    }
  }

  return new_ns_seen;
}

bool is_allowed_to_remove(const namespace_index ns)
{
  if (ns == VW::details::CCB_SLOT_NAMESPACE || ns == VW::details::CCB_ID_NAMESPACE) { return false; }
  return true;
}
}  // namespace automl

namespace util
{
// fail if incompatible reductions got setup
// todo: audit if they reference global all interactions
void fail_if_enabled(VW::workspace& all, const std::set<std::string>& not_compat)
{
  std::vector<std::string> enabled_learners;
  if (all.l != nullptr) { all.l->get_enabled_learners(enabled_learners); }

  for (const auto& reduction : enabled_learners)
  {
    if (not_compat.count(reduction) > 0) THROW("automl does not yet support this reduction: " + reduction);
  }
}

std::string ns_to_str(unsigned char ns)
{
  if (ns == VW::details::CONSTANT_NAMESPACE) { return "[constant]"; }
  else if (ns == VW::details::CCB_SLOT_NAMESPACE) { return "[ccbslot]"; }
  else if (ns == VW::details::CCB_ID_NAMESPACE) { return "[ccbid]"; }
  else if (ns == VW::details::WILDCARD_NAMESPACE) { return "[wild]"; }
  else if (ns == VW::details::DEFAULT_NAMESPACE) { return "[default]"; }
  else { return std::string(1, ns); }
}

std::string interaction_vec_t_to_string(const std::vector<std::vector<namespace_index>>& interactions)
{
  std::stringstream ss;
  for (const std::vector<VW::namespace_index>& v : interactions)
  {
    if (v.size() == 2) { ss << "-q "; }
    else if (v.size() == 3) { ss << "--cubic="; }
    else { THROW("Only supports up to cubic interactions"); }
    for (VW::namespace_index c : v) { ss << ns_to_str(c); }
    ss << " ";
  }
  return ss.str();
}

std::string elements_to_string(const automl::set_ns_list_t& elements, const char* const delim)
{
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
