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

example_predict::iterator example_predict::begin() { return {feature_space.data(), indices.begin()}; }
example_predict::iterator example_predict::end() { return {feature_space.data(), indices.end()}; }

VW_WARNING_STATE_PUSH
VW_WARNING_DISABLE_DEPRECATED_USAGE
safe_example_predict::safe_example_predict()
{
  ft_offset = 0;
}

VW_WARNING_STATE_POP
void safe_example_predict::clear()
{
  for (auto ns : indices) feature_space[ns].clear();
  indices.clear();
}

void namespace_interactions::clear()
{
  active_interactions.clear();
  all_seen_namespaces.clear();
  interactions.clear();
  quadratics_wildcard_expansion = false;
  leave_duplicate_interactions = false;
  all_seen_namespaces_size = 0;
}

void namespace_interactions::append(const namespace_interactions& src)
{
  active_interactions.insert(src.active_interactions.begin(), src.active_interactions.end());
  all_seen_namespaces.insert(src.all_seen_namespaces.begin(), src.all_seen_namespaces.end());
  std::copy(src.interactions.begin(), src.interactions.end(), std::back_inserter(interactions));
  quadratics_wildcard_expansion = src.quadratics_wildcard_expansion;
  leave_duplicate_interactions = src.leave_duplicate_interactions;
  all_seen_namespaces_size = src.all_seen_namespaces_size;
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

std::string debug_depth_indent_string(const int32_t depth)
{
  constexpr const char* indent_str = "- ";
  constexpr const char* space_str = "  ";

  if (depth == 0) return indent_str;

  std::stringstream str_stream;
  for (int32_t i = 0; i < depth - 1; i++) { str_stream << space_str; }
  str_stream << indent_str;
  return str_stream.str();
}