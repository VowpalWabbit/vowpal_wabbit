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
struct namespaced_features;

struct namespaced_feature_group : public features
{
  features _features;
  uint64_t _hash;
  namespace_index _index;

  namespaced_feature_group(uint64_t hash, namespace_index index) : _hash(hash), _index(index) {}

  template <typename FeaturesT>
  namespaced_feature_group(FeaturesT&& inner_features, uint64_t hash, namespace_index index)
      : _features(std::forward<FeaturesT>(inner_features)), _hash(hash), _index(index)
  {
  }

  namespaced_feature_group(const namespaced_feature_group& other) = default;
  namespaced_feature_group& operator=(const namespaced_feature_group& other) = default;
  namespaced_feature_group(namespaced_feature_group&& other) = default;
  namespaced_feature_group& operator=(namespaced_feature_group&& other) = default;
};

using feature_group_map_t = std::map<uint64_t, namespaced_feature_group>;


/// Insertion or removal will result in this value in invalidated.
template <typename IteratorT, typename FeaturesT>
class iterator_t
{
  IteratorT _iterator;

public:
  iterator_t(IteratorT it)
      : _iterator(std::move(it))
  {
  }

  inline FeaturesT& operator*() { return _iterator->second._features; }
  inline iterator_t& operator++()
  {
    _iterator++;
    return *this;
  }

  inline namespace_index index() const
  {
    return _iterator->second._index;
  }
  inline uint64_t hash() const { return _iterator->second._hash; }

  bool operator==(const iterator_t& rhs) { return _iterator == rhs._iterator; }
  bool operator!=(const iterator_t& rhs) { return _iterator != rhs._iterator; }
};

/// Insertion or removal will result in this value in invalidated.
template <typename MapT, typename HashIteratorT, typename FeaturesT>
class indexed_iterator_t
{
  MapT* _feature_groups;
  HashIteratorT _hash_it;

public:
  using difference_type = std::ptrdiff_t;

  indexed_iterator_t(MapT* feature_groups, HashIteratorT hash_it)
      : _feature_groups(feature_groups)
      , _hash_it(std::move(hash_it))
  {
  }
  FeaturesT& operator*()
  {
    auto it = _feature_groups->find(*_hash_it);
    return it->second._features;
  }

  indexed_iterator_t& operator++()
  {
    _hash_it++;
    return *this;
  }

  indexed_iterator_t& operator--()
  {
    _hash_it--;
    return *this;
  }

  namespace_index index()
  {
    auto it = _feature_groups->find(*_hash_it);
    return it->second._index;
  }

  uint64_t hash()
  {
    auto it = _feature_groups->find(*_hash_it);
    return it->second._hash;
  }

  friend bool operator<(const indexed_iterator_t& lhs, const indexed_iterator_t& rhs)
  {
    return lhs._hash_it < rhs._hash_it;
  }

  friend bool operator>(const indexed_iterator_t& lhs, const indexed_iterator_t& rhs)
  {
    return lhs._hash_it > rhs._hash_it;
  }

  friend bool operator<=(const indexed_iterator_t& lhs, const indexed_iterator_t& rhs) { return !(lhs > rhs); }
  friend bool operator>=(const indexed_iterator_t& lhs, const indexed_iterator_t& rhs) { return !(lhs < rhs); }

  friend difference_type operator-(const indexed_iterator_t& lhs, const indexed_iterator_t& rhs)
  {
    assert(lhs._hash_it >= rhs._hash_it);
    return lhs._hash_it - rhs._hash_it;
  }

  bool operator==(const indexed_iterator_t& rhs) const { return _hash_it == rhs._hash_it; }
  bool operator!=(const indexed_iterator_t& rhs) const { return _hash_it != rhs._hash_it; }
};



/// namespace_index - 1 byte namespace identifier. Either the first character of the namespace or a reserved namespace
/// identifier namespace_hash - 8 byte hash
struct namespaced_features
{
  using iterator = iterator_t<feature_group_map_t::iterator, features>;
  using const_iterator = iterator_t<feature_group_map_t::const_iterator, const features>;
  using indexed_iterator = indexed_iterator_t<feature_group_map_t, std::vector<uint64_t>::iterator, features>;
  using const_indexed_iterator =
      indexed_iterator_t<const feature_group_map_t, std::vector<uint64_t>::const_iterator, const features>;

  namespaced_features() = default;
  ~namespaced_features() = default;
  namespaced_features(const namespaced_features& other) = default;
  namespaced_features& operator=(const namespaced_features& other) = default;
  namespaced_features(namespaced_features&& other) = default;
  namespaced_features& operator=(namespaced_features&& other) = default;

  inline size_t size() const { return _feature_groups.size(); }
  inline bool empty() const { return _feature_groups.empty(); }

  // Returns nullptr if not found.
  inline features* get_feature_group(uint64_t hash)
  {
    auto it = _feature_groups.find(hash);
    if (it == _feature_groups.end()) { return nullptr; }
    return &it->second._features;
  }
  // Returns nullptr if not found.
  inline const features* get_feature_group(uint64_t hash) const
  {
    auto it = _feature_groups.find(hash);
    if (it == _feature_groups.end()) { return nullptr; }
    return &it->second._features;
  }

  std::vector<namespace_index> get_indices() const;
  namespace_index get_index_for_hash(uint64_t hash) const;

