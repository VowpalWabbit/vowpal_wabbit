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

  bool is_epsilon_supplied()
  {
    if (epsilon >= 0.f && epsilon <= 1.f) { return true; }
    else if (epsilon == -1.f) { return false; }
    THROW("Invalid epsilon value: " << epsilon << " supplied. Epsilon must be between 0 and 1 inclusive.");
  }
  void reset_to_default() { epsilon = -1.f; }
};
}  // namespace greedy
}  // namespace cb_explore_adf
}  // namespace VW
