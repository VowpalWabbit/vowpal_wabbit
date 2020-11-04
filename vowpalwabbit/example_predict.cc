// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "example_predict.h"

#include <sstream>

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

std::string features_to_string(const example_predict& ec)
{
  std::stringstream strstream;
  strstream << "[off=" << ec.ft_offset << "]";
  for (auto& f : ec.feature_space)
  {
    auto ind_iter = f.indicies.cbegin();
    auto val_iter = f.values.cbegin();
    for (; ind_iter != f.indicies.cend(); ++ind_iter, ++val_iter)
    {
      strstream << "[h=" << *ind_iter << ","
                << "v=" << *val_iter << "]";
    }
  }
  return strstream.str();
}

std::string depth_indent_string(const int32_t depth)
{
  constexpr const char* indent_str = "- ";
  constexpr const char* space_str = "  ";

  if (depth == 0) return indent_str;

  std::stringstream str_stream;
  for (int32_t i = 0; i < depth - 1; i++) { str_stream << space_str; }
  str_stream << indent_str;
  return str_stream.str();
}

std::string depth_indent_string(const example_predict& ec) { return depth_indent_string(ec._current_reduction_depth); }
