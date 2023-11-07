// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/core/feature_group.h"
#include "vw/core/multi_ex.h"
#include "vw/core/vw_fwd.h"

#include <vector>

namespace VW
{
namespace large_action_space
{
class las_reduction_features
{
public:
  std::vector<std::vector<VW::namespace_index>>* generated_interactions = nullptr;
  std::vector<std::vector<extent_term>>* generated_extent_interactions = nullptr;
  VW::multi_ex::value_type shared_example = nullptr;
  float squarecb_gamma = 1.f;

  las_reduction_features() = default;

  void reset_to_default()
  {
    generated_interactions = nullptr;
    generated_extent_interactions = nullptr;
    shared_example = nullptr;
    squarecb_gamma = 1.f;
  }
};

}  // namespace large_action_space
}  // namespace VW
