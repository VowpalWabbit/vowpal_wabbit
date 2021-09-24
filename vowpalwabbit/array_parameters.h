// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <unordered_map>
#include <cstddef>
#include <functional>

#ifndef _WIN32
#  define NOMINMAX
#  include <sys/mman.h>
#endif

// It appears that on OSX MAP_ANONYMOUS is mapped to MAP_ANON
// https://github.com/leftmike/foment/issues/4
#ifdef __APPLE__
#  define MAP_ANONYMOUS MAP_ANON
#endif

#include "array_parameters_dense.h"
#include "vw_exception.h"

class sparse_parameters;
using weight_map = std::unordered_map<uint64_t, weight *>;

template <typename T>
class sparse_iterator
{
private:
  weight_map::iterator _iter;
  uint32_t _stride;

public:
  using iterator_category = std::forward_iterator_tag;
  using value_type = T;
  using difference_type = ptrdiff_t;
  using pointer = T *;
  using reference = T &;

  sparse_iterator(weight_map::iterator& iter, uint32_t stride) : _iter(iter), _stride(stride) {}

  sparse_iterator& operator=(const sparse_iterator& other) = default;
  sparse_iterator(const sparse_iterator& other) = default;
  sparse_iterator& operator=(sparse_iterator&& other) = default;
  sparse_iterator(sparse_iterator&& other) = default;

  uint64_t index() { return _iter->first; }

  T& operator*() { return *(_iter->second); }

  sparse_iterator& operator++()
  {
    _iter++;
    return *this;
  }

  bool operator==(const sparse_iterator& rhs) const { return _iter == rhs._iter; }
  bool operator!=(const sparse_iterator& rhs) const { return _iter != rhs._iter; }
};

class sparse_parameters
{
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
  inline weight* get_or_default_and_get(size_t i) const
  {
    uint64_t index = i & _weight_mask;
    weight_map::iterator iter = _map.find(index);
    if (iter == _map.end())
    {
      _map.insert(std::make_pair(index, calloc_mergable_or_throw<weight>(stride())));
      iter = _map.find(index);
      if (_default_func != nullptr) { _default_func(iter->second, index); }
    }
    return iter->second;
  }

public:
  using iterator = sparse_iterator<weight>;
  using const_iterator = sparse_iterator<const weight>;

  sparse_parameters(size_t length, uint32_t stride_shift = 0)
      : _map()
      , _weight_mask((length << stride_shift) - 1)
      , _stride_shift(stride_shift)
      , _seeded(false)
      , _delete(false)
      , _default_func(nullptr)
  {
  }

  sparse_parameters()
      : _map(), _weight_mask(0), _stride_shift(0), _seeded(false), _delete(false), _default_func(nullptr)
  {
  }

  bool not_null() { return (_weight_mask > 0 && !_map.empty()); }

  sparse_parameters(const sparse_parameters& other) = delete;
  sparse_parameters& operator=(const sparse_parameters& other) = delete;
  sparse_parameters& operator=(sparse_parameters&&) noexcept = delete;
  sparse_parameters(sparse_parameters&&) noexcept = delete;

  weight* first() { THROW_OR_RETURN("Allreduce currently not supported in sparse", nullptr); }

  // iterator with stride
  iterator begin()
  {
    weight_map::iterator i = _map.begin();
    return iterator(i, stride());
  }
  iterator end()
  {
    weight_map::iterator i = _map.end();
    return iterator(i, stride());
  }

  // const iterator
  const_iterator cbegin()
  {
    weight_map::iterator i = _map.begin();
    return const_iterator(i, stride());
  }
  const_iterator cend()
  {
    weight_map::iterator i = _map.begin();
    return const_iterator(i, stride());
  }

  inline weight& operator[](size_t i) { return *(get_or_default_and_get(i)); }

  inline const weight& operator[](size_t i) const { return *(get_or_default_and_get(i)); }

  inline weight& strided_index(size_t index) { return operator[](index << _stride_shift); }

  void shallow_copy(const sparse_parameters& input)
  {
    // TODO: this is level-1 copy (weight* are stilled shared)
    if (!_seeded)
    {
      for (auto& iter : _map)
      {
        auto* weight_ptr = iter.second;
        free(weight_ptr);
      }
    }
    _map = input._map;
    _weight_mask = input._weight_mask;
    _stride_shift = input._stride_shift;
    _seeded = true;
  }

  template <typename Lambda>
  void set_default(Lambda&& default_func)
  {
    _default_func = default_func;
  }

  void set_zero(size_t offset)
  {
    for (auto& iter : _map) { (&(*(iter.second)))[offset] = 0; }
  }

  uint64_t mask() const { return _weight_mask; }

  uint64_t seeded() const { return _seeded; }

  uint32_t stride() const { return 1 << _stride_shift; }

  uint32_t stride_shift() const { return _stride_shift; }

  void stride_shift(uint32_t stride_shift) { _stride_shift = stride_shift; }

#ifndef _WIN32
  void share(size_t /* length */) { THROW_OR_RETURN("Operation not supported on Windows"); }
#endif

  ~sparse_parameters()
  {
    if (!_delete && !_seeded)  // don't free weight vector if it is shared with another instance
    {
      for (auto& iter : _map)
      {
        auto* weight_ptr = iter.second;
        free(weight_ptr);
      }
      _map.clear();
      _delete = true;
    }
  }
};

class parameters
{
public:
  bool adaptive;
  bool normalized;

  bool sparse;
  dense_parameters dense_weights;
  sparse_parameters sparse_weights;

  inline weight& operator[](size_t i)
  {
    if (sparse)
      return sparse_weights[i];
    else
      return dense_weights[i];
  }

  template <typename Lambda>
  void set_default(Lambda&& default_func)
  {
    if (sparse) { sparse_weights.set_default(std::forward<Lambda>(default_func)); }
    else
    {
      dense_weights.set_default(std::forward<Lambda>(default_func));
    }
  }

  inline uint32_t stride_shift() const
  {
    if (sparse)
      return sparse_weights.stride_shift();
    else
      return dense_weights.stride_shift();
  }

  inline uint32_t stride() const
  {
    if (sparse)
      return sparse_weights.stride();
    else
      return dense_weights.stride();
  }

  inline uint64_t mask() const
  {
    if (sparse)
      return sparse_weights.mask();
    else
      return dense_weights.mask();
  }

  inline uint64_t seeded() const
  {
    if (sparse)
      return sparse_weights.seeded();
    else
      return dense_weights.seeded();
  }

  inline void shallow_copy(const parameters& input)
  {
    if (sparse)
      sparse_weights.shallow_copy(input.sparse_weights);
    else
      dense_weights.shallow_copy(input.dense_weights);
  }

  inline void set_zero(size_t offset)
  {
    if (sparse)
      sparse_weights.set_zero(offset);
    else
      dense_weights.set_zero(offset);
  }
#ifndef _WIN32
#  ifndef DISABLE_SHARED_WEIGHTS
  inline void share(size_t length)
  {
    if (sparse)
      sparse_weights.share(length);
    else
      dense_weights.share(length);
  }
#  endif
#endif

  inline void stride_shift(uint32_t stride_shift)
  {
    if (sparse)
      sparse_weights.stride_shift(stride_shift);
    else
      dense_weights.stride_shift(stride_shift);
  }

  inline weight& strided_index(size_t index)
  {
    if (sparse)
      return sparse_weights.strided_index(index);
    else
      return dense_weights.strided_index(index);
  }

  inline bool not_null()
  {
    if (sparse)
      return sparse_weights.not_null();
    else
      return dense_weights.not_null();
  }
};
