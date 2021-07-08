// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "namespaced_features.h"
#include "vw_exception.h"

using namespace VW;

namespace_index namespaced_features::get_index_for_hash(uint64_t hash) const
{
  auto it = std::find_if(_feature_groups.begin(), _feature_groups.end(),
      [hash](const details::namespaced_feature_group& group) {
    return group._hash == hash;
  });

  if (it == _feature_groups.end())
  {
#ifdef VW_NOEXCEPT
    return {};
#else
    THROW("No index found for hash: " << hash);
#endif
  }
  return it->_index;
}

std::pair<namespaced_features::indexed_iterator, namespaced_features::indexed_iterator>
namespaced_features::get_namespace_index_groups(namespace_index ns_index)
{
  return std::make_pair(namespace_index_begin(ns_index), namespace_index_end(ns_index));
}

std::pair<namespaced_features::const_indexed_iterator, namespaced_features::const_indexed_iterator>
namespaced_features::get_namespace_index_groups(namespace_index ns_index) const
{
  return std::make_pair(namespace_index_begin(ns_index), namespace_index_end(ns_index));
}


void namespaced_features::remove_feature_group(uint64_t hash)
{
  auto it = std::find_if(_feature_groups.begin(), _feature_groups.end(),
      [hash](const details::namespaced_feature_group& group) {
    return group._hash == hash;
  });
  if (it == _feature_groups.end()) { return; }

  auto existing_index = std::distance(_feature_groups.begin(), it);

  it->_features.clear();
  _saved_feature_groups.reclaim_object(std::move(it->_features));

  _feature_groups.erase(it);

  for (auto idx_it = _legacy_indices_existing.begin(); idx_it != _legacy_indices_existing.end();)
  {
    auto& index_vec = _legacy_indices_to_index_mapping[*idx_it];
    if (!index_vec.empty())
    {
      // Remove this index from ns_index mappings if it exists
      auto inner_it = std::find(index_vec.begin(), index_vec.end(), existing_index);
      if (inner_it != index_vec.end()) { index_vec.erase(inner_it); }

      // Shift down any index that came after this one.
      for (auto& idx : index_vec)
      {
        if (idx > existing_index) { idx -= 1; }
      }

      if (index_vec.empty()) { idx_it = _legacy_indices_existing.erase(idx_it); }
      else
      {
        ++idx_it;
      }
    }
  }
}

void namespaced_features::clear()
{
  for (auto& namespaced_feat_group : _feature_groups)
  {
    namespaced_feat_group._features.clear();
    _saved_feature_groups.reclaim_object(std::move(namespaced_feat_group._features));
  }
  _feature_groups.clear();
  for (auto idx : _legacy_indices_existing)
  {
    _legacy_indices_to_index_mapping[idx].clear();
  }
  _legacy_indices_existing.clear();
}

generic_range<namespaced_features::indexed_iterator> namespaced_features::namespace_index_range(
    namespace_index ns_index)
{
  return {namespace_index_begin(ns_index), namespace_index_end(ns_index)};
}

generic_range<namespaced_features::const_indexed_iterator> namespaced_features::namespace_index_range(
    namespace_index ns_index) const
{
  return {namespace_index_begin(ns_index), namespace_index_end(ns_index)};
}

VW::chained_proxy_iterator<namespaced_features::indexed_iterator, features::audit_iterator>
namespaced_features::namespace_index_begin_proxy(namespace_index ns_index)
{
  auto begin_it = namespace_index_begin(ns_index);
  auto end_it = namespace_index_end(ns_index);
  features::audit_iterator inner_it;
  // If the range is empty we must default construct the inner iterator as dereferencing an end pointer (What begin_it
  // is here) is not valid.
  if (begin_it == end_it)
  {
    inner_it = features::audit_iterator{};
  }
  else
  {
    --end_it;
    inner_it = (*begin_it).audit_begin();
  }
  // end_it always points to the last valid outer iterator instead of the actual end iterator of the outer collection.
  // This is because the end chained_proxy_iterator points to the end iterator of the last valid item of the outer
  // collection.
  return {begin_it, end_it, inner_it};
}

