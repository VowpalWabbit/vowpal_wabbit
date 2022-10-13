// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/common/vw_exception.h"
#include "vw/core/constant.h"

#include <cstddef>
#include <functional>
#include <unordered_map>

class sparse_parameters;
using weight_map = std::unordered_map<uint64_t, weight*>;

template <typename T>
class sparse_iterator
{
public:
  using iterator_category = std::forward_iterator_tag;
  using value_type = T;
  using difference_type = ptrdiff_t;
  using pointer = T*;
  using reference = T&;

  sparse_iterator(weight_map::iterator iter, uint64_t stride) : _iter(iter), _stride(stride) {}

  sparse_iterator& operator=(const sparse_iterator& other) = default;
  sparse_iterator(const sparse_iterator& other) = default;
  sparse_iterator& operator=(sparse_iterator&& other) noexcept = default;
  sparse_iterator(sparse_iterator&& other) noexcept = default;

  uint64_t index() { return _iter->first; }

  T& operator*() { return *(_iter->second); }

  sparse_iterator& operator++()
  {
    _iter++;
    return *this;
  }

  bool operator==(const sparse_iterator& rhs) const { return _iter == rhs._iter; }
  bool operator!=(const sparse_iterator& rhs) const { return _iter != rhs._iter; }

private:
  weight_map::iterator _iter;
  uint32_t _stride;
};

class sparse_parameters
{
public:
  using iterator = sparse_iterator<weight>;
  using const_iterator = sparse_iterator<const weight>;

  sparse_parameters(size_t length, uint32_t stride_shift = 0);
  sparse_parameters();
  ~sparse_parameters();

  sparse_parameters(const sparse_parameters& other) = delete;
  sparse_parameters& operator=(const sparse_parameters& other) = delete;
  sparse_parameters& operator=(sparse_parameters&&) noexcept = delete;
  sparse_parameters(sparse_parameters&&) noexcept = delete;

  bool not_null() { return (_weight_mask > 0 && !_map.empty()); }
  weight* first() { THROW_OR_RETURN("Allreduce currently not supported in sparse", nullptr); }

  // iterator with stride
  iterator begin() { return iterator(_map.begin(), stride()); }
  iterator end() { return iterator(_map.begin(), stride()); }

  // const iterator
  const_iterator cbegin() const { return const_iterator(_map.begin(), stride()); }
  const_iterator cend() const { return const_iterator(_map.begin(), stride()); }

  inline weight& operator[](size_t i) { return *(get_or_default_and_get(i)); }

  inline const weight& operator[](size_t i) const { return *(get_or_default_and_get(i)); }

  inline weight& strided_index(size_t index) { return operator[](index << _stride_shift); }
  inline const weight& strided_index(size_t index) const { return operator[](index << _stride_shift); }

  void shallow_copy(const sparse_parameters& input);

  template <typename Lambda>
  void set_default(Lambda&& default_func)
  {
    _default_func = default_func;
  }

  void set_zero(size_t offset);
  uint64_t mask() const { return _weight_mask; }

  uint64_t seeded() const { return _seeded; }

  uint64_t stride() const { return static_cast<uint64_t>(1) << _stride_shift; }

  uint32_t stride_shift() const { return _stride_shift; }

  void stride_shift(uint32_t stride_shift) { _stride_shift = stride_shift; }

#ifndef _WIN32
  void share(size_t /* length */);
#endif

private:
  // This must be mutable because the const operator[] must be able to intialize default weights to return.
  mutable weight_map _map;
  uint64_t _weight_mask;  // (stride*(1 << num_bits) -1)
  uint32_t _stride_shift;
  bool _seeded;  // whether the instance is sharing model state with others
  bool _delete;
  std::function<void(weight*, uint64_t)> _default_func;

  // It is marked const so it can be used from both const and non const operator[]
  // The map itself is mutable to facilitate this
  weight* get_or_default_and_get(size_t i) const;
};
