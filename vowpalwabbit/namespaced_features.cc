// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "namespaced_features.h"
#include "vw_exception.h"

using namespace VW;

std::vector<namespace_index> namespaced_features::get_indices() const
{
  std::vector<namespace_index> indices;
  for (auto it : _legacy_indices_to_hash_mapping)
  {
    if (!it.second.empty()) { indices.push_back(it.first); }
  }
  return indices;
}

namespace_index namespaced_features::get_index_for_hash(uint64_t hash) const
{
  auto it = _feature_groups.find(hash);
  if (it == _feature_groups.end())
  {
#ifdef VW_NOEXCEPT
    return {};
#else
    THROW("No index found for hash: " << hash);
#endif
  }

  return it->second._index;
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
  auto it = _feature_groups.find(hash);
  if (it == _feature_groups.end()) { return; }
  auto& feat_group_to_remove = it->second._features;
  feat_group_to_remove.clear();
  _saved_feature_groups.reclaim_object(std::move(feat_group_to_remove));
  _feature_groups.erase(it);
  for (auto& kv : _legacy_indices_to_hash_mapping)
  {
    auto& hash_vec = kv.second;
    // Remove this hash from ns_index mappings if it exists
    auto it = std::find(hash_vec.begin(), hash_vec.end(), hash);
    if (it != hash_vec.end()) { hash_vec.erase(it); }
  }
}

void namespaced_features::clear()
{
  for (auto& feat_group : _feature_groups)
  {
    feat_group.second.clear();
    _saved_feature_groups.reclaim_object(std::move(feat_group.second));
  }
  _feature_groups.clear();
  for (auto& item : _legacy_indices_to_hash_mapping) { item.second.clear(); }
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
    --end_it;
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
    --end_it;
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
    --end_it;
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
    --end_it;
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
    --end_it;
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
    --end_it;
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
  auto it = _legacy_indices_to_hash_mapping.find(ns_index);
  if (it == _legacy_indices_to_hash_mapping.end() || (*it).second.size() == 0)
  {
    return {&_feature_groups, _empty_vector_to_be_used_for_empty_iterators.begin()};
  }
  return {&_feature_groups, it->second.begin()};
}

namespaced_features::indexed_iterator namespaced_features::namespace_index_end(namespace_index ns_index)
{
  auto it = _legacy_indices_to_hash_mapping.find(ns_index);
  if (it == _legacy_indices_to_hash_mapping.end() || (*it).second.size() == 0)
  {
    return {&_feature_groups, _empty_vector_to_be_used_for_empty_iterators.end()};
  }
  return {&_feature_groups, it->second.end()};
}

namespaced_features::const_indexed_iterator namespaced_features::namespace_index_begin(namespace_index ns_index) const
{
  auto it = _legacy_indices_to_hash_mapping.find(ns_index);
  if (it == _legacy_indices_to_hash_mapping.end() || (*it).second.size() == 0)
  {
    return {&_feature_groups, _empty_vector_to_be_used_for_empty_iterators.begin()};
  }
  return {&_feature_groups, it->second.begin()};
}

namespaced_features::const_indexed_iterator namespaced_features::namespace_index_end(namespace_index ns_index) const
{
  auto it = _legacy_indices_to_hash_mapping.find(ns_index);
  if (it == _legacy_indices_to_hash_mapping.end() || (*it).second.size() == 0)
  {
    return {&_feature_groups, _empty_vector_to_be_used_for_empty_iterators.end()};
  }
  return {&_feature_groups, it->second.end()};
}

namespaced_features::const_indexed_iterator namespaced_features::namespace_index_cbegin(namespace_index ns_index) const
{
  return namespace_index_begin(ns_index);
}

namespaced_features::const_indexed_iterator namespaced_features::namespace_index_cend(namespace_index ns_index) const
{
  return namespace_index_end(ns_index);
}