VW::chained_proxy_iterator<namespaced_features::indexed_iterator, features::audit_iterator>
namespaced_features::namespace_index_end_proxy(namespace_index ns_index)
{
  auto begin_it = namespace_index_begin(ns_index);
  auto end_it = namespace_index_end(ns_index);
  features::audit_iterator inner_it;
  if (begin_it == end_it)
  {
    inner_it = features::audit_iterator{};
  }
  else
  {
    --end_it;
    inner_it = (*end_it).audit_end();
  }

  return {end_it, end_it, inner_it};
}

VW::chained_proxy_iterator<namespaced_features::const_indexed_iterator, features::const_audit_iterator>
namespaced_features::namespace_index_begin_proxy(namespace_index ns_index) const
{
  auto begin_it = namespace_index_cbegin(ns_index);
  auto end_it = namespace_index_cend(ns_index);
  features::const_audit_iterator inner_it;
  if (begin_it == end_it)
  {
    inner_it = features::const_audit_iterator{};
  }
  else
  {
    --end_it;
    inner_it = (*begin_it).audit_cbegin();
  }
  return {begin_it, end_it, inner_it};
}

VW::chained_proxy_iterator<namespaced_features::const_indexed_iterator, features::const_audit_iterator>
namespaced_features::namespace_index_end_proxy(namespace_index ns_index) const
{
  auto begin_it = namespace_index_cbegin(ns_index);
  auto end_it = namespace_index_cend(ns_index);
  features::const_audit_iterator inner_it;
  if (begin_it == end_it)
  {
    inner_it = features::const_audit_iterator{};
  }
  else
  {
    --end_it;
    inner_it = (*end_it).audit_cend();
  }
  return {end_it, end_it, inner_it};
}

VW::chained_proxy_iterator<namespaced_features::const_indexed_iterator, features::const_audit_iterator>
namespaced_features::namespace_index_cbegin_proxy(namespace_index ns_index) const
{
  auto begin_it = namespace_index_cbegin(ns_index);
  auto end_it = namespace_index_cend(ns_index);

  features::const_audit_iterator inner_it;
  if (begin_it == end_it)
  {
    inner_it = features::const_audit_iterator{};
  }
  else
  {
    --end_it;
    inner_it = (*begin_it).audit_cbegin();
  }

  return {begin_it, end_it, inner_it};
}

VW::chained_proxy_iterator<namespaced_features::const_indexed_iterator, features::const_audit_iterator>
namespaced_features::namespace_index_cend_proxy(namespace_index ns_index) const
{
  auto begin_it = namespace_index_cbegin(ns_index);
  auto end_it = namespace_index_cend(ns_index);
  features::const_audit_iterator inner_it;
  if (begin_it == end_it)
  {
    inner_it = features::const_audit_iterator{};
  }
  else
  {
    --end_it;
    inner_it = (*end_it).audit_cend();
  }
  return {end_it, end_it, inner_it};
}

namespaced_features::indexed_iterator namespaced_features::namespace_index_begin(namespace_index ns_index)
{
  auto& index_vec = _legacy_indices_to_index_mapping[ns_index];
  return {index_vec.begin(), _feature_groups.begin()};
}

namespaced_features::indexed_iterator namespaced_features::namespace_index_end(namespace_index ns_index)
{
  auto& index_vec = _legacy_indices_to_index_mapping[ns_index];
  return {index_vec.end(), _feature_groups.begin()};
}

namespaced_features::const_indexed_iterator namespaced_features::namespace_index_begin(namespace_index ns_index) const
{
  auto& index_vec = _legacy_indices_to_index_mapping[ns_index];
  return {index_vec.cbegin(), _feature_groups.cbegin()};
}

namespaced_features::const_indexed_iterator namespaced_features::namespace_index_end(namespace_index ns_index) const
{
  auto& index_vec = _legacy_indices_to_index_mapping[ns_index];
  return {index_vec.cend(), _feature_groups.cbegin()};
}

namespaced_features::const_indexed_iterator namespaced_features::namespace_index_cbegin(namespace_index ns_index) const
{
  return namespace_index_begin(ns_index);
}

namespaced_features::const_indexed_iterator namespaced_features::namespace_index_cend(namespace_index ns_index) const
{
  return namespace_index_end(ns_index);
}
