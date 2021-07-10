// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <utility>
#include <vector>
#include <unordered_map>
#include <cassert>
#include <set>
#include <map>

#include "feature_group.h"
#include "generic_range.h"
#include "chained_proxy_iterator.h"
#include "object_pool.h"

typedef unsigned char namespace_index;



namespace VW
{
class indexed_iterator_t
{
  std::array<std::list<features>, 256>* _feature_group_buckets;
  std::vector<namespace_index>::iterator _indices_it;

public:
  using difference_type = std::ptrdiff_t;

  indexed_iterator_t(
      std::vector<namespace_index>::iterator  indices,
      std::array<std::list<features>, 256>* feature_groups)
      : _indices_it(indices)
      , _feature_group_buckets(feature_groups)
  {
  }

  std::list<features>& operator*()
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
struct namespaced_features
{
  using iterator = indexed_iterator_t;
  using bucket_iterator = std::list<features>::iterator;
  using const_bucket_iterator = std::list<features>::const_iterator;

  using ns_index_iterator = std::vector<namespace_index>::iterator;
  using const_ns_index_iterator = std::vector<namespace_index>::const_iterator;

  namespaced_features() = default;
  ~namespaced_features() = default;

  // For copies we deep copy the features but not the pool
  namespaced_features(const namespaced_features& other)
  {
    _feature_groups = other._feature_groups;
    _legacy_indices_existing = other._legacy_indices_existing;
  }
  namespaced_features& operator=(const namespaced_features& other)
  {
    _feature_groups = other._feature_groups;
    _legacy_indices_existing = other._legacy_indices_existing;
    return *this;
  }
  namespaced_features(namespaced_features&& other) = default;
  namespaced_features& operator=(namespaced_features&& other) = default;

  inline size_t size() const { size_t accumulator = 0;
    for (auto& ns_index : _legacy_indices_existing)
    {
      accumulator += _feature_groups[ns_index].size();
    }

    return accumulator;
  }
  inline bool empty() const { return _legacy_indices_existing.empty(); }

  inline features* get_feature_group(namespace_index hash) { return get_feature_group(hash, hash); }
  inline const features* get_feature_group(namespace_index hash) const { return get_feature_group(hash, hash); }

  // Returns nullptr if not found.
  inline features* get_feature_group(namespace_index ns_index, uint64_t hash)
  {
    auto& bucket = _feature_groups[ns_index];
    auto it = std::find_if(bucket.begin(), bucket.end(), [hash](const features& group) {
      return group._hash == hash;
    });
    if (it == bucket.end()) { return nullptr; }
    return &(*it);
  }
  // Returns nullptr if not found.
  inline const features* get_feature_group(namespace_index ns_index, uint64_t hash) const
  {
    auto& bucket = _feature_groups[ns_index];
    auto it = std::find_if(bucket.begin(), bucket.end(), [hash](const features& group) { return group._hash == hash; });
    if (it == bucket.end()) { return nullptr; }
    return &(*it);
  }

  inline features& at(namespace_index ns_index) { return at(ns_index, ns_index);
  }

  inline const features& at(namespace_index ns_index) const
  { return at(ns_index, ns_index);
  }

  inline features& at(namespace_index ns_index, uint64_t hash)
  {
    auto* fs = get_feature_group(ns_index, hash);
    assert(fs != nullptr);
    return *fs;
  }

  inline const features& at(namespace_index ns_index, uint64_t hash) const
  {
    auto* fs = get_feature_group(ns_index, hash);
    assert(fs != nullptr);
    return *fs;
  }

  // If no feature group already exists a default one will be created.
  // Creating new feature groups will invalidate any pointers or references held.
  inline features& get_or_create_feature_group(uint64_t hash, namespace_index ns_index)
  {
    auto* existing_group = get_feature_group(ns_index, hash);
    if (existing_group == nullptr)
    {
      _feature_groups[ns_index].emplace_back(_saved_feature_groups.take_back());
      _saved_feature_groups.pop_back();
      auto& current = _feature_groups[ns_index].back();
      current._hash = hash;
      current._index = ns_index;
      if (_feature_groups[ns_index].size() == 1) { _legacy_indices_existing.push_back(ns_index); }
      return _feature_groups[ns_index].back();
    }

    return *existing_group;
  }

  // Removing a feature group will invalidate any pointers or references held.
  void remove_feature_group(namespace_index ns_index, uint64_t hash);

  void clear();

  // Experimental, hence the cumbersome names.
  // These iterators allow you to iterate over an entire namespace index as if it were a single feature group.
  VW::chained_proxy_iterator<bucket_iterator, features::audit_iterator> namespace_index_begin_proxy(
      namespace_index ns_index);
  VW::chained_proxy_iterator<bucket_iterator, features::audit_iterator> namespace_index_end_proxy(
      namespace_index ns_index);

  // All of the following are experimental and may be superseded with the above proxies.
  bucket_iterator namespace_index_begin(namespace_index ns_index);
  bucket_iterator namespace_index_end(namespace_index ns_index);
  const_bucket_iterator namespace_index_begin(namespace_index ns_index) const;
  const_bucket_iterator namespace_index_end(namespace_index ns_index) const;
  const_bucket_iterator namespace_index_cbegin(namespace_index ns_index) const;
  const_bucket_iterator namespace_index_cend(namespace_index ns_index) const;

  // Wil contains duplicates if there exists more than one feature group per index.
  inline ns_index_iterator index_begin()
  {
    return _legacy_indices_existing.begin();
  }

  inline ns_index_iterator index_end()
  {
    return _legacy_indices_existing.end();
  }

  inline iterator begin() { return {_legacy_indices_existing.begin(), &_feature_groups};
  }
  inline iterator end()
  {
      return {_legacy_indices_existing.end(), &_feature_groups};
  }

private:
  std::array<std::list<features>, 256> _feature_groups;
  std::vector<namespace_index> _legacy_indices_existing;
  VW::moved_object_pool<features> _saved_feature_groups;
};

}  // namespace VW
