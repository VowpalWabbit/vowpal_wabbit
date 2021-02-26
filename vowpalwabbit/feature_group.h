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
  feature(feature&&) noexcept = default;
  feature& operator=(feature&&) noexcept = default;
};

static_assert(std::is_trivial<feature>::value, "To be used in v_array feature must be trivial");

/// iterator over feature values only
class features_value_iterator
{
protected:
  feature_value* _begin;

public:
  explicit features_value_iterator(feature_value* begin) : _begin(begin) {}

  features_value_iterator(const features_value_iterator&) = default;
  features_value_iterator& operator=(const features_value_iterator&) = default;
  features_value_iterator(features_value_iterator&&) = default;
  features_value_iterator& operator=(features_value_iterator&&) = default;

  inline features_value_iterator& operator++()
  {
    _begin++;
    return *this;
  }

  /// \return reference to the current value
  inline feature_value& value() { return *_begin; }

  inline features_value_iterator operator+(std::ptrdiff_t index) { return features_value_iterator(_begin + index); }

  inline features_value_iterator& operator+=(std::ptrdiff_t index)
  {
    _begin += index;
    return *this;
  }

  inline features_value_iterator& operator-=(std::ptrdiff_t index)
  {
    _begin -= index;
    return *this;
  }

  inline features_value_iterator& operator*() { return *this; }

  bool operator==(const features_value_iterator& rhs) { return _begin == rhs._begin; }
  bool operator!=(const features_value_iterator& rhs) { return _begin != rhs._begin; }

  friend void swap(features_value_iterator& lhs, features_value_iterator& rhs) { std::swap(lhs._begin, rhs._begin); }
  friend struct features;
};

/// iterator over values and indicies
class features_value_index_iterator : public features_value_iterator
{
protected:
  feature_index* _begin_index;

public:
  features_value_index_iterator(feature_value* begin, feature_index* begin_index)
      : features_value_iterator(begin), _begin_index(begin_index)
  {
  }

  features_value_index_iterator(const features_value_index_iterator&) = default;
  features_value_index_iterator& operator=(const features_value_index_iterator&) = default;
  features_value_index_iterator(features_value_index_iterator&&) = default;
  features_value_index_iterator& operator=(features_value_index_iterator&&) = default;

  inline features_value_index_iterator& operator++()
  {
    features_value_iterator::operator++();
    _begin_index++;
    return *this;
  }

  inline feature_index& index() { return *_begin_index; }

  inline features_value_index_iterator& operator+=(std::ptrdiff_t index)
  {
    features_value_iterator::operator+=(index);
    _begin_index += index;
    return *this;
  }

  inline features_value_index_iterator operator+(std::ptrdiff_t index)
  {
    return {_begin + index, _begin_index + index};
  }

  inline features_value_index_iterator& operator-=(std::ptrdiff_t index)
  {
    features_value_iterator::operator-=(index);
    _begin_index -= index;
    return *this;
  }

  inline features_value_index_iterator& operator*() { return *this; }

  friend void swap(features_value_index_iterator& lhs, features_value_index_iterator& rhs)
  {
    swap(static_cast<features_value_iterator&>(lhs), static_cast<features_value_iterator&>(rhs));
    std::swap(lhs._begin_index, rhs._begin_index);
  }
};

/// iterator over values, indicies and audit space names
class features_value_index_audit_iterator : public features_value_index_iterator
{
protected:
  audit_strings_ptr* _begin_audit;

public:
  features_value_index_audit_iterator(feature_value* begin, feature_index* begin_index, audit_strings_ptr* begin_audit)
      : features_value_index_iterator(begin, begin_index), _begin_audit(begin_audit)
  {
  }

  features_value_index_audit_iterator(const features_value_index_audit_iterator&) = default;
  features_value_index_audit_iterator& operator=(const features_value_index_audit_iterator&) = default;
  features_value_index_audit_iterator(features_value_index_audit_iterator&&) = default;
  features_value_index_audit_iterator& operator=(features_value_index_audit_iterator&&) = default;

  // prefix increment
  inline features_value_index_audit_iterator& operator++()
  {
    features_value_index_iterator::operator++();
    if (_begin_audit != nullptr) { _begin_audit++; }
    return *this;
  }

  inline audit_strings_ptr* audit() { return _begin_audit; }

  inline features_value_index_audit_iterator& operator+=(std::ptrdiff_t index)
  {
    features_value_index_iterator::operator+=(index);
    if (_begin_audit != nullptr) { _begin_audit += index; }
    return *this;
  }

  inline features_value_index_audit_iterator operator+(std::ptrdiff_t index)
  {
    return {_begin + index, _begin_index + index, _begin_audit + index};
  }

  inline features_value_index_audit_iterator& operator-=(std::ptrdiff_t index)
  {
    features_value_index_iterator::operator-=(index);
    _begin_audit += index;
    return *this;
  }

  inline features_value_index_audit_iterator& operator*() { return *this; }

  friend void swap(features_value_index_audit_iterator& lhs, features_value_index_audit_iterator& rhs)
  {
    swap(static_cast<features_value_index_iterator&>(lhs), static_cast<features_value_index_iterator&>(rhs));
    std::swap(lhs._begin_audit, rhs._begin_audit);
  }
};

/// the core definition of a set of features.
struct features
{
  using iterator = features_value_index_iterator;
  using iterator_value = features_value_iterator;
  using iterator_all = features_value_index_audit_iterator;

  v_array<feature_value> values;               // Always needed.
  v_array<feature_index> indicies;             // Optional for sparse data.
  std::vector<audit_strings_ptr> space_names;  // Optional for audit mode.

  float sum_feat_sq;

  /// defines a "range" usable by C++ 11 for loops
  class features_value_index_audit_range
  {
  private:
    features* _outer;

  public:
    features_value_index_audit_range(features* outer) : _outer(outer) {}

    inline features_value_index_audit_iterator begin()
    {
      return {_outer->values.begin(), _outer->indicies.begin(), _outer->space_names.data()};
    }
    inline features_value_index_audit_iterator end()
    {
      return {_outer->values.end(), _outer->indicies.end(), _outer->space_names.data() + _outer->space_names.size()};
    }
  };

  features();
  ~features();
  features(const features&) = delete;
  features& operator=(const features&) = delete;

  // custom move operators required since we need to leave the old value in
  // a null state to prevent freeing of shallow copied v_arrays
  features(features&& other) noexcept;
  features& operator=(features&& other) noexcept;

  inline size_t size() const { return values.size(); }

  inline bool nonempty() const { return !values.empty(); }

  VW_DEPRECATED("Freeing space names is handled directly by truncation or removal.")
  void free_space_names(size_t i);

  inline features_value_index_audit_range values_indices_audit() { return features_value_index_audit_range{this}; }
  // default iterator for values & features
  inline iterator begin() { return {values.begin(), indicies.begin()}; }
  inline iterator end() { return {values.end(), indicies.end()}; }

  void clear();
  void truncate_to(const features_value_iterator& pos);
  void truncate_to(size_t i);
  void push_back(feature_value v, feature_index i);
  bool sort(uint64_t parse_mask);
  void deep_copy_from(const features& src);
};
