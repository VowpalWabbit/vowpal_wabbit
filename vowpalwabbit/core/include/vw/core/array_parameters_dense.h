// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/core/constant.h"

#include <cassert>
#include <cstdint>
#include <iterator>
#include <memory>

namespace VW
{

namespace details
{

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
}  // namespace details

class dense_parameters
{
public:
  using iterator = details::dense_iterator<VW::weight>;
  using const_iterator = details::dense_iterator<const VW::weight>;

  dense_parameters(size_t length, uint32_t stride_shift = 0);
  dense_parameters();

  dense_parameters(const dense_parameters& other) = delete;
  dense_parameters& operator=(const dense_parameters& other) = delete;
  dense_parameters& operator=(dense_parameters&&) noexcept;
  dense_parameters(dense_parameters&&) noexcept;

  bool not_null();
  VW::weight* first()
  {
    return _begin.get();
  }  // TODO: Temporary fix for allreduce.

  VW::weight* data() { return _begin.get(); }

  const VW::weight* data() const { return _begin.get(); }

  // iterator with stride
  iterator begin() { return iterator(_begin.get(), _begin.get(), stride_shift()); }
  iterator end() { return iterator(_begin.get() + _weight_mask + 1, _begin.get(), stride_shift()); }

  // const iterator
  const_iterator cbegin() const { return const_iterator(_begin.get(), _begin.get(), stride_shift()); }
  const_iterator cend() const { return const_iterator(_begin.get() + _weight_mask + 1, _begin.get(), stride_shift()); }

  inline const VW::weight& operator[](size_t i) const { return _begin.get()[i & _weight_mask]; }
  inline VW::weight& operator[](size_t i) { return _begin.get()[i & _weight_mask]; }

  VW_ATTR(nodiscard) static dense_parameters shallow_copy(const dense_parameters& input);
  VW_ATTR(nodiscard) static dense_parameters deep_copy(const dense_parameters& input);

  inline VW::weight& strided_index(size_t index) { return operator[](index << _stride_shift); }
  inline const VW::weight& strided_index(size_t index) const { return operator[](index << _stride_shift); }

  template <typename Lambda>
  void set_default(Lambda&& default_func)
  {
    if (not_null())
    {
      auto iter = begin();
      for (size_t i = 0; iter != end(); ++iter, i += stride())
      {
        // Types are required to be VW::weight* and uint64_t.
        default_func(&(*iter), iter.index());
      }
    }
  }

  void set_zero(size_t offset);

  uint64_t mask() const { return _weight_mask; }

  uint64_t raw_length() const { return _weight_mask + 1; }

  uint64_t stride() const { return static_cast<uint64_t>(1) << _stride_shift; }

  uint32_t stride_shift() const { return _stride_shift; }

  void stride_shift(uint32_t stride_shift) { _stride_shift = stride_shift; }

#ifndef _WIN32
#  ifndef DISABLE_SHARED_WEIGHTS
  void share(size_t length);
#  endif
#endif

private:
  std::shared_ptr<VW::weight> _begin;
  uint64_t _weight_mask;  // (stride*(1 << num_bits) -1)
  uint32_t _stride_shift;
};
}  // namespace VW
using dense_parameters VW_DEPRECATED("dense_parameters moved into VW namespace") = VW::dense_parameters;
