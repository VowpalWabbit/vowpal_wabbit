// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/automl/automl.h"
#include "vw/core/vw.h"

namespace VW
{
namespace reductions
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
    const VW::reductions::automl::interaction_vec_t& interactions, const std::string& interaction_type)
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
}  // namespace reductions
}  // namespace VW
