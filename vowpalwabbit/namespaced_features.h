// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <utility>
#include <vector>
#include <unordered_map>

#include "vw_types.h"
#include "feature_group.h"

template <typename IteratorT, typename dummy = void>
class generic_range
{
};

template <typename IteratorT>
class generic_range<IteratorT, typename std::enable_if<std::is_const<IteratorT>::value>::type>
{
  IteratorT _begin;
  IteratorT _end;

public:
  generic_range(IteratorT begin, IteratorT end) : _begin(begin), _end(end) {}
  typename std::enable_if<!std::is_const<IteratorT>::value>::type begin() { return _begin; }
  typename std::enable_if<!std::is_const<IteratorT>::value>::type end() { return _end; }
  IteratorT begin() const { return _begin; }
  IteratorT end() const { return _end; }
};

template <typename IteratorT>
class generic_range<IteratorT, typename std::enable_if<!std::is_const<IteratorT>::value>::type>
{
  IteratorT _begin;
  IteratorT _end;

public:
  generic_range(IteratorT begin, IteratorT end) : _begin(begin), _end(end) {}
  IteratorT begin() { return _begin; }
  IteratorT end() { return _end; }
};

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

template <typename FeaturesT, typename IndexT, typename HashT>
class indexed_iterator_t
{
  std::vector<size_t>::iterator _index_vec_it;
  FeaturesT* _feature_groups;
  IndexT* _namespace_indices;
  HashT* _namespace_hashes;

public:
  indexed_iterator_t(std::vector<size_t>::iterator index_vec_it, FeaturesT* feature_groups, IndexT* namespace_indices,
      HashT* namespace_hashes)
      : _index_vec_it(index_vec_it)
      , _feature_groups(feature_groups)
      , _namespace_indices(namespace_indices)
      , _namespace_hashes(namespace_hashes)
  {
  }
  FeaturesT& operator*() { return _feature_groups[*_index_vec_it]; }
  indexed_iterator_t& operator++()
  {
    ++_index_vec_it;
    return *this;
  }

  IndexT index() { return _namespace_indices[*_index_vec_it]; }
  HashT hash() { return _namespace_hashes[*_index_vec_it]; }

  bool operator==(const indexed_iterator_t& rhs) { return _index_vec_it == rhs._index_vec_it; }
  bool operator!=(const indexed_iterator_t& rhs) { return _index_vec_it != rhs._index_vec_it; }
};

/// namespace_index - 1 byte namespace identifier. Either the first character of the namespace or a reserved namespace
/// identifier namespace_hash - 8 byte hash
struct namespaced_features
{
  using iterator = iterator_t<features, namespace_index, uint64_t>;
  using const_iterator = iterator_t<const features, const namespace_index, const uint64_t>;

  using indexed_iterator = indexed_iterator_t<features, namespace_index, uint64_t>;
  using const_indexed_iterator = indexed_iterator_t<const features, const namespace_index, const uint64_t>;

  inline generic_range<indexed_iterator> namespace_index_range(namespace_index index)
  {
    return {namespace_index_begin(index), namespace_index_end(index)};
  }

  inline generic_range<const_indexed_iterator> namespace_index_range(namespace_index index) const
  {
    return {namespace_index_begin(index), namespace_index_end(index)};
  }

  // Returns nullptr if not found.
  features* get_feature_group(uint64_t hash);
  // Returns nullptr if not found.
  const features* get_feature_group(uint64_t hash) const;

  const std::vector<namespace_index>& get_indices() const;

  // Returns empty range if not found
  std::pair<indexed_iterator, indexed_iterator> get_namespace_index_groups(namespace_index index);
  // Returns empty range if not found
  std::pair<const_indexed_iterator, const_indexed_iterator> get_namespace_index_groups(namespace_index index) const;

  // If a feature group already exists in this "slot" it will be merged
  features& merge_feature_group(features&& ftrs, uint64_t hash, namespace_index index);
  features& merge_feature_group(const features& ftrs, uint64_t hash, namespace_index index);

  // If no feature group already exists a default one will be created.
  features& get_or_create_feature_group(uint64_t hash, namespace_index index);

  const features& operator[](uint64_t hash) const;
  features& operator[](uint64_t hash);

  void remove_feature_group(uint64_t hash);

  indexed_iterator namespace_index_begin(namespace_index index);
  indexed_iterator namespace_index_end(namespace_index index);
  const_indexed_iterator namespace_index_begin(namespace_index index) const;
  const_indexed_iterator namespace_index_end(namespace_index index) const;
  const_indexed_iterator namespace_index_cbegin(namespace_index index) const;
  const_indexed_iterator namespace_index_cend(namespace_index index) const;

  iterator begin() { return {0, _feature_groups.data(), _namespace_indices.data(), _namespace_hashes.data()}; }
  iterator end()
  {
    return iterator(
        _feature_groups.size(), _feature_groups.data(), _namespace_indices.data(), _namespace_hashes.data());
  }
  const_iterator begin() const
  {
    return const_iterator{0, _feature_groups.data(), _namespace_indices.data(), _namespace_hashes.data()};
  }
  const_iterator end() const
  {
    return const_iterator{
        _feature_groups.size(), _feature_groups.data(), _namespace_indices.data(), _namespace_hashes.data()};
  }
  const_iterator cbegin() const
  {
    return const_iterator{0, _feature_groups.data(), _namespace_indices.data(), _namespace_hashes.data()};
  }
  const_iterator cend() const
  {
    return const_iterator{
        _feature_groups.size(), _feature_groups.data(), _namespace_indices.data(), _namespace_hashes.data()};
  }

private:
  std::vector<features> _feature_groups;
  std::vector<namespace_index> _namespace_indices;
  std::vector<uint64_t> _namespace_hashes;

  std::unordered_map<namespace_index, std::vector<size_t>> _legacy_indices_to_index_mapping;
  std::unordered_map<uint64_t, size_t> _hash_to_index_mapping;
};
}  // namespace VW
