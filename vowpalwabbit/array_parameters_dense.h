// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <cstdint>
#ifndef _WIN32
#  include <sys/mman.h>
#endif

#include "memory.h"
#include <bitset>
#include <unordered_map>

using weight = float;

template <typename T>
class dense_iterator
{
private:
  T* _current;
  T* _begin;
  uint32_t _stride;

public:
  using iterator_category = std::forward_iterator_tag;
  using value_type = T;
  using difference_type = std::ptrdiff_t;
  using pointer = T*;
  using reference = T&;

  dense_iterator(T* current, T* begin, uint32_t stride) : _current(current), _begin(begin), _stride(stride) {}

  T& operator*() { return *_current; }

  size_t index() { return _current - _begin; }

  dense_iterator& operator++()
  {
    _current += _stride;
    return *this;
  }

  bool operator==(const dense_iterator& rhs) const { return _current == rhs._current; }
  bool operator!=(const dense_iterator& rhs) const { return _current != rhs._current; }
};

class dense_parameters
{
private:
  weight* _begin;
  uint64_t _weight_mask;  // (stride*(1 << num_bits) -1)
  uint32_t _stride_shift;
  bool _seeded;  // whether the instance is sharing model state with others
#ifdef PRIVACY_ACTIVATION
  // struct to store the tag hash and if it is set or not
  struct tag_hash_info
  {
    uint64_t tag_hash;
    bool is_set = false;
  };
  size_t _privacy_activation_threshold;
  std::shared_ptr<std::unordered_map<uint64_t, std::bitset<32>>> _feature_bitset;  // define the bitset for each feature
  tag_hash_info _tag_info;
#endif

public:
  using iterator = dense_iterator<weight>;
  using const_iterator = dense_iterator<const weight>;
  dense_parameters(size_t length, uint32_t stride_shift = 0)
      : _begin(calloc_mergable_or_throw<weight>(length << stride_shift))
      , _weight_mask((length << stride_shift) - 1)
      , _stride_shift(stride_shift)
      , _seeded(false)
#ifdef PRIVACY_ACTIVATION
      , _privacy_activation_threshold(0)
      , _feature_bitset(nullptr)
#endif
  {
  }

  dense_parameters()
      : _begin(nullptr)
      , _weight_mask(0)
      , _stride_shift(0)
      , _seeded(false)
#ifdef PRIVACY_ACTIVATION
      , _privacy_activation_threshold(0)
      , _feature_bitset(nullptr)
#endif
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
  iterator begin() { return iterator(_begin, _begin, stride()); }
  iterator end() { return iterator(_begin + _weight_mask + 1, _begin, stride()); }

  // const iterator
  const_iterator cbegin() { return const_iterator(_begin, _begin, stride()); }
  const_iterator cend() { return const_iterator(_begin + _weight_mask + 1, _begin, stride()); }

  inline const weight& operator[](size_t i) const { return _begin[i & _weight_mask]; }
  inline weight& operator[](size_t i)
  {
#ifdef PRIVACY_ACTIVATION
    if (_feature_bitset && _tag_info.is_set)
    {
      // lookup a bit for a feature in the bitset using the
      // tag hash and turn it on
      (*_feature_bitset)[i & _weight_mask][_tag_info.tag_hash] = 1;
    }
#endif
    return _begin[i & _weight_mask];
  }

#ifdef PRIVACY_ACTIVATION
  void set_tag(uint64_t tag_hash)
  {
    if (_feature_bitset)
    {
      _tag_info.tag_hash = tag_hash;
      _tag_info.is_set = true;
    }
  }

  void unset_tag() { _tag_info.is_set = false; }

  // function to check if the number of bits set to 1 are greater than a threshold for a feature
  bool is_activated(uint64_t index)
  {
    if (!_feature_bitset) { return false; }
    return (*_feature_bitset)[index].count() >= _privacy_activation_threshold;
  }
#endif

  void shallow_copy(const dense_parameters& input)
  {
    if (!_seeded) free(_begin);
    _begin = input._begin;
    _weight_mask = input._weight_mask;
    _stride_shift = input._stride_shift;
    _seeded = true;
#ifdef PRIVACY_ACTIVATION
    _privacy_activation_threshold = input._privacy_activation_threshold;
    _feature_bitset = input._feature_bitset;
#endif
  }

  inline weight& strided_index(size_t index) { return operator[](index << _stride_shift); }

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
    for (iterator iter = begin(); iter != end(); ++iter) (&(*iter))[offset] = 0;
  }

  void copy_offsets(const size_t from, const size_t to, const size_t params_per_problem)
  {
    assert(from < params_per_problem);
    assert(to < params_per_problem);
    uint32_t stride_size = 1 << stride_shift();

    int64_t diff = to - from;
    for (auto iter = begin(); iter != end(); ++iter)
    {
      size_t prestride_index = iter.index() >> stride_shift();
      size_t current_offset = prestride_index & (params_per_problem - 1);
      if (current_offset == from)
      {
        float* other = &_begin[(prestride_index + diff) << stride_shift()];

        if (*other != 0.f || *iter != 0.f)
        {
          for (size_t stride_offset = 0; stride_offset < stride_size; stride_offset++)
          { (&(*other))[stride_offset] = (&(*iter))[stride_offset]; }
        }
      }
    }
  }

  void clear_offset(const size_t offset, const size_t params_per_problem)
  {
    assert(offset < params_per_problem);
    uint32_t stride_size = 1 << stride_shift();

    for (iterator iter = begin(); iter != end(); ++iter)
    {
      if (*iter != 0.f)
      {
        size_t current_offset = (iter.index() >> stride_shift()) & (params_per_problem - 1);
        if (current_offset == offset)
        {
          for (size_t stride_offset = 0; stride_offset < stride_size; stride_offset++)
          { (&(*iter))[stride_offset] = 0.f; }
        }
      }
    }
  }

  uint64_t mask() const { return _weight_mask; }

  uint64_t seeded() const { return _seeded; }

  uint32_t stride() const { return 1 << _stride_shift; }

  uint32_t stride_shift() const { return _stride_shift; }

  void stride_shift(uint32_t stride_shift) { _stride_shift = stride_shift; }

#ifdef PRIVACY_ACTIVATION
  void privacy_activation_threshold(size_t privacy_activation_threshold)
  {
    _privacy_activation_threshold = privacy_activation_threshold;
    _feature_bitset = std::make_shared<std::unordered_map<uint64_t, std::bitset<32>>>();
  }
#endif

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
