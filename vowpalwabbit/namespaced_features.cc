// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "namespaced_features.h"
#include "vw_exception.h"

using namespace VW;

void namespaced_features::remove_feature_group(namespace_index ns_index, uint64_t hash)
{
  auto& bucket = _feature_groups[ns_index];
  auto it = std::find_if(bucket.begin(), bucket.end(),
      [hash](const details::namespaced_feature_group& group) {
    return group._hash == hash;
  });
  if (it == bucket.end()) { return; }
  it->_features.clear();
  _saved_feature_group_nodes.splice(_saved_feature_group_nodes.end(), _feature_groups[ns_index], it);
  if (bucket.empty()) {

    auto idx_it = std::find(_legacy_indices_existing.begin(), _legacy_indices_existing.end(), ns_index);
    if (idx_it != _legacy_indices_existing.end()) { _legacy_indices_existing.erase(idx_it);

    }
  }
}

void namespaced_features::clear()
{

  for (auto& ns_index : _legacy_indices_existing)
  {
    for (auto& namespaced_feat_group : _feature_groups[ns_index])
    {
      namespaced_feat_group._features.clear();
    }
    _saved_feature_group_nodes.splice(_saved_feature_group_nodes.end(), _feature_groups[ns_index]);
  }
  _legacy_indices_existing.clear();
}

VW::chained_proxy_iterator<namespaced_features::bucket_iterator, features::audit_iterator>
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
    inner_it = begin_it->_features.audit_begin();
  }
  // end_it always points to the last valid outer iterator instead of the actual end iterator of the outer collection.
  // This is because the end chained_proxy_iterator points to the end iterator of the last valid item of the outer
  // collection.
  return {begin_it, end_it, inner_it};
}

VW::chained_proxy_iterator<namespaced_features::bucket_iterator, features::audit_iterator>
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
    inner_it = end_it->_features.audit_end();
  }

  return {end_it, end_it, inner_it};
}

namespaced_features::bucket_iterator namespaced_features::namespace_index_begin(namespace_index ns_index)
{
  return _feature_groups[ns_index].begin();
}

namespaced_features::bucket_iterator namespaced_features::namespace_index_end(namespace_index ns_index)
{
  return _feature_groups[ns_index].end();
}

namespaced_features::const_bucket_iterator namespaced_features::namespace_index_begin(namespace_index ns_index) const
{
  return _feature_groups[ns_index].begin();
}

namespaced_features::const_bucket_iterator namespaced_features::namespace_index_end(namespace_index ns_index) const
{
  return _feature_groups[ns_index].end();
}

