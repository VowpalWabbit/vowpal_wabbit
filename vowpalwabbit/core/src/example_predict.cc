// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/example_predict.h"

#include <sstream>

VW::example_predict::iterator::iterator(features* feature_space, namespace_index* index)
    : _feature_space(feature_space), _index(index)
{
}

VW::features& VW::example_predict::iterator::operator*() { return _feature_space[*_index]; }

VW::example_predict::iterator& VW::example_predict::iterator::operator++()
{
  _index++;
  return *this;
}

VW::namespace_index VW::example_predict::iterator::index() { return *_index; }

bool VW::example_predict::iterator::operator==(const iterator& rhs) const { return _index == rhs._index; }
bool VW::example_predict::iterator::operator!=(const iterator& rhs) const { return _index != rhs._index; }

VW::example_predict::iterator VW::example_predict::begin() { return {feature_space.data(), indices.begin()}; }
VW::example_predict::iterator VW::example_predict::end() { return {feature_space.data(), indices.end()}; }

uint64_t VW::example_predict::get_or_calculate_order_independent_feature_space_hash()
{
  if (!is_set_feature_space_hash)
  {
    is_set_feature_space_hash = true;
    for (const auto ns : indices)
    {
      feature_space_hash += std::hash<namespace_index>()(ns);
      for (const auto& f : feature_space[ns])
      {
        feature_space_hash += std::hash<feature_index>()(f.index());
        feature_space_hash += std::hash<feature_value>()(f.value());
      }
    }
  }

  return feature_space_hash;
}
