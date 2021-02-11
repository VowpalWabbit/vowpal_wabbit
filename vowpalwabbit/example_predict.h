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
#include <set>
#include <unordered_set>
#include <array>

struct namsepace_interactions
{
  std::set<std::vector<namespace_index>> active_interactions;  // TODO maybe remove
  std::set<namespace_index> all_example_namespaces;            // TODO maybe ordered vector
  std::vector<std::vector<namespace_index>> interactions;
  bool quadraditcs_wildcard_expansion = false;
  bool leave_duplicate_interactions = false;
  std::unordered_set<namespace_index> extra_interactions;  // e.g. ccb_id_namespace from conditional_contextual_bandits
};

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

  // Pushing to indices should be done via set_namespace and never by pushing directly to the indices array
  v_array<namespace_index> indices;
  std::array<features, NUM_NAMESPACES> feature_space;  // Groups of feature values.
  uint64_t ft_offset;                                  // An offset for all feature values.

  void set_namespace(const namespace_index& index, bool interact = true);

  // Interactions are specified by this struct's interactions vector of vectors of unsigned characters, where each
  // vector is an interaction and each char is a namespace.
  namsepace_interactions* interactions;
  reduction_features _reduction_features;

  uint32_t _current_reduction_depth;  // Used for debugging reductions.  Keeps track of current reduction level
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
