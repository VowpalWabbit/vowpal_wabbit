// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/common/future_compat.h"
#include "vw/core/generic_range.h"
#include "vw/core/v_array.h"

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <memory>
#include <numeric>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace VW
{

using feature_value = float;
using feature_index = uint64_t;
using namespace_index = unsigned char;

class features;

class audit_strings
{
public:
  std::string ns;
  std::string name;

  // This is only set if chain hashing is in use.
  std::string str_value;

  audit_strings() = default;
  audit_strings(std::string ns, std::string name) : ns(std::move(ns)), name(std::move(name)) {}
  audit_strings(std::string ns, std::string name, std::string str_value)
      : ns(std::move(ns)), name(std::move(name)), str_value(std::move(str_value))
  {
  }

  bool is_empty() const { return ns.empty() && name.empty() && str_value.empty(); }
};

inline std::string to_string(const audit_strings& ai)
{
  std::ostringstream ss;
  if (!ai.ns.empty() && ai.ns != " ") { ss << ai.ns << '^'; }
  ss << ai.name;
  if (!ai.str_value.empty()) { ss << '^' << ai.str_value; }
  return ss.str();
}

// First: character based feature group, second: hash of extent
using extent_term = std::pair<namespace_index, uint64_t>;

// sparse feature definition for the library interface
class feature
{
public:
  float x;
  uint64_t weight_index;

  feature() = default;
  feature(float _x, uint64_t _index) : x(_x), weight_index(_index) {}

  feature(const feature&) = default;
  feature& operator=(const feature&) = default;
  feature(feature&&) = default;
  feature& operator=(feature&&) = default;
};
static_assert(std::is_trivial<feature>::value, "To be used in v_array feature must be trivial");

class namespace_extent
{
public:
  namespace_extent() = default;

  namespace_extent(size_t begin_index, size_t end_index, uint64_t hash)
      : begin_index(begin_index), end_index(end_index), hash(hash)
  {
  }

  namespace_extent(size_t begin_index, uint64_t hash) : begin_index(begin_index), hash(hash) {}

  size_t begin_index = 0;
  size_t end_index = 0;
  uint64_t hash = 0;

  friend bool operator==(const namespace_extent& lhs, const namespace_extent& rhs)
  {
    return lhs.hash == rhs.hash && lhs.begin_index == rhs.begin_index && lhs.end_index == rhs.end_index;
  }
  friend bool operator!=(const namespace_extent& lhs, const namespace_extent& rhs) { return !(lhs == rhs); }
};
namespace details
{
std::vector<std::pair<bool, uint64_t>> flatten_namespace_extents(
    const std::vector<namespace_extent>& extents, size_t overall_feature_space_size);

std::vector<namespace_extent> unflatten_namespace_extents(const std::vector<std::pair<bool, uint64_t>>& extents);

template <typename feature_value_type_t, typename feature_index_type_t, typename audit_type_t>
class audit_features_iterator final
{
public:
  using iterator_category = std::random_access_iterator_tag;
  using difference_type = std::ptrdiff_t;
  using value_type = audit_features_iterator<feature_value_type_t, feature_index_type_t, audit_type_t>;
  using pointer = value_type*;
  using reference = value_type&;
  using const_reference = const value_type&;

  audit_features_iterator() : _begin_values(nullptr), _begin_indices(nullptr), _begin_audit(nullptr) {}

  audit_features_iterator(
      feature_value_type_t* begin_values, feature_index_type_t* begin_indices, audit_type_t* begin_audit)
      : _begin_values(begin_values), _begin_indices(begin_indices), _begin_audit(begin_audit)
  {
  }

  audit_features_iterator(const audit_features_iterator&) = default;
  audit_features_iterator& operator=(const audit_features_iterator&) = default;
  audit_features_iterator(audit_features_iterator&&) noexcept = default;
  audit_features_iterator& operator=(audit_features_iterator&&) noexcept = default;

  inline feature_value_type_t& value() { return *_begin_values; }
  inline const feature_value_type_t& value() const { return *_begin_values; }

  inline feature_index_type_t& index() { return *_begin_indices; }
  inline const feature_index_type_t& index() const { return *_begin_indices; }

  inline audit_type_t* audit() { return _begin_audit; }
  inline const audit_type_t* audit() const { return _begin_audit; }

  inline reference operator*() { return *this; }
  inline const_reference operator*() const { return *this; }

  // Required for forward_iterator
  audit_features_iterator& operator++()
  {
    _begin_values++;
    _begin_indices++;
    if (_begin_audit != nullptr) { _begin_audit++; }
    return *this;
  }

  audit_features_iterator& operator+=(difference_type diff)
  {
    _begin_values += diff;
    _begin_indices += diff;
    if (_begin_audit != nullptr) { _begin_audit += diff; }
    return *this;
  }

  audit_features_iterator& operator-=(difference_type diff)
  {
    _begin_values -= diff;
    _begin_indices -= diff;
    if (_begin_audit != nullptr) { _begin_audit += diff; }
    return *this;
  }

  friend audit_features_iterator<feature_value_type_t, feature_index_type_t, audit_type_t> operator+(
      const audit_features_iterator& lhs, difference_type rhs)
  {
    return {lhs._begin_values + rhs, lhs._begin_indices + rhs,
        lhs._begin_audit != nullptr ? lhs._begin_audit + rhs : nullptr};
  }

  friend audit_features_iterator<feature_value_type_t, feature_index_type_t, audit_type_t> operator+(
      difference_type lhs, const audit_features_iterator& rhs)
  {
    return {rhs._begin_values + lhs, rhs._begin_indices + lhs,
        rhs._begin_audit != nullptr ? rhs._begin_audit + lhs : nullptr};
  }

  friend difference_type operator-(const audit_features_iterator& lhs, const audit_features_iterator& rhs)
  {
    return lhs._begin_values - rhs._begin_values;
  }

  friend audit_features_iterator<feature_value_type_t, feature_index_type_t, audit_type_t> operator-(
      const audit_features_iterator& lhs, difference_type rhs)
  {
    return {lhs._begin_values - rhs, lhs._begin_indices - rhs,
        lhs._begin_audit != nullptr ? lhs._begin_audit - rhs : nullptr};
  }

  // For all of the comparison operations only _begin_values is used, since the other values are incremented in sequence
  // (or ignored in the case of _begin_audit if it is nullptr)
  friend bool operator<(const audit_features_iterator& lhs, const audit_features_iterator& rhs)
  {
    return lhs._begin_values < rhs._begin_values;
  }

  friend bool operator>(const audit_features_iterator& lhs, const audit_features_iterator& rhs)
  {
    return lhs._begin_values > rhs._begin_values;
  }

  friend bool operator<=(const audit_features_iterator& lhs, const audit_features_iterator& rhs)
  {
    return !(lhs > rhs);
  }
  friend bool operator>=(const audit_features_iterator& lhs, const audit_features_iterator& rhs)
  {
    return !(lhs < rhs);
  }

  friend bool operator==(const audit_features_iterator& lhs, const audit_features_iterator& rhs)
  {
    return lhs._begin_values == rhs._begin_values;
  }

  friend bool operator!=(const audit_features_iterator& lhs, const audit_features_iterator& rhs)
  {
    return !(lhs == rhs);
  }

  friend void swap(audit_features_iterator& lhs, audit_features_iterator& rhs)
  {
    std::swap(lhs._begin_values, rhs._begin_values);
    std::swap(lhs._begin_indices, rhs._begin_indices);
    std::swap(lhs._begin_audit, rhs._begin_audit);
  }
  friend class ::VW::features;

private:
  feature_value_type_t* _begin_values;
  feature_index_type_t* _begin_indices;
  audit_type_t* _begin_audit;
};

template <typename features_t, typename audit_features_iterator_t, typename extent_it>
class ns_extent_iterator final
{
public:
  ns_extent_iterator(features_t* feature_group, uint64_t hash, extent_it index_current)
      : _feature_group(feature_group), _hash(hash), _index_current(index_current)
  {
    // Seek to the first valid position.
    while (_index_current != _feature_group->namespace_extents.end() && _index_current->hash != _hash)
    {
      ++_index_current;
    }
  }

  using iterator_category = std::forward_iterator_tag;
  using difference_type = std::ptrdiff_t;
  using value_type = std::pair<audit_features_iterator_t, audit_features_iterator_t>;
  using pointer = value_type*;
  using reference = value_type&;
  using const_reference = const value_type&;
  std::pair<audit_features_iterator_t, audit_features_iterator_t> operator*()
  {
    return std::make_pair(_feature_group->audit_begin() + _index_current->begin_index,
        _feature_group->audit_begin() + _index_current->end_index);
  }
  std::pair<audit_features_iterator_t, audit_features_iterator_t> operator*() const
  {
    return std::make_pair(_feature_group->audit_begin() + _index_current->begin_index,
        _feature_group->audit_begin() + _index_current->end_index);
  }

  // Required for forward_iterator
  ns_extent_iterator& operator++()
  {
    ++_index_current;
    while (_index_current != _feature_group->namespace_extents.end() && _index_current->hash != _hash)
    {
      ++_index_current;
    }

    return *this;
  }

  friend bool operator==(const ns_extent_iterator& lhs, const ns_extent_iterator& rhs)
  {
    return lhs._feature_group == rhs._feature_group && lhs._index_current == rhs._index_current;
  }

  friend bool operator!=(const ns_extent_iterator& lhs, const ns_extent_iterator& rhs) { return !(lhs == rhs); }
  friend class ::VW::features;

private:
  features_t* _feature_group;
  uint64_t _hash;
  extent_it _index_current;
};

template <typename feature_value_type_t, typename feature_index_type_t>
class features_iterator final
{
public:
  using iterator_category = std::random_access_iterator_tag;
  using difference_type = std::ptrdiff_t;
  using value_type = features_iterator<feature_value_type_t, feature_index_type_t>;
  using pointer = value_type*;
  using reference = value_type&;
  using const_reference = const value_type&;

  features_iterator() : _begin_values(nullptr), _begin_indices(nullptr) {}

  features_iterator(feature_value_type_t* begin_values, feature_index_type_t* begin_indices)
      : _begin_values(begin_values), _begin_indices(begin_indices)
  {
  }

  features_iterator(const features_iterator&) = default;
  features_iterator& operator=(const features_iterator&) = default;
  features_iterator(features_iterator&&) noexcept = default;
  features_iterator& operator=(features_iterator&&) noexcept = default;

  inline feature_value_type_t& value() { return *_begin_values; }
  inline const feature_value_type_t& value() const { return *_begin_values; }

  inline feature_index_type_t& index() { return *_begin_indices; }
  inline const feature_index_type_t& index() const { return *_begin_indices; }

  inline reference operator*() { return *this; }
  inline const_reference operator*() const { return *this; }

  features_iterator& operator++()
  {
    _begin_values++;
    _begin_indices++;
    return *this;
  }

  features_iterator& operator+=(difference_type diff)
  {
    _begin_values += diff;
    _begin_indices += diff;
    return *this;
  }

  features_iterator& operator-=(difference_type diff)
  {
    _begin_values -= diff;
    _begin_indices -= diff;
    return *this;
  }

  friend features_iterator<feature_value_type_t, feature_index_type_t> operator+(
      const features_iterator& lhs, difference_type rhs)
  {
    return {lhs._begin_values + rhs, lhs._begin_indices + rhs};
  }

  friend features_iterator<feature_value_type_t, feature_index_type_t> operator+(
      difference_type lhs, const features_iterator& rhs)
  {
    return {rhs._begin_values + lhs, rhs._begin_indices + lhs};
  }

  friend difference_type operator-(const features_iterator& lhs, const features_iterator& rhs)
  {
    return lhs._begin_values - rhs._begin_values;
  }

  friend features_iterator<feature_value_type_t, feature_index_type_t> operator-(
      const features_iterator& lhs, difference_type rhs)
  {
    return {lhs._begin_values - rhs, lhs._begin_indices - rhs};
  }

  // For all of the comparison operations only _begin_values is used, since _begin_indices is incremented along with it
  friend bool operator<(const features_iterator& lhs, const features_iterator& rhs)
  {
    return lhs._begin_values < rhs._begin_values;
  }

  friend bool operator>(const features_iterator& lhs, const features_iterator& rhs)
  {
    return lhs._begin_values > rhs._begin_values;
  }

  friend bool operator<=(const features_iterator& lhs, const features_iterator& rhs) { return !(lhs > rhs); }
  friend bool operator>=(const features_iterator& lhs, const features_iterator& rhs) { return !(lhs < rhs); }

  friend bool operator==(const features_iterator& lhs, const features_iterator& rhs)
  {
    return lhs._begin_values == rhs._begin_values;
  }

  friend bool operator!=(const features_iterator& lhs, const features_iterator& rhs) { return !(lhs == rhs); }

  friend void swap(features_iterator& lhs, features_iterator& rhs)
  {
    std::swap(lhs._begin_values, rhs._begin_values);
    std::swap(lhs._begin_indices, rhs._begin_indices);
  }
  friend class ::VW::features;

private:
  feature_value_type_t* _begin_values;
  feature_index_type_t* _begin_indices;
};

}  // namespace details

/// the core definition of a set of features.
class features
{
public:
  using iterator = details::features_iterator<feature_value, feature_index>;
  using const_iterator = details::features_iterator<const feature_value, const feature_index>;
  using audit_iterator = details::audit_features_iterator<feature_value, feature_index, VW::audit_strings>;
  using const_audit_iterator =
      details::audit_features_iterator<const feature_value, const feature_index, const VW::audit_strings>;
  using extent_iterator =
      details::ns_extent_iterator<features, audit_iterator, std::vector<VW::namespace_extent>::iterator>;
  using const_extent_iterator = details::ns_extent_iterator<const features, const_audit_iterator,
      std::vector<VW::namespace_extent>::const_iterator>;

  VW::v_array<feature_value> values;           // Always needed.
  VW::v_array<feature_index> indices;          // Optional for sparse data.
  std::vector<VW::audit_strings> space_names;  // Optional for audit mode.

  // Each extent represents a range [begin, end) of values which exist in a
  // given namespace. These extents must not overlap and the indices must not go
  // outside the range of the values container.
  std::vector<VW::namespace_extent> namespace_extents;

  float sum_feat_sq = 0.f;

  features() = default;
  ~features() = default;
  features(const features&) = default;
  features& operator=(const features&) = default;

  // custom move operators required since we need to leave the old value in
  // a null state to prevent freeing of shallow copied v_arrays
  features(features&& other) = default;
  features& operator=(features&& other) = default;

  inline size_t size() const { return values.size(); }

  inline bool empty() const { return values.empty(); }
  inline bool nonempty() const { return !empty(); }

  // default iterator for values & features
  inline iterator begin() { return {values.begin(), indices.begin()}; }
  inline const_iterator begin() const { return {values.begin(), indices.begin()}; }
  inline iterator end() { return {values.end(), indices.end()}; }
  inline const_iterator end() const { return {values.end(), indices.end()}; }

  inline const_iterator cbegin() const { return {values.cbegin(), indices.cbegin()}; }
  inline const_iterator cend() const { return {values.cend(), indices.cend()}; }

  inline VW::generic_range<audit_iterator> audit_range() { return {audit_begin(), audit_end()}; }
  inline VW::generic_range<const_audit_iterator> audit_range() const { return {audit_cbegin(), audit_cend()}; }

  inline audit_iterator audit_begin() { return {values.begin(), indices.begin(), space_names.data()}; }
  inline const_audit_iterator audit_begin() const { return {values.begin(), indices.begin(), space_names.data()}; }
  inline audit_iterator audit_end() { return {values.end(), indices.end(), space_names.data() + space_names.size()}; }
  inline const_audit_iterator audit_end() const
  {
    return {values.end(), indices.end(), space_names.data() + space_names.size()};
  }

  inline const_audit_iterator audit_cbegin() const { return {values.begin(), indices.begin(), space_names.data()}; }
  inline const_audit_iterator audit_cend() const
  {
    return {values.end(), indices.end(), space_names.data() + space_names.size()};
  }

  extent_iterator hash_extents_begin(uint64_t hash) { return {this, hash, namespace_extents.begin()}; }
  const_extent_iterator hash_extents_begin(uint64_t hash) const { return {this, hash, namespace_extents.begin()}; }
  extent_iterator hash_extents_end(uint64_t hash) { return {this, hash, namespace_extents.end()}; }
  const_extent_iterator hash_extents_end(uint64_t hash) const { return {this, hash, namespace_extents.end()}; }

  template <typename FuncT>
  void foreach_feature_for_hash(uint64_t hash, const FuncT& func) const
  {
    for (auto it = hash_extents_begin(hash); it != hash_extents_end(hash); ++it)
    {
      auto this_range = *it;
      for (auto inner_begin = this_range.first; inner_begin != this_range.second; ++inner_begin) { func(inner_begin); }
    }
  }

  void clear();
  // These 3 overloads can be used if the sum_feat_sq of the removed section is known to avoid recalculating.
  void truncate_to(const audit_iterator& pos, float sum_feat_sq_of_removed_section);
  void truncate_to(const iterator& pos, float sum_feat_sq_of_removed_section);
  void truncate_to(size_t i, float sum_feat_sq_of_removed_section);
  void truncate_to(const audit_iterator& pos);
  void truncate_to(const iterator& pos);
  void truncate_to(size_t i);
  void concat(const features& other);
  void push_back(feature_value v, feature_index i);
  void push_back(feature_value v, feature_index i, uint64_t ns_hash);
  bool sort(uint64_t parse_mask);

  void start_ns_extent(uint64_t hash);
  void end_ns_extent();

  bool validate_extents()
  {
    // For an extent to be complete it must not have an end index of 0 and it must be > 0 in width.
    const auto all_extents_complete = std::all_of(namespace_extents.begin(), namespace_extents.end(),
        [](const VW::namespace_extent& obj) { return obj.begin_index < obj.end_index; });
    return all_extents_complete;
  }
};
}  // namespace VW

using feature_value VW_DEPRECATED("Moved into VW namespace. Will be removed in VW 10.") = VW::feature_value;
using feature_index VW_DEPRECATED("Moved into VW namespace. Will be removed in VW 10.") = VW::feature_index;
using namespace_index VW_DEPRECATED("Moved into VW namespace. Will be removed in VW 10.") = VW::namespace_index;
using extent_term VW_DEPRECATED("Moved into VW namespace. Will be removed in VW 10.") = VW::extent_term;
using audit_strings VW_DEPRECATED("Moved into VW namespace. Will be removed in VW 10.") = VW::audit_strings;
using features VW_DEPRECATED("Moved into VW namespace. Will be removed in VW 10.") = VW::features;
using feature VW_DEPRECATED("Moved into VW namespace. Will be removed in VW 10.") = VW::feature;
