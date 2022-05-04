// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/example_predict.h"

VW::example_predict::iterator VW::example_predict::begin() { return {feature_space.data(), indices.begin()}; }
VW::example_predict::const_iterator VW::example_predict::begin() const
{
  return {feature_space.data(), indices.begin()};
}
VW::example_predict::iterator VW::example_predict::end() { return {feature_space.data(), indices.end()}; }
VW::example_predict::const_iterator VW::example_predict::end() const { return {feature_space.data(), indices.end()}; }
