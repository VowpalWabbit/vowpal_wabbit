// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <memory>
#include <string>
#include <cstddef>
#include <cstdint>
#include "v_array.h"
#include <algorithm>
#include <vector>

#ifndef _WIN32
#include <sys/types.h>
#else
#define ssize_t int64_t
#endif
 
typedef float feature_value;
typedef uint64_t feature_index;
typedef std::pair<std::string, std::string> audit_strings;
typedef std::shared_ptr<audit_strings> audit_strings_ptr;

struct feature  // sparse feature definition for the library interface
{
  float x;
  uint64_t weight_index;
  feature(float _x, uint64_t _index) : x(_x), weight_index(_index) {}
  feature() : x(0.f), weight_index(0) {}
};

struct feature_slice  // a helper struct for functions using the set {v,i,space_name}
{
  feature_value x;
  feature_index weight_index;
  audit_strings space_name;
};

template <class T>
inline int order_features(const void* first, const void* second)
{
  if (((T*)first)->weight_index != ((T*)second)->weight_index)
    return (int)(((T*)first)->weight_index - ((T*)second)->weight_index);
  else if (((T*)first)->x > ((T*)second)->x)
    return 1;
  else
    return -1;
}

struct features;

/// iterator over feature values only
class features_value_iterator
{
 protected:
  feature_value* _begin;

 public:
  features_value_iterator(feature_value* begin) : _begin(begin) {}

  features_value_iterator(const features_value_iterator& other) : _begin(other._begin) {}

  features_value_iterator& operator++()
  {
    _begin++;
    return *this;
  }

  /// \return reference to the current value
  inline feature_value& value() { return *_begin; }

  /// creates a new iterator advanced by \p index
  /// \remark template<typename T> used to avoid warnings or tons of overloads for int, size_t, ...
  template <typename T>
  features_value_iterator operator+(T index)
  {
    return features_value_iterator(_begin + index);
  }

  template <typename T>
  features_value_iterator& operator+=(T index)
  {
    _begin += index;
    return *this;
  }

  template <typename T>
  features_value_iterator& operator-=(T index)
  {
    _begin -= index;
    return *this;
  }

  features_value_iterator& operator=(const features_value_iterator& other)
  {
    _begin = other._begin;
    return *this;
  }

  features_value_iterator& operator*() { return *this; }

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

  features_value_index_iterator(const features_value_index_iterator& other)
      : features_value_iterator(other), _begin_index(other._begin_index)
  {
  }

  features_value_index_iterator& operator++()
  {
    features_value_iterator::operator++();
    _begin_index++;
    return *this;
  }

  inline feature_index& index() { return *_begin_index; }

  template <typename T>
  features_value_index_iterator& operator+=(T index)
  {
    features_value_iterator::operator+=(index);
    _begin_index += index;
    return *this;
  }

  template <typename T>
  features_value_index_iterator operator+(T index)
  {
    return features_value_index_iterator(_begin + index, _begin_index + index);
  }

  template <typename T>
  features_value_index_iterator& operator-=(T index)
  {
    features_value_iterator::operator-=(index);
    _begin_index -= index;
    return *this;
  }

  features_value_index_iterator& operator=(const features_value_index_iterator& other)
  {
    features_value_iterator::operator=(other);
    _begin_index = other._begin_index;
    return *this;
  }

  features_value_index_iterator& operator*() { return *this; }

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

  features_value_index_audit_iterator(const features_value_index_audit_iterator& other)
      : features_value_index_iterator(other), _begin_audit(other._begin_audit)
  {
  }

  // prefix increment
  features_value_index_audit_iterator& operator++()
  {
    features_value_index_iterator::operator++();
    _begin_audit++;
    return *this;
  }

  inline audit_strings_ptr& audit() { return *_begin_audit; }

  template <typename T>
  features_value_index_audit_iterator& operator+=(T index)
  {
    features_value_index_iterator::operator+=(index);
    _begin_audit += index;
    return *this;
  }

  template <typename T>
  features_value_index_audit_iterator operator+(T index)
  {
    return features_value_index_audit_iterator(_begin + index, _begin_index + index, _begin_audit + index);
  }

  template <typename T>
  features_value_index_audit_iterator& operator-=(T index)
  {
    features_value_index_iterator::operator-=(index);
    _begin_audit += index;
    return *this;
  }

  features_value_index_audit_iterator& operator=(const features_value_index_audit_iterator& other)
  {
    features_value_index_iterator::operator=(other);
    _begin_audit = other._begin_audit;
    return *this;
  }

  features_value_index_audit_iterator& operator*() { return *this; }

  friend void swap(features_value_index_audit_iterator& lhs, features_value_index_audit_iterator& rhs)
  {
    swap(static_cast<features_value_index_iterator&>(lhs), static_cast<features_value_index_iterator&>(rhs));
    std::swap(lhs._begin_audit, rhs._begin_audit);
  }
};

/// the core definition of a set of features.
struct features
{
  v_array<feature_value> values;           // Always needed.
  v_array<feature_index> indicies;         // Optional for sparse data.
  v_array<audit_strings_ptr> space_names;  // Optional for audit mode.

  float sum_feat_sq;

  typedef features_value_index_iterator iterator;
  typedef features_value_iterator iterator_value;
  typedef features_value_index_audit_iterator iterator_all;

  /// defines a "range" usable by C++ 11 for loops
  class features_value_index_audit_range
  {
   private:
    features* _outer;

