// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

namespace VW
{
namespace cb_explore_adf
{
namespace greedy
{
struct reduction_features
{
  float epsilon;

  reduction_features() { epsilon = -1.f; }

  void clear() { epsilon = -1.f; }
};
}  // namespace greedy
}  // namespace cb_explore_adf
}  // namespace VW
