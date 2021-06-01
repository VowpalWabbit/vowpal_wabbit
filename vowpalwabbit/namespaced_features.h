// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <utility>
#include <vector>
#include <unordered_map>

#include "feature_group.h"
#include "generic_range.h"

typedef unsigned char namespace_index;

namespace VW
{
template <typename FeaturesT, typename IndexT, typename HashT>
class iterator_t
{
  size_t _index;
  FeaturesT* _feature_groups;
  IndexT* _namespace_indices;
  HashT* _namespace_hashes;

public:
  iterator_t(size_t index, FeaturesT* feature_groups, IndexT* namespace_indices, HashT* namespace_hashes)
      : _index(index)
      , _feature_groups(feature_groups)
      , _namespace_indices(namespace_indices)
      , _namespace_hashes(namespace_hashes)
  {
  }
  FeaturesT& operator*() { return _feature_groups[_index]; }
  iterator_t& operator++()
  {
    ++_index;
    return *this;
  }

  IndexT index() { return _namespace_indices[_index]; }
  HashT hash() { return _namespace_hashes[_index]; }

  bool operator==(const iterator_t& rhs) { return _index == rhs._index; }
  bool operator!=(const iterator_t& rhs) { return _index != rhs._index; }
};

template <typename IndicesT, typename FeaturesT, typename IndexT, typename HashT>
class indexed_iterator_t
{
  IndicesT* _indices;
  FeaturesT* _feature_groups;
  IndexT* _namespace_indices;
  HashT* _namespace_hashes;

public:
  using difference_type = std::ptrdiff_t;

  indexed_iterator_t(IndicesT* indices, FeaturesT* feature_groups, IndexT* namespace_indices, HashT* namespace_hashes)
      : _indices(indices)
      , _feature_groups(feature_groups)
      , _namespace_indices(namespace_indices)
      , _namespace_hashes(namespace_hashes)
  {
  }
  FeaturesT& operator*()
  {
    if (_indices == nullptr) { THROW("Invalid iterator"); }
    return _feature_groups[*_indices];
  }
  indexed_iterator_t& operator++()
  {
    if (_indices != nullptr) { ++_indices; }
    return *this;
  }

  IndexT index()
  {
    if (_indices == nullptr) { THROW("Invalid iterator"); }
    return _namespace_indices[*_indices];
  }
  HashT hash()
  {
    if (_indices == nullptr) { THROW("Invalid iterator"); }
    return _namespace_hashes[*_indices];
  }

  friend difference_type operator-(const indexed_iterator_t& lhs, const indexed_iterator_t& rhs)
  {
    return lhs._indices - rhs._indices;
  }

  bool operator==(const indexed_iterator_t& rhs) { return _indices == rhs._indices; }
  bool operator!=(const indexed_iterator_t& rhs) { return _indices != rhs._indices; }
};

/// namespace_index - 1 byte namespace identifier. Either the first character of the namespace or a reserved namespace
/// identifier namespace_hash - 8 byte hash
struct namespaced_features
{
  using iterator = iterator_t<features, namespace_index, uint64_t>;
  using const_iterator = iterator_t<const features, const namespace_index, const uint64_t>;
  using indexed_iterator = indexed_iterator_t<size_t, features, namespace_index, uint64_t>;
  using const_indexed_iterator =
      indexed_iterator_t<const size_t, const features, const namespace_index, const uint64_t>;

  inline size_t size() const { return _feature_groups.size(); }
  inline bool empty() const { return _feature_groups.empty(); }

  // Returns nullptr if not found.
  features* get_feature_group(uint64_t hash);
  // Returns nullptr if not found.
  const features* get_feature_group(uint64_t hash) const;

  // TODO - don't generate this per call.
  std::vector<namespace_index> get_indices() const;

  // Returns empty range if not found
  std::pair<indexed_iterator, indexed_iterator> get_namespace_index_groups(namespace_index index);
  // Returns empty range if not found
  std::pair<const_indexed_iterator, const_indexed_iterator> get_namespace_index_groups(namespace_index index) const;

  // If a feature group already exists in this "slot" it will be merged
  template <typename FeaturesT>
  features& merge_feature_group(FeaturesT&& ftrs, uint64_t hash, namespace_index index);

  // If no feature group already exists a default one will be created.
  // Creating new feature groups will invalidate any pointers or references held.
  features& get_or_create_feature_group(uint64_t hash, namespace_index index);

  const features& operator[](uint64_t hash) const;
  features& operator[](uint64_t hash);

  // Removing a feature group will invalidate any pointers or references held.
  void remove_feature_group(uint64_t hash);

  generic_range<indexed_iterator> namespace_index_range(namespace_index index);
  generic_range<const_indexed_iterator> namespace_index_range(namespace_index index) const;
  indexed_iterator namespace_index_begin(namespace_index index);
  indexed_iterator namespace_index_end(namespace_index index);
  const_indexed_iterator namespace_index_begin(namespace_index index) const;
  const_indexed_iterator namespace_index_end(namespace_index index) const;
  const_indexed_iterator namespace_index_cbegin(namespace_index index) const;
  const_indexed_iterator namespace_index_cend(namespace_index index) const;

  iterator begin();
  iterator end();
  const_iterator begin() const;
  const_iterator end() const;
  const_iterator cbegin() const;
  const_iterator cend() const;

private:
  std::vector<features> _feature_groups;
  std::vector<namespace_index> _namespace_indices;
  std::vector<uint64_t> _namespace_hashes;

  std::unordered_map<namespace_index, std::vector<size_t>> _legacy_indices_to_index_mapping;
  std::unordered_map<uint64_t, size_t> _hash_to_index_mapping;
};

// If a feature group already exists in this "slot" it will be merged
// Creating new feature groups will invalidate any pointers or references held.
template <typename FeaturesT>
features& namespaced_features::merge_feature_group(FeaturesT&& ftrs, uint64_t hash, namespace_index ns_index)
{
  auto* existing_group = get_feature_group(hash);
  if (existing_group == nullptr)
  {
    _feature_groups.emplace_back(std::forward<FeaturesT>(ftrs));
    _namespace_indices.push_back(ns_index);
    _namespace_hashes.push_back(hash);
    auto new_index = _feature_groups.size() - 1;
    _hash_to_index_mapping[hash] = new_index;
    _legacy_indices_to_index_mapping[ns_index].push_back(new_index);
    existing_group = &_feature_groups.back();
  }
  else
  {
    existing_group->push_back(std::forward<FeaturesT>(ftrs));
    auto existing_index = _hash_to_index_mapping[hash];
    // Should we ensure that this doesnt already exist under a DIFFERENT namespace_index?
    // However, his shouldn't be possible as ns_index depends on hash.
    auto& indices_list = _legacy_indices_to_index_mapping[ns_index];
    if (std::find(indices_list.begin(), indices_list.end(), ns_index) == indices_list.end())
    { indices_list.push_back(existing_index); }
  }
  return *existing_group;
}

}  // namespace VW
