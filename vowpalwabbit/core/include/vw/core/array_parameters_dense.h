// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/core/constant.h"

#include <cassert>
#include <cstdint>
#include <iterator>

template <typename T>
class dense_iterator
{
public:
  using iterator_category = std::forward_iterator_tag;
  using value_type = T;
  using difference_type = std::ptrdiff_t;
  using pointer = T*;
  using reference = T&;

  dense_iterator(T* current, T* begin, uint32_t stride_shift)
      : _current(current), _begin(begin), _stride(static_cast<uint64_t>(1) << stride_shift), _stride_shift(stride_shift)
  {
  }

  T& operator*() { return *_current; }

  size_t index() { return _current - _begin; }

  size_t index_without_stride() { return (index() >> _stride_shift); }

  dense_iterator& operator++()
  {
    _current += _stride;
    return *this;
  }

  dense_iterator& operator+(size_t n)
  {
    _current += _stride * n;
    return *this;
  }

  dense_iterator& operator+=(size_t n)
  {
    _current += _stride * n;
    return *this;
  }

  dense_iterator& next_non_zero(const dense_iterator& end)
  {
    while (_current + _stride < end._current)
    {
      _current += _stride;
      if (*_current != 0.0f) { return *this; }
    }
    _current = end._current;
    return *this;
  }

  // ignores the stride
  pointer operator[](size_t n)
  {
    assert(n < _stride);
    return _current + n;
  }

  bool operator==(const dense_iterator& rhs) const { return _current == rhs._current; }
  bool operator!=(const dense_iterator& rhs) const { return _current != rhs._current; }
  bool operator<(const dense_iterator& rhs) const { return _current < rhs._current; }
  bool operator<=(const dense_iterator& rhs) const { return _current <= rhs._current; }

private:
  T* _current;
  T* _begin;
  uint64_t _stride;
  uint32_t _stride_shift;
};

class dense_parameters
{
public:
  using iterator = dense_iterator<weight>;
  using const_iterator = dense_iterator<const weight>;

  dense_parameters(size_t length, uint32_t stride_shift = 0);
  dense_parameters();
  ~dense_parameters();

  dense_parameters(const dense_parameters& other) = delete;
  dense_parameters& operator=(const dense_parameters& other) = delete;
  dense_parameters& operator=(dense_parameters&&) noexcept = delete;
  dense_parameters(dense_parameters&&) noexcept = delete;

  bool not_null();
  weight* first()
  {
    return _begin;
  }  // TODO: Temporary fix for allreduce.
     // iterator with stride
  iterator begin() { return iterator(_begin, _begin, stride_shift()); }
  iterator end() { return iterator(_begin + _weight_mask + 1, _begin, stride_shift()); }

  // const iterator
  const_iterator cbegin() const { return const_iterator(_begin, _begin, stride_shift()); }
  const_iterator cend() const { return const_iterator(_begin + _weight_mask + 1, _begin, stride_shift()); }

  inline const weight& operator[](size_t i) const { return _begin[i & _weight_mask]; }
  inline weight& operator[](size_t i) { return _begin[i & _weight_mask]; }

  void shallow_copy(const dense_parameters& input);

  inline weight& strided_index(size_t index) { return operator[](index << _stride_shift); }
  inline const weight& strided_index(size_t index) const { return operator[](index << _stride_shift); }

  template <typename Lambda>
  void set_default(Lambda&& default_func)
  {
    if (not_null())
    {
      auto iter = begin();
      for (size_t i = 0; iter != end(); ++iter, i += stride())
      {
        // Types are required to be weight* and uint64_t.
        default_func(&(*iter), iter.index());
      }
    }
  }

  void set_zero(size_t offset);
  void move_offsets(const size_t from, const size_t to, const size_t params_per_problem, bool swap = false);

  // ***** NOTE: params_per_problem must be of form 2^n *****
  void clear_offset(const size_t offset, const size_t params_per_problem);

  uint64_t mask() const { return _weight_mask; }

  uint64_t seeded() const { return _seeded; }

  uint64_t stride() const { return static_cast<uint64_t>(1) << _stride_shift; }

  uint32_t stride_shift() const { return _stride_shift; }

  void stride_shift(uint32_t stride_shift) { _stride_shift = stride_shift; }

#ifndef _WIN32
#  ifndef DISABLE_SHARED_WEIGHTS
  void share(size_t length);
#  endif
#endif

private:
  weight* _begin;
  uint64_t _weight_mask;  // (stride*(1 << num_bits) -1)
  uint32_t _stride_shift;
  bool _seeded;  // whether the instance is sharing model state with others
};
