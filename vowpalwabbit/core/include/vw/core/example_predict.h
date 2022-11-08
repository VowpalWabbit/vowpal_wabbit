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
class example_predict
{
public:
  class iterator
  {
  public:
    iterator(features* feature_space, namespace_index* index);
    features& operator*();
    iterator& operator++();
    namespace_index index();
    bool operator==(const iterator& rhs) const;
    bool operator!=(const iterator& rhs) const;

  private:
    features* _feature_space;
    VW::v_array<namespace_index>::iterator _index;
  };

  example_predict() = default;
  ~example_predict() = default;
  example_predict(const example_predict&) = delete;
  example_predict& operator=(const example_predict&) = delete;
  example_predict(example_predict&& other) = default;
  example_predict& operator=(example_predict&& other) = default;

  /// If indices is modified this iterator is invalidated.
  iterator begin();
  /// If indices is modified this iterator is invalidated.
  iterator end();

  VW::v_array<namespace_index> indices;
  std::array<features, NUM_NAMESPACES> feature_space;  // Groups of feature values.
  uint64_t ft_offset = 0;                              // An offset for all feature values.

  // Interactions are specified by this struct's interactions vector of vectors of unsigned characters, where each
  // vector is an interaction and each char is a namespace.
  std::vector<std::vector<namespace_index>>* interactions = nullptr;

  // Optional
  std::vector<std::vector<extent_term>>* extent_interactions = nullptr;
  reduction_features ex_reduction_features;

  // Used for debugging reductions.  Keeps track of current reduction level.
  uint32_t debug_current_reduction_depth = 0;
};
}  // namespace VW

using namespace_index VW_DEPRECATED("namespace_index moved into VW namespace") = VW::namespace_index;
using example_predict VW_DEPRECATED("example_predict moved into VW namespace") = VW::example_predict;
