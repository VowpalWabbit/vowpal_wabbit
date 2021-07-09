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

namespace details
{
  struct namespaced_feature_group
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
}

/// Insertion or removal will result in this value in invalidated.
template <typename FeatureGroupIteratorT, typename FeaturesT>
class iterator_t
{
  FeatureGroupIteratorT _feature_group_iterator;

public:
  iterator_t(FeatureGroupIteratorT feature_group_iterator)
      : _feature_group_iterator(feature_group_iterator)
  {
  }

  inline FeaturesT& operator*() { return _feature_group_iterator->_features; }
  inline iterator_t& operator++()
  {
    _feature_group_iterator++;
    return *this;
  }

  inline namespace_index index() { return _feature_group_iterator->_index; }
  inline uint64_t hash() { return _feature_group_iterator->_hash; }

  bool operator==(const iterator_t& rhs) { return _feature_group_iterator == rhs._feature_group_iterator; }
  bool operator!=(const iterator_t& rhs) { return _feature_group_iterator != rhs._feature_group_iterator; }
};

template <typename FeatureGroupIteratorT>
class index_iterator_t
{
  FeatureGroupIteratorT _feature_group_iterator;

public:
  using difference_type = std::ptrdiff_t;
  using value_type = namespace_index;
  using pointer = value_type*;
  using reference = value_type&;
  using iterator_category = std::forward_iterator_tag;

  index_iterator_t(FeatureGroupIteratorT feature_group_iterator)
      : _feature_group_iterator(feature_group_iterator)
  {
  }

  inline value_type operator*() { return _feature_group_iterator->_index; }
  inline index_iterator_t& operator++()
  {
    _feature_group_iterator++;
    return *this;
  }

    friend difference_type operator-(const index_iterator_t& lhs, const index_iterator_t& rhs)
  {
      return lhs._feature_group_iterator - rhs._feature_group_iterator;
  }


  bool operator==(const index_iterator_t& rhs) { return _feature_group_iterator == rhs._feature_group_iterator; }
  bool operator!=(const index_iterator_t& rhs) { return _feature_group_iterator != rhs._feature_group_iterator; }
};

/// Insertion or removal will result in this value in invalidated.
template <typename IndexItT, typename FeatureGroupIteratorT, typename FeaturesT>
class indexed_iterator_t
{
  IndexItT _indices_it;
  FeatureGroupIteratorT _namespaced_feature_groups;

public:
  using difference_type = std::ptrdiff_t;

  indexed_iterator_t(IndexItT indices, FeatureGroupIteratorT feature_groups)
      : _indices_it(indices)
      , _namespaced_feature_groups(feature_groups)
  {
  }

