// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

typedef unsigned char namespace_index;

#include "constant.h"
#include "future_compat.h"
#include "reduction_features.h"
#include "feature_group.h"
#include "v_array.h"

#include <vector>
#include <array>
#include <sstream>

struct example_predict
{
  class iterator
  {
    features* _feature_space;
    v_array<namespace_index>::iterator _index;

  public:
    iterator(features* feature_space, namespace_index* index);
    features& operator*();
    iterator& operator++();
    namespace_index index();
    bool operator==(const iterator& rhs);
    bool operator!=(const iterator& rhs);
  };

  example_predict();
  ~example_predict();
  example_predict(const example_predict&) = delete;
  example_predict& operator=(const example_predict&) = delete;
  example_predict(example_predict&& other) noexcept;
  example_predict& operator=(example_predict&& other) noexcept;

  /// If indices is modified this iterator is invalidated.
  iterator begin();
  /// If indices is modified this iterator is invalidated.
  iterator end();

  v_array<namespace_index> indices;
  std::array<features, NUM_NAMESPACES> feature_space;  // Groups of feature values.
  uint64_t ft_offset;                                  // SAn offset for all feature values.
  uint32_t stack_depth;

  // Interactions are specified by this vector of vectors of unsigned characters, where each vector is an interaction
  // and each char is a namespace.
  std::vector<std::vector<namespace_index>>* interactions;
  reduction_features _reduction_features;

  // Used for debugging reductions.  Keeps track of current reduction level.
  uint32_t _debug_current_reduction_depth;
};

// make sure we have an exception safe version of example_predict

class VW_DEPRECATED("example_predict is now RAII based. That class can be used instead.") safe_example_predict
    : public example_predict
{
public:
  safe_example_predict();
  ~safe_example_predict();

  void clear();
};

std::string features_to_string(const example_predict& ec);
std::string depth_indent_string(const example_predict& ec);
std::string depth_indent_string(int32_t stack_depth);
