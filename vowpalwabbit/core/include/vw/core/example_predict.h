// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "vw/common/future_compat.h"
#include "vw/core/constant.h"
#include "vw/core/feature_group.h"
#include "vw/core/reduction_features.h"
#include "vw/core/v_array.h"

#include <array>
#include <set>
#include <unordered_set>
#include <vector>

namespace VW
{
using namespace_index = unsigned char;
struct example_predict
{
  template <typename FeaturesIt, typename FeaturesRef, typename IndexIt>
  class iterator_impl
  {
    FeaturesIt _feature_space;
    IndexIt _index;

  public:
    iterator_impl(FeaturesIt feature_space, IndexIt index) : _feature_space(feature_space), _index(index) {}

    FeaturesRef operator*() { return _feature_space[*_index]; }

    iterator_impl& operator++()
    {
      _index++;
      return *this;
    }
    namespace_index index() { return *_index; }
    bool operator==(const iterator_impl& rhs) const { return _index == rhs._index; }
    bool operator!=(const iterator_impl& rhs) const { return !(*this == rhs); }
  };

  using iterator = iterator_impl<features*, features&, VW::v_array<namespace_index>::iterator>;
  using const_iterator = iterator_impl<const features*, const features&, VW::v_array<namespace_index>::const_iterator>;

  example_predict() = default;
  ~example_predict() = default;
  example_predict(const example_predict&) = delete;
  example_predict& operator=(const example_predict&) = delete;
  example_predict(example_predict&& other) = default;
  example_predict& operator=(example_predict&& other) = default;

  /// If indices is modified this iterator is invalidated.
  iterator begin();
  const_iterator begin() const;
  /// If indices is modified this iterator is invalidated.
  iterator end();
  const_iterator end() const;

  VW::v_array<namespace_index> indices;
  std::array<features, NUM_NAMESPACES> feature_space;  // Groups of feature values.
  uint64_t ft_offset = 0;                              // An offset for all feature values.

  // Interactions are specified by this struct's interactions vector of vectors of unsigned characters, where each
  // vector is an interaction and each char is a namespace.
  std::vector<std::vector<namespace_index>>* interactions = nullptr;

  // Optional
  std::vector<std::vector<extent_term>>* extent_interactions = nullptr;
  reduction_features _reduction_features;

  // Used for debugging reductions.  Keeps track of current reduction level.
  uint32_t _debug_current_reduction_depth = 0;
};
}  // namespace VW

using namespace_index VW_DEPRECATED("namespace_index moved into VW namespace") = VW::namespace_index;
using example_predict VW_DEPRECATED("example_predict moved into VW namespace") = VW::example_predict;
