// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <cassert>
#include <list>
#include <array>
#include <vector>

#include "feature_group.h"
#include "chained_proxy_iterator.h"

typedef unsigned char namespace_index;

namespace VW
{
class indexed_iterator_t
{
  std::array<std::list<namespaced_features>, 256>* _feature_group_buckets;
  std::vector<namespace_index>::iterator _indices_it;

public:
  using difference_type = std::ptrdiff_t;

  indexed_iterator_t(
      std::vector<namespace_index>::iterator  indices,
      std::array<std::list<namespaced_features>, 256>* feature_groups)
      : _indices_it(indices)
      , _feature_group_buckets(feature_groups)
  {
  }

  std::list<namespaced_features>& operator*()
  {
    return (*_feature_group_buckets)[*_indices_it];
  }

  indexed_iterator_t& operator++()
  {
    _indices_it++;
    return *this;
  }

  indexed_iterator_t& operator--()
  {
    --_indices_it;
    return *this;
  }

  friend bool operator<(const indexed_iterator_t& lhs, const indexed_iterator_t& rhs)
  {
    return lhs._indices_it < rhs._indices_it;
  }

  friend bool operator>(const indexed_iterator_t& lhs, const indexed_iterator_t& rhs)
  {
    return lhs._indices_it > rhs._indices_it;
  }

  friend bool operator<=(const indexed_iterator_t& lhs, const indexed_iterator_t& rhs) { return !(lhs > rhs); }
  friend bool operator>=(const indexed_iterator_t& lhs, const indexed_iterator_t& rhs) { return !(lhs < rhs); }

  friend difference_type operator-(const indexed_iterator_t& lhs, const indexed_iterator_t& rhs)
  {
    assert(lhs._indices_it >= rhs._indices_it);
    return lhs._indices_it - rhs._indices_it;
  }

  bool operator==(const indexed_iterator_t& rhs) const { return _indices_it == rhs._indices_it; }
  bool operator!=(const indexed_iterator_t& rhs) const { return _indices_it != rhs._indices_it; }
};

/// namespace_index - 1 byte namespace identifier. Either the first character of the namespace or a reserved namespace
/// identifier namespace_hash - 8 byte hash
struct namespaced_feature_store
{
  using iterator = indexed_iterator_t;
  using list_iterator = std::list<namespaced_features>::iterator;
  using const_list_iterator = std::list<namespaced_features>::const_iterator;
  using index_flat_iterator = VW::chained_proxy_iterator<list_iterator, features::audit_iterator>;

  namespaced_feature_store() = default;
  ~namespaced_feature_store() = default;

  // For copies we deep copy the features but not the pool
  namespaced_feature_store(const namespaced_feature_store& other)
  {
    _feature_groups = other._feature_groups;
    _legacy_indices_existing = other._legacy_indices_existing;
  }

  namespaced_feature_store& operator=(const namespaced_feature_store& other)
  {
    _feature_groups = other._feature_groups;
    _legacy_indices_existing = other._legacy_indices_existing;
    return *this;
  }
  namespaced_feature_store(namespaced_feature_store&& other) = default;
  namespaced_feature_store& operator=(namespaced_feature_store&& other) = default;

  inline size_t size() const {
    size_t accumulator = 0;
    for (auto ns_index : _legacy_indices_existing)
    {
      accumulator += _feature_groups[ns_index].size();
    }
    return accumulator;
  }
  inline bool empty() const { return _legacy_indices_existing.empty(); }

  // Returns nullptr if not found.
  inline features* get_or_null(namespace_index ns_index, uint64_t hash)
  {
    auto& bucket = _feature_groups[ns_index];
    auto it = std::find_if(bucket.begin(), bucket.end(), [hash](const namespaced_features& group) {
      return group.hash == hash;
    });
    if (it == bucket.end()) { return nullptr; }
    return &it->features;
  }

  // Returns nullptr if not found.
  inline const features* get_or_null(namespace_index ns_index, uint64_t hash) const
  {
    auto& bucket = _feature_groups[ns_index];
    auto it = std::find_if(bucket.begin(), bucket.end(), [hash](const namespaced_features& group) { return group.hash == hash; });
    if (it == bucket.end()) { return nullptr; }
    return &it->features;
  }

  inline features& get(namespace_index ns_index, uint64_t hash)
  {
    auto* fs = get_or_null(ns_index, hash);
    assert(fs != nullptr);
    return *fs;
  }

  inline const features& get(namespace_index ns_index, uint64_t hash) const
  {
    auto* fs = get_or_null(ns_index, hash);
    assert(fs != nullptr);
    return *fs;
  }

  // If no feature group already exists a default one will be created.
  // Creating new feature groups will invalidate any pointers or references held.
  features& get_or_create(namespace_index ns_index, uint64_t hash);

  // UB if the list itself is modified.
  inline std::list<namespaced_features>& get_list(namespace_index index)
  {
    return _feature_groups[index];
  }

  inline const std::list<namespaced_features>& get_list(namespace_index index) const
  {
    return _feature_groups[index];
  }

  /// Will invalidate any iterator to the removed namespaced_features. Use the overload which returns an iterator if removing while traversing a list.
  void remove(namespace_index ns_index, uint64_t hash);

  void clear();

  index_flat_iterator index_flat_begin(namespace_index ns_index);
  index_flat_iterator index_flat_end(namespace_index ns_index);

  const std::vector<namespace_index>& indices() const { return _legacy_indices_existing; }

  // UB if the index or hash is modified.
  // UB if the list itself is modified.
  inline iterator begin() { return {_legacy_indices_existing.begin(), &_feature_groups}; }

  // UB if the index or hash is modified.
  // UB if the list itself is modified.
  inline iterator end()
  {
      return {_legacy_indices_existing.end(), &_feature_groups};
  }

private:
  std::array<std::list<namespaced_features>, 256> _feature_groups;
  std::vector<namespace_index> _legacy_indices_existing;
  std::list<namespaced_features> _saved_feature_group_nodes;
};


// UB if the index or hash is modified.
 template <typename FuncT>
inline void foreach (namespaced_feature_store& store, FuncT func)
{
  for (const auto& ns_index : _legacy_indices_existing)
  {
    for (auto& namespaced_feat_group : _feature_groups[ns_index])
    {
#ifndef NDEBUG
      const auto prev_index = namespaced_feat_group.index;
      const auto prev_hash = namespaced_feat_group.hash;
#endif

      func(namespaced_feat_group.index, namespaced_feat_group.hash, namespaced_feat_group.features);

      // Callers of this function should never change the index or hash values
#ifndef NDEBUG
      assert(prev_index == namespaced_feat_group.index);
      assert(prev_hash == namespaced_feat_group.hash);
#endif
    }
  }
}

template <typename FuncT>
inline void foreach (const namespaced_feature_store& store, FuncT func)
{
  for (const auto& ns_index : _legacy_indices_existing)
  {
    for (const auto& namespaced_feat_group : _feature_groups[ns_index])
    {
      func(namespaced_feat_group.index, namespaced_feat_group.hash, namespaced_feat_group.features);
    }
  }
}

}  // namespace VW
