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
class reduction_features
{
public:
  float epsilon;

  reduction_features() { epsilon = -1.f; }

  bool valid_epsilon_supplied() { return epsilon >= 0.f && epsilon <= 1.f; }
  void reset_to_default() { epsilon = -1.f; }
};
}  // namespace greedy
}  // namespace cb_explore_adf
}  // namespace VW
