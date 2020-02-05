// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <unordered_map>
#include <cstddef>

#ifndef _WIN32
#define NOMINMAX
#include <sys/mman.h>
#endif

// It appears that on OSX MAP_ANONYMOUS is mapped to MAP_ANON
// https://github.com/leftmike/foment/issues/4
#ifdef __APPLE__
#define MAP_ANONYMOUS MAP_ANON
#endif

#include "array_parameters_dense.h"
#include "vw_exception.h"

class sparse_parameters;
typedef std::unordered_map<uint64_t, weight*> weight_map;

template <typename T>
class sparse_iterator
{
 private:
  weight_map::iterator _iter;
  uint32_t _stride;

 public:
  typedef std::forward_iterator_tag iterator_category;
  typedef T value_type;
  typedef ptrdiff_t difference_type;
  typedef T* pointer;
  typedef T& reference;

  sparse_iterator(weight_map::iterator& iter, uint32_t stride) : _iter(iter), _stride(stride) {}

  sparse_iterator& operator=(const sparse_iterator& other)
  {
    _iter = other._iter;
    _stride = other._stride;
    return *this;
  }
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
  weight_map _map;
  uint64_t _weight_mask;  // (stride*(1 << num_bits) -1)
  uint32_t _stride_shift;
  bool _seeded;  // whether the instance is sharing model state with others
  bool _delete;
  void* default_data;
  float* default_value;

 public:
  typedef sparse_iterator<weight> iterator;
  typedef sparse_iterator<const weight> const_iterator;

 private:
  void (*fun)(const weight*, void*);

 public:
  sparse_parameters(size_t length, uint32_t stride_shift = 0)
      : _map()
      , _weight_mask((length << stride_shift) - 1)
      , _stride_shift(stride_shift)
      , _seeded(false)
      , _delete(false)
      , default_data(nullptr)
      , fun(nullptr)
  {
    default_value = calloc_mergable_or_throw<weight>(stride());
  }

  sparse_parameters()
      : _map(), _weight_mask(0), _stride_shift(0), _seeded(false), _delete(false), default_data(nullptr), fun(nullptr)
  {
    default_value = calloc_mergable_or_throw<weight>(stride());
  }

  bool not_null() { return (_weight_mask > 0 && !_map.empty()); }

  sparse_parameters(const sparse_parameters& other) { shallow_copy(other); }
  sparse_parameters(sparse_parameters&&) = delete;

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

  inline weight& operator[](size_t i)
  {
    uint64_t index = i & _weight_mask;
    weight_map::iterator iter = _map.find(index);
    if (iter == _map.end())
    {
      _map.insert(std::make_pair(index, calloc_mergable_or_throw<weight>(stride())));
      iter = _map.find(index);
      if (fun != nullptr)
        fun(iter->second, default_data);
    }
    return *(iter->second);
  }

  inline const weight& operator[](size_t i) const
  {
    uint64_t index = i & _weight_mask;
    weight_map::const_iterator iter = _map.find(index);
    if (iter == _map.end())
      return *default_value;
    return *(iter->second);
  }

  inline weight& strided_index(size_t index) { return operator[](index << _stride_shift); }

  void shallow_copy(const sparse_parameters& input)
  {
    // TODO: this is level-1 copy (weight* are stilled shared)
    if (!_seeded)
    {
      for (auto iter = _map.begin(); iter != _map.end(); ++iter) free(iter->second);
    }
    _map = input._map;
    _weight_mask = input._weight_mask;
    _stride_shift = input._stride_shift;
    free(default_value);
    default_value = calloc_mergable_or_throw<weight>(stride());
    memcpy(default_value, input.default_value, stride());
    default_data = input.default_data;
    _seeded = true;
  }

  template <class R, class T>
  void set_default(R& info)
  {
    R& new_R = calloc_or_throw<R>();
    new_R = info;
    default_data = &new_R;
    fun = (void (*)(const weight*, void*))T::func;
    fun(default_value, default_data);
  }

  template <class T>
  void set_default()
  {
    fun = (void (*)(const weight*, void*))T::func;
  }

  void set_zero(size_t offset)
  {
    for (weight_map::iterator iter = _map.begin(); iter != _map.end(); ++iter) (&(*(iter->second)))[offset] = 0;
  }

  uint64_t mask() const { return _weight_mask; }

  uint64_t seeded() const { return _seeded; }

  uint32_t stride() const { return 1 << _stride_shift; }

  uint32_t stride_shift() const { return _stride_shift; }

  void stride_shift(uint32_t stride_shift)
  {
    _stride_shift = stride_shift;
    free(default_value);
    default_value = calloc_mergable_or_throw<weight>(stride());
    if (fun != nullptr)
      fun(default_value, default_data);
  }

#ifndef _WIN32
  void share(size_t /* length */) { THROW_OR_RETURN("Operation not supported on Windows"); }
#endif

  ~sparse_parameters()
  {
    if (!_delete && !_seeded)  // don't free weight vector if it is shared with another instance
    {
      for (auto iter = _map.begin(); iter != _map.end(); ++iter) free(iter->second);
      _map.clear();
      _delete = true;
    }
    if (default_data != nullptr)
      free(default_data);
    free(default_value);
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

  inline uint32_t stride_shift()
  {
    if (sparse)
      return sparse_weights.stride_shift();
    else
      return dense_weights.stride_shift();
  }

  inline uint32_t stride()
  {
    if (sparse)
      return sparse_weights.stride();
    else
      return dense_weights.stride();
  }

  inline uint64_t mask()
  {
    if (sparse)
      return sparse_weights.mask();
    else
      return dense_weights.mask();
  }

  inline uint64_t seeded()
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
#ifndef DISABLE_SHARED_WEIGHTS
  inline void share(size_t length)
  {
    if (sparse)
      sparse_weights.share(length);
    else
      dense_weights.share(length);
  }
#endif
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
