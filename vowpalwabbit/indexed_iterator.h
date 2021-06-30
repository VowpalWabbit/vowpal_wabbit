// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <utility>
#include <vector>
#include <unordered_map>
#include <cassert>
#include <set>
#include <map>

#include "feature_group.h"
#include "generic_range.h"

typedef unsigned char namespace_index;

namespace VW
{

/// Insertion or removal will result in this value in invalidated.
class indexed_iterator_t
{
  size_t* _indices;
  features* _feature_groups;
  namespace_index* _namespace_indices;
  uint64_t* _namespace_hashes;

public:
  using difference_type = std::ptrdiff_t;

  indexed_iterator_t(size_t* indices, features* feature_groups, namespace_index* namespace_indices, uint64_t* namespace_hashes)
      : _indices(indices)
      , _feature_groups(feature_groups)
      , _namespace_indices(namespace_indices)
      , _namespace_hashes(namespace_hashes)
  {
  }
  features& operator*()
  {
    assert(_indices != nullptr);
    return _feature_groups[*_indices];
  }
  indexed_iterator_t& operator++()
  {
    if (_indices != nullptr) { ++_indices; }
    return *this;
  }

  indexed_iterator_t& operator--()
  {
    if (_indices != nullptr) { --_indices; }
    return *this;
  }

  namespace_index index()
  {
    assert(_indices != nullptr);
    return _namespace_indices[*_indices];
  }
  uint64_t hash()
  {
    assert(_indices != nullptr);
    return _namespace_hashes[*_indices];
  }

  friend bool operator<(const indexed_iterator_t& lhs, const indexed_iterator_t& rhs)
  {
    return lhs._indices < rhs._indices;
  }

  friend bool operator>(const indexed_iterator_t& lhs, const indexed_iterator_t& rhs)
  {
    return lhs._indices > rhs._indices;
  }

  friend bool operator<=(const indexed_iterator_t& lhs, const indexed_iterator_t& rhs) { return !(lhs > rhs); }
  friend bool operator>=(const indexed_iterator_t& lhs, const indexed_iterator_t& rhs) { return !(lhs < rhs); }

  friend difference_type operator-(const indexed_iterator_t& lhs, const indexed_iterator_t& rhs)
  {
    assert(lhs._indices >= rhs._indices);
    return lhs._indices - rhs._indices;
  }

  bool operator==(const indexed_iterator_t& rhs) const { return _indices == rhs._indices; }
  bool operator!=(const indexed_iterator_t& rhs) const { return _indices != rhs._indices; }
};
}