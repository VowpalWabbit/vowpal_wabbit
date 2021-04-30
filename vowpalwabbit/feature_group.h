// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "v_array.h"
#include "future_compat.h"

#include <utility>
#include <memory>
#include <string>
#include <algorithm>
#include <cstddef>
#include <vector>
#include <type_traits>

using feature_value = float;
using feature_index = uint64_t;
using audit_strings = std::pair<std::string, std::string>;
using audit_strings_ptr = std::shared_ptr<audit_strings>;

struct features;
struct features_value_index_audit_range;

// sparse feature definition for the library interface
struct feature
{
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

template <typename feature_value_type_t, typename feature_index_type_t, typename audit_type_t>
class audit_features_iterator final
{
private:
  feature_value_type_t* _begin_values;
  feature_index_type_t* _begin_indices;
  audit_type_t* _begin_audit;

public:
  using iterator_category = std::random_access_iterator_tag;
  using difference_type = std::ptrdiff_t;
  using value_type = audit_features_iterator<feature_value_type_t, feature_index_type_t, audit_type_t>;
  using pointer = value_type*;
  using reference = value_type&;

  audit_features_iterator(
      feature_value_type_t* begin_values, feature_index_type_t* begin_indices, audit_type_t* begin_audit)
      : _begin_values(begin_values), _begin_indices(begin_indices), _begin_audit(begin_audit)
  {
  }

  audit_features_iterator(const audit_features_iterator&) = default;
  audit_features_iterator& operator=(const audit_features_iterator&) = default;
  audit_features_iterator(audit_features_iterator&&) = default;
  audit_features_iterator& operator=(audit_features_iterator&&) = default;

  inline feature_value_type_t& value() { return *_begin_values; }
  inline const feature_value_type_t& value() const { return *_begin_values; }

  inline feature_index_type_t& index() { return *_begin_indices; }
  inline const feature_index_type_t& index() const { return *_begin_indices; }

  inline audit_type_t& audit() { return *_begin_audit; }
  inline const audit_type_t& audit() const { return *_begin_audit; }

  inline reference operator*() { return *this; }
  inline const reference operator*() const { return *this; }

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
    if (_begin_audit != nullptr) { _begin_audit++; }
    return *this;
  }

  audit_features_iterator& operator-=(difference_type diff)
  {
    _begin_values -= diff;
    _begin_indices -= diff;
    if (_begin_audit != nullptr) { _begin_audit++; }
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
  friend struct features;
};

template <typename feature_value_type_t, typename feature_index_type_t>
class features_iterator final
{
private:
  feature_value_type_t* _begin_values;
  feature_index_type_t* _begin_indices;

public:
  using iterator_category = std::random_access_iterator_tag;
  using difference_type = std::ptrdiff_t;
  using value_type = features_iterator<feature_value_type_t, feature_index_type_t>;
  using pointer = value_type*;
  using reference = value_type&;

  features_iterator(feature_value_type_t* begin_values, feature_index_type_t* begin_indices)
      : _begin_values(begin_values), _begin_indices(begin_indices)
  {
  }

  features_iterator(const features_iterator&) = default;
  features_iterator& operator=(const features_iterator&) = default;
  features_iterator(features_iterator&&) = default;
  features_iterator& operator=(features_iterator&&) = default;

  inline feature_value_type_t& value() { return *_begin_values; }
  inline const feature_value_type_t& value() const { return *_begin_values; }

  inline feature_index_type_t& index() { return *_begin_indices; }
  inline const feature_index_type_t& index() const { return *_begin_indices; }

  inline reference operator*() { return *this; }
  inline const reference operator*() const { return *this; }

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
  friend struct features;
};

/// the core definition of a set of features.
struct features
{
  using iterator = features_iterator<feature_value, feature_index>;
  using const_iterator = features_iterator<const feature_value, const feature_index>;
  using audit_iterator = audit_features_iterator<feature_value, feature_index, audit_strings_ptr>;
  using const_audit_iterator =
      audit_features_iterator<const feature_value, const feature_index, const audit_strings_ptr>;

  v_array<feature_value> values;               // Always needed.
  v_array<feature_index> indicies;             // Optional for sparse data.
  std::vector<audit_strings_ptr> space_names;  // Optional for audit mode.

  float sum_feat_sq = 0.f;

  class features_audit_range
  {
  private:
    features* _outer;

  public:
    explicit features_audit_range(features* outer) : _outer(outer) {}

    inline audit_iterator begin() { return _outer->audit_begin(); }
    inline const_audit_iterator begin() const { return _outer->audit_cbegin(); }
    inline audit_iterator end() { return _outer->audit_end(); }
    inline const_audit_iterator end() const { return _outer->audit_cend(); }

    inline const_audit_iterator cbegin() const { return _outer->audit_cbegin(); }
    inline const_audit_iterator cend() const { return _outer->audit_cend(); }
  };

  class const_features_audit_range
  {
  private:
    const features* _outer;

  public:
    explicit const_features_audit_range(const features* outer) : _outer(outer) {}

    inline const_audit_iterator cbegin() const { return _outer->audit_cbegin(); }
    inline const_audit_iterator cend() const { return _outer->audit_cend(); }
  };

  features() = default;
  ~features() = default;
  features(const features&) = delete;
  features& operator=(const features&) = delete;

  // custom move operators required since we need to leave the old value in
  // a null state to prevent freeing of shallow copied v_arrays
  features(features&& other) = default;
  features& operator=(features&& other) = default;

  inline size_t size() const { return values.size(); }

  inline bool empty() const { return values.empty(); }
  inline bool nonempty() const { return !empty(); }

  VW_DEPRECATED("Freeing space names is handled directly by truncation or removal.")
  void free_space_names(size_t i);

  // default iterator for values & features
  inline iterator begin() { return {values.begin(), indicies.begin()}; }
  inline const_iterator begin() const { return {values.begin(), indicies.begin()}; }
  inline iterator end() { return {values.end(), indicies.end()}; }
  inline const_iterator end() const { return {values.end(), indicies.end()}; }

  inline const_iterator cbegin() const { return {values.cbegin(), indicies.cbegin()}; }
  inline const_iterator cend() const { return {values.cend(), indicies.cend()}; }

  inline features_audit_range audit_range() { return features_audit_range{this}; }
  inline const_features_audit_range audit_range() const { return const_features_audit_range{this}; }

  inline audit_iterator audit_begin() { return {values.begin(), indicies.begin(), space_names.data()}; }
  inline const_audit_iterator audit_begin() const { return {values.begin(), indicies.begin(), space_names.data()}; }
  inline audit_iterator audit_end() { return {values.end(), indicies.end(), space_names.data() + space_names.size()}; }
  inline const_audit_iterator audit_end() const
  {
    return {values.end(), indicies.end(), space_names.data() + space_names.size()};
  }

  inline const_audit_iterator audit_cbegin() const { return {values.begin(), indicies.begin(), space_names.data()}; }
  inline const_audit_iterator audit_cend() const
  {
    return {values.end(), indicies.end(), space_names.data() + space_names.size()};
  }

  void clear();
  void truncate_to(const audit_iterator& pos);
  void truncate_to(const iterator& pos);
  void truncate_to(size_t i);
  void push_back(feature_value v, feature_index i);
  bool sort(uint64_t parse_mask);
  void deep_copy_from(const features& src);
};