   public:
    features_value_index_audit_range(features* outer) : _outer(outer) {}

    iterator_all begin()
    {
      return iterator_all(_outer->values.begin(), _outer->indicies.begin(), _outer->space_names.begin());
    }
    iterator_all end() { return iterator_all(_outer->values.end(), _outer->indicies.end(), _outer->space_names.end()); }
  };

  features()
  {
    values = v_init<feature_value>();
    indicies = v_init<feature_index>();
    space_names = v_init<audit_strings_ptr>();
    sum_feat_sq = 0.f;
  }

  ~features() { 
     values.delete_v();
     indicies.delete_v();
     space_names.delete_v();
   }
   features(const features&) = delete;
   features & operator=( const features& ) = delete;
   
   
   // custom move operators required since we need to leave the old value in
   // a null state to prevent freeing of shallow copied v_arrays
   features(features&& other) :
   values(std::move(other.values)),
   indicies(std::move(other.indicies)),
   space_names(std::move(other.space_names)),
   sum_feat_sq(other.sum_feat_sq)
   {
     // We need to null out all the v_arrays to prevent double freeing during moves
     auto & v = other.values;
     v._begin = nullptr;
     v._end = nullptr;
     v.end_array = nullptr;
     auto & i = other.indicies;
     i._begin = nullptr;
     i._end = nullptr;
     i.end_array = nullptr;
     auto & s = other.space_names;
     s._begin = nullptr;
     s._end = nullptr;
     s.end_array = nullptr;
     other.sum_feat_sq = 0;
   }
   features & operator=(features&& other)
   {
     values = std::move(other.values);
     indicies = std::move(other.indicies);
     space_names = std::move(other.space_names);
     sum_feat_sq = other.sum_feat_sq;
     // We need to null out all the v_arrays to prevent double freeing during moves
     auto & v = other.values;
     v._begin = nullptr;
     v._end = nullptr;
     v.end_array = nullptr;
     auto & i = other.indicies;
     i._begin = nullptr;
     i._end = nullptr;
     i.end_array = nullptr;
     auto & s = other.space_names;
     s._begin = nullptr;
     s._end = nullptr;
     s.end_array = nullptr;
     other.sum_feat_sq = 0;
     return *this;
   }

  inline size_t size() const { return values.size(); }

  inline bool nonempty() const { return !values.empty(); }

  void free_space_names(size_t i)
  {
    for (; i < space_names.size(); i++) space_names[i].~audit_strings_ptr();
  }

  features_value_index_audit_range values_indices_audit() { return {this}; }

  // default iterator for values & features
  iterator begin() { return iterator(values.begin(), indicies.begin()); }

  iterator end() { return iterator(values.end(), indicies.end()); }

  void clear()
  {
    sum_feat_sq = 0.f;
    values.clear();
    indicies.clear();
    space_names.clear();
  }

  void truncate_to(const features_value_iterator& pos)
  {
    ssize_t i = pos._begin - values.begin();
    values.end() = pos._begin;
    if (indicies.end() != indicies.begin())
      indicies.end() = indicies.begin() + i;
    if (space_names.begin() != space_names.end())
    {
      free_space_names((size_t)i);
      space_names.end() = space_names.begin() + i;
    }
  }

  void truncate_to(size_t i)
  {
    values.end() = values.begin() + i;
    if (indicies.end() != indicies.begin())
      indicies.end() = indicies.begin() + i;
    if (space_names.begin() != space_names.end())
    {
      free_space_names(i);
      space_names.end() = space_names.begin() + i;
    }
  }

  void push_back(feature_value v, feature_index i)
  {
    values.push_back(v);
    indicies.push_back(i);
    sum_feat_sq += v * v;
  }

  bool sort(uint64_t parse_mask)
  {
    if (indicies.empty())
      return false;

    if (!space_names.empty())
    {
      std::vector<feature_slice> slice;
      slice.reserve(indicies.size());
      for (size_t i = 0; i < indicies.size(); i++)
      {
        slice.push_back({values[i], indicies[i] & parse_mask, *space_names[i].get()});
      }
      // The comparator should return true if the first element is less than the second.
      std::sort(slice.begin(), slice.end(), [](const feature_slice& first, const feature_slice& second) {
        return (first.weight_index < second.weight_index) ||
            ((first.weight_index == second.weight_index) && (first.x < second.x));
      });

      for (size_t i = 0; i < slice.size(); i++)
      {
        values[i] = slice[i].x;
        indicies[i] = slice[i].weight_index;
        *space_names[i].get() = slice[i].space_name;
      }
    }
    else
    {
      std::vector<feature> slice;
      slice.reserve(indicies.size());

      for (size_t i = 0; i < indicies.size(); i++)
      {
        slice.push_back({values[i], indicies[i] & parse_mask});
      }
      // The comparator should return true if the first element is less than the second.
      std::sort(slice.begin(), slice.end(), [](const feature& first, const feature& second) {
        return (first.weight_index < second.weight_index) ||
            ((first.weight_index == second.weight_index) && (first.x < second.x));
      });
      for (size_t i = 0; i < slice.size(); i++)
      {
        values[i] = slice[i].x;
        indicies[i] = slice[i].weight_index;
      }
    }
    return true;
  }

  void deep_copy_from(const features& src)
  {
    copy_array(values, src.values);
    copy_array(indicies, src.indicies);
    copy_array_no_memcpy(space_names, src.space_names);
    sum_feat_sq = src.sum_feat_sq;
  }
};