  FeaturesT& operator*()
  {
    return _namespaced_feature_groups[*_indices_it]._features;
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

  namespace_index index()
  {
    return _namespaced_feature_groups[*_indices_it]._index;
  }

  uint64_t hash()
  {
    return _namespaced_feature_groups[*_indices_it]._hash;
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
  using iterator = iterator_t<std::vector<details::namespaced_feature_group>::iterator, features>;
  using const_iterator = iterator_t<std::vector<details::namespaced_feature_group>::const_iterator, const features>;
  using indexed_iterator = indexed_iterator_t<std::vector<size_t>::iterator, std::vector<details::namespaced_feature_group>::iterator, features>;
  using const_indexed_iterator =
      indexed_iterator_t<std::vector<size_t>::const_iterator, std::vector<details::namespaced_feature_group>::const_iterator, const features>;

  using ns_index_iterator = index_iterator_t<std::vector<details::namespaced_feature_group>::iterator>;
  using const_ns_index_iterator = index_iterator_t<std::vector<details::namespaced_feature_group>::const_iterator>;

  namespaced_features() = default;
  ~namespaced_features() = default;

  // For copies we deep copy the features but not the pool
  namespaced_features(const namespaced_features& other)
  {
    _feature_groups = other._feature_groups;
    _legacy_indices_to_index_mapping = other._legacy_indices_to_index_mapping;
  }
  namespaced_features& operator=(const namespaced_features& other)
  {
    _feature_groups = other._feature_groups;
    _legacy_indices_to_index_mapping = other._legacy_indices_to_index_mapping;
    return *this;
  }
  namespaced_features(namespaced_features&& other) = default;
  namespaced_features& operator=(namespaced_features&& other) = default;

  inline size_t size() const { return _feature_groups.size(); }
  inline bool empty() const { return _feature_groups.empty(); }

  // Returns nullptr if not found.
  inline features* get_feature_group(uint64_t hash)
  {
    auto it = std::find_if(_feature_groups.begin(), _feature_groups.end(), [hash](const details::namespaced_feature_group& group) {
      return group._hash == hash;
    });
    if (it == _feature_groups.end()) { return nullptr; }
    return &it->_features;
  }
  // Returns nullptr if not found.
  inline const features* get_feature_group(uint64_t hash) const
  {
    auto it = std::find_if(_feature_groups.begin(), _feature_groups.end(),
        [hash](const details::namespaced_feature_group& group) {
      return group._hash == hash;
    });
    if (it == _feature_groups.end()) { return nullptr; }
    return &it->_features;
  }

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
      _feature_groups.emplace_back(_saved_feature_groups.take_back(), hash, ns_index);
      _saved_feature_groups.pop_back();
      auto new_index = _feature_groups.size() - 1;
      auto& idx_vec = _legacy_indices_to_index_mapping[ns_index];
      idx_vec.push_back(new_index);
      if (idx_vec.size() == 1) { _legacy_indices_existing.push_back(ns_index); }
      return _feature_groups.back()._features;
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

  // Wil contains duplicates if there exists more than one feature group per index.
  inline ns_index_iterator index_begin()
  {
    return {_feature_groups.begin()};
  }

  inline ns_index_iterator index_end()
  {
    return {_feature_groups.end()};
  }

  inline iterator begin()
  {
    return {_feature_groups.begin()};
  }
  inline iterator end()
  {
    return {_feature_groups.end()};
  }
  inline const_iterator begin() const
  {
    return {_feature_groups.cbegin()};
  }
  inline const_iterator end() const
  {
    return {_feature_groups.cend()};
  }
  inline const_iterator cbegin() const
  {
    return {_feature_groups.cbegin()};
  }
  inline const_iterator cend() const
  {
    return {_feature_groups.cend()};
  }

private:
  std::vector<details::namespaced_feature_group> _feature_groups;
  std::array<std::vector<size_t>, 256> _legacy_indices_to_index_mapping;
  std::vector<namespace_index> _legacy_indices_existing;
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
    _feature_groups.emplace_back(std::forward<FeaturesT>(ftrs), hash, ns_index);
    auto new_index = _feature_groups.size() - 1;
    auto& idx_vec = _legacy_indices_to_index_mapping[ns_index];
    idx_vec.push_back(new_index);
    if (idx_vec.size() == 1) { _legacy_indices_existing.push_back(ns_index); }
    return _feature_groups.back()._features;
  }

  existing_group->concat(std::forward<FeaturesT>(ftrs));
  // Should we ensure that this doesnt already exist under a DIFFERENT namespace_index?
  // However, his shouldn't be possible as ns_index depends on hash.

  auto it = std::find_if(_feature_groups.begin(), _feature_groups.end(),
      [hash](const details::namespaced_feature_group& group) {
    return group._hash == hash;
  });
  assert(it != _feature_groups.end());
  auto group_index = std::distance(_feature_groups.begin(), it);
  auto& ns_indices_list = _legacy_indices_to_index_mapping[ns_index];
  if (std::find(ns_indices_list.begin(), ns_indices_list.end(), group_index) == ns_indices_list.end())
  {
    ns_indices_list.push_back(group_index);
  }
  return *existing_group;
}

}  // namespace VW
