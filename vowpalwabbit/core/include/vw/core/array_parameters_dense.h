// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <cstdint>
#ifndef _WIN32
#  include <sys/mman.h>
#endif

#include "vw/core/memory.h"

#include <cassert>

// It appears that on OSX MAP_ANONYMOUS is mapped to MAP_ANON
// https://github.com/leftmike/foment/issues/4
#ifdef __APPLE__
#  define MAP_ANONYMOUS MAP_ANON
#endif

using weight = float;

template <typename T>
class dense_iterator
{
private:
  T* _current;
  T* _begin;
  uint32_t _stride;
  uint32_t _stride_shift;

public:
  using iterator_category = std::forward_iterator_tag;
  using value_type = T;
  using difference_type = std::ptrdiff_t;
  using pointer = T*;
  using reference = T&;

  dense_iterator(T* current, T* begin, uint32_t stride_shift)
      : _current(current), _begin(begin), _stride(1 << stride_shift), _stride_shift(stride_shift)
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
};

class dense_parameters
{
private:
  weight* _begin;
  uint64_t _weight_mask;  // (stride*(1 << num_bits) -1)
  uint32_t _stride_shift;
  bool _seeded;  // whether the instance is sharing model state with others

public:
  using iterator = dense_iterator<weight>;
  using const_iterator = dense_iterator<const weight>;
  dense_parameters(size_t length, uint32_t stride_shift = 0)
      : _begin(calloc_mergable_or_throw<weight>(length << stride_shift))
      , _weight_mask((length << stride_shift) - 1)
      , _stride_shift(stride_shift)
      , _seeded(false)
  {
  }

  dense_parameters()
      : _begin(nullptr)
      , _weight_mask(0)
      , _stride_shift(0)
      , _seeded(false)
  {
  }

  bool not_null() { return (_weight_mask > 0 && _begin != nullptr); }

  dense_parameters(const dense_parameters& other) = delete;
  dense_parameters& operator=(const dense_parameters& other) = delete;
  dense_parameters& operator=(dense_parameters&&) noexcept = delete;
  dense_parameters(dense_parameters&&) noexcept = delete;

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
  inline weight& operator[](size_t i)
  {
    return _begin[i & _weight_mask];
  }

  void shallow_copy(const dense_parameters& input)
  {
    if (!_seeded) { free(_begin); }
    _begin = input._begin;
    _weight_mask = input._weight_mask;
    _stride_shift = input._stride_shift;
    _seeded = true;
  }

  inline weight& strided_index(size_t index) { return operator[](index << _stride_shift); }
  inline const weight& strided_index(size_t index) const { return operator[](index << _stride_shift); }

  template <typename Lambda>
  void set_default(Lambda&& default_func)
  {
    auto iter = begin();
    for (size_t i = 0; iter != end(); ++iter, i += stride())
    {
      // Types are required to be weight* and uint64_t.
      default_func(&(*iter), iter.index());
    }
  }

  void set_zero(size_t offset)
  {
    for (iterator iter = begin(); iter != end(); ++iter) { (&(*iter))[offset] = 0; }
  }

  void move_offsets(const size_t from, const size_t to, const size_t params_per_problem, bool swap = false)
  {
    assert(from < params_per_problem);
    assert(to < params_per_problem);

    auto iterator_from = begin() + from;
    auto iterator_to = begin() + to;

    for (; iterator_from < end(); iterator_from += params_per_problem, iterator_to += params_per_problem)
    {
      assert((iterator_to.index_without_stride() & (params_per_problem - 1)) == to);
      assert((iterator_from.index_without_stride() & (params_per_problem - 1)) == from);

      for (size_t stride_offset = 0; stride_offset < stride(); stride_offset++)
      {
        if (*iterator_to[stride_offset] != *iterator_from[stride_offset])
        {
          if (swap) { std::swap(*iterator_to[stride_offset], *iterator_from[stride_offset]); }
          else
          {
            *iterator_to[stride_offset] = *iterator_from[stride_offset];
          }
        }
      }
    }
  }

  // ***** NOTE: params_per_problem must be of form 2^n *****
  void clear_offset(const size_t offset, const size_t params_per_problem)
  {
    assert(offset < params_per_problem);

    for (auto iterator_clear = begin() + offset; iterator_clear < end(); iterator_clear += params_per_problem)
    {
      assert((iterator_clear.index_without_stride() & (params_per_problem - 1)) == offset);
      for (size_t stride_offset = 0; stride_offset < stride(); stride_offset++)
      {
        if (*iterator_clear[stride_offset] != 0.0f) { *iterator_clear[stride_offset] = 0.0f; }
      }
    }
  }

  uint64_t mask() const { return _weight_mask; }

  uint64_t seeded() const { return _seeded; }

  uint32_t stride() const { return 1 << _stride_shift; }

  uint32_t stride_shift() const { return _stride_shift; }

  void stride_shift(uint32_t stride_shift) { _stride_shift = stride_shift; }

#ifndef _WIN32
#  ifndef DISABLE_SHARED_WEIGHTS
  void share(size_t length)
  {
    float* shared_weights = static_cast<float*>(mmap(
        nullptr, (length << _stride_shift) * sizeof(float), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0));
    size_t float_count = length << _stride_shift;
    weight* dest = shared_weights;
    memcpy(dest, _begin, float_count * sizeof(float));
    free(_begin);
    _begin = dest;
  }
#  endif
#endif

  ~dense_parameters()
  {
    if (_begin != nullptr && !_seeded)  // don't free weight vector if it is shared with another instance
    {
      free(_begin);
      _begin = nullptr;
    }
  }
};