  // The following are experimental and may be superseded with namespace_index_begin_proxy
  // Returns empty range if not found
  std::pair<indexed_iterator, indexed_iterator> get_namespace_index_groups(namespace_index ns_index);
  // Returns empty range if not found
  std::pair<const_indexed_iterator, const_indexed_iterator> get_namespace_index_groups(namespace_index ns_index) const;

  // If a feature group already exists in this "slot" it will be merged
  template <typename FeaturesT>
  features& merge_feature_group(FeaturesT&& ftrs, uint64_t hash, namespace_index ns_index);

  // If no feature group already exists a default one will be created.
  // Creating new feature groups will invalidate any pointers or references held.
  inline features& get_or_create_feature_group(uint64_t hash, namespace_index ns_index)
  {
    auto* existing_group = get_feature_group(hash);
    if (existing_group == nullptr)
    {
      // _saved_feature_groups
      auto pair_it = _feature_groups.emplace(hash, namespaced_feature_group{hash, ns_index});
      auto& added_feat_group = pair_it.first->second._features;
      _saved_feature_groups.acquire_object(added_feat_group);
      _legacy_indices_to_hash_mapping[ns_index].push_back(hash);
      return added_feat_group;
    }

    return *existing_group;
  }

  // This operation is only allowed in code that allows exceptions.
  // get_feature_group should be used instead for noexcept code
#ifndef VW_NOEXCEPT
  // These will throw if the hash does not exist
  inline const features& operator[](uint64_t hash) const
  {
    auto* existing_group = get_feature_group(hash);
    assert(existing_group != nullptr);
    return *existing_group;
  }
  inline features& operator[](uint64_t hash)
  {
    auto* existing_group = get_feature_group(hash);
    assert(existing_group != nullptr);
    return *existing_group;
  }
#endif

  // Removing a feature group will invalidate any pointers or references held.
  void remove_feature_group(uint64_t hash);

  void clear();

  // Experimental, hence the cumbersome names.
  // These iterators allow you to iterate over an entire namespace index as if it were a single feature group.
  VW::chained_proxy_iterator<indexed_iterator, features::audit_iterator> namespace_index_begin_proxy(
      namespace_index ns_index);
  VW::chained_proxy_iterator<indexed_iterator, features::audit_iterator> namespace_index_end_proxy(
      namespace_index ns_index);
  VW::chained_proxy_iterator<const_indexed_iterator, features::const_audit_iterator> namespace_index_begin_proxy(
      namespace_index ns_index) const;
  VW::chained_proxy_iterator<const_indexed_iterator, features::const_audit_iterator> namespace_index_end_proxy(
      namespace_index ns_index) const;
  VW::chained_proxy_iterator<const_indexed_iterator, features::const_audit_iterator> namespace_index_cbegin_proxy(
      namespace_index ns_index) const;
  VW::chained_proxy_iterator<const_indexed_iterator, features::const_audit_iterator> namespace_index_cend_proxy(
      namespace_index ns_index) const;

  // All of the following are experimental and may be superseded with the above proxies.
  generic_range<indexed_iterator> namespace_index_range(namespace_index ns_index);
  generic_range<const_indexed_iterator> namespace_index_range(namespace_index ns_index) const;
  indexed_iterator namespace_index_begin(namespace_index ns_index);
  indexed_iterator namespace_index_end(namespace_index ns_index);
  const_indexed_iterator namespace_index_begin(namespace_index ns_index) const;
  const_indexed_iterator namespace_index_end(namespace_index ns_index) const;
  const_indexed_iterator namespace_index_cbegin(namespace_index ns_index) const;
  const_indexed_iterator namespace_index_cend(namespace_index ns_index) const;


inline iterator begin()
{ return {_feature_groups.begin()}; }
inline iterator end()
{ return {_feature_groups.end()}; }
inline const_iterator begin() const
{
  return { _feature_groups.cbegin() };
}
inline const_iterator end() const
{ return {_feature_groups.cend()}; }
inline const_iterator cbegin() const
{ return {_feature_groups.cbegin()}; }
inline const_iterator cend() const
{ return {_feature_groups.cend()}; }


private:
  feature_group_map_t _feature_groups;
  std::map<namespace_index, std::vector<uint64_t>> _legacy_indices_to_hash_mapping;
  std::vector<uint64_t> _empty_vector_to_be_used_for_empty_iterators;
  VW::moved_object_pool<features> _saved_feature_groups;
};

// If a feature group already exists in this "slot" it will be merged
// Creating new feature groups will invalidate any pointers or references held.
template <typename FeaturesT>
features& namespaced_features::merge_feature_group(FeaturesT&& ftrs, uint64_t hash, namespace_index ns_index)
{
  auto* existing_group = get_feature_group(hash);
  if (existing_group == nullptr)
  {
    auto pair_it = _feature_groups.emplace(hash, namespaced_feature_group{std::forward<FeaturesT>(ftrs), hash, ns_index});
    _legacy_indices_to_hash_mapping[ns_index].push_back(hash);
    return pair_it.first->second._features;
  }
  else
  {
    existing_group->concat(std::forward<FeaturesT>(ftrs));
    // Should we ensure that this doesnt already exist under a DIFFERENT namespace_index?
    // However, his shouldn't be possible as ns_index depends on hash.
    auto& ns_indices_list = _legacy_indices_to_hash_mapping[ns_index];
    if (std::find(ns_indices_list.begin(), ns_indices_list.end(), ns_index) == ns_indices_list.end())
    { ns_indices_list.push_back(hash); }
  }
  return *existing_group;
}

}  // namespace VW
