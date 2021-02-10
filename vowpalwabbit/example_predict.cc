// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "example_predict.h"

example_predict::iterator::iterator(features* feature_space, namespace_index* index)
    : _feature_space(feature_space), _index(index)
{
}

features& example_predict::iterator::operator*() { return _feature_space[*_index]; }

example_predict::iterator& example_predict::iterator::operator++()
{
  _index++;
  return *this;
}

namespace_index example_predict::iterator::index() { return *_index; }

bool example_predict::iterator::operator==(const iterator& rhs) { return _index == rhs._index; }
bool example_predict::iterator::operator!=(const iterator& rhs) { return _index != rhs._index; }

example_predict::example_predict()
{
  indices = v_init<namespace_index>();
  ft_offset = 0;
  interactions = nullptr;
}

example_predict::~example_predict() { indices.delete_v(); }

example_predict::example_predict(example_predict&& other) noexcept
    : indices(std::move(other.indices))
    , feature_space(std::move(other.feature_space))
    , ft_offset(other.ft_offset)
    , interactions(other.interactions)
{
  // We need to null out all the v_arrays to prevent double freeing during moves
  auto& v = other.indices;
  v._begin = nullptr;
  v._end = nullptr;
  v.end_array = nullptr;
  other.ft_offset = 0;
  other.interactions = nullptr;
}

void example_predict::set_feature_space_and_active_namespace(
    const namespace_index& index, feature_value fv, feature_index fi)
{
  feature_space[index].push_back(fv, fi);
  // keep active namespaces if we are doing wildcard expansion for interactions
  // skip if constant feature
  if (constant != fi && interactions != nullptr && interactions->wild_card_expansion)
  {
    // active_namespaces_of_example.emplace(index);
    interactions->all_example_namespaces.emplace(index);
  }
}

example_predict& example_predict::operator=(example_predict&& other) noexcept
{
  indices = std::move(other.indices);
  feature_space = std::move(other.feature_space);
  interactions = other.interactions;
  // We need to null out all the v_arrays to prevent double freeing during moves

  auto& v = other.indices;
  v._begin = nullptr;
  v._end = nullptr;
  v.end_array = nullptr;
  other.ft_offset = 0;
  other.interactions = nullptr;
  return *this;
}

example_predict::iterator example_predict::begin() { return {feature_space.data(), indices.begin()}; }
example_predict::iterator example_predict::end() { return {feature_space.data(), indices.end()}; }

VW_WARNING_STATE_PUSH
VW_WARNING_DISABLE_DEPRECATED_USAGE
safe_example_predict::safe_example_predict()
{
  indices = v_init<namespace_index>();
  ft_offset = 0;
  // feature_space is initialized through constructors
}

safe_example_predict::~safe_example_predict() { indices.delete_v(); }
VW_WARNING_STATE_POP
void safe_example_predict::clear()
{
  for (auto ns : indices) feature_space[ns].clear();
  indices.clear();
}
