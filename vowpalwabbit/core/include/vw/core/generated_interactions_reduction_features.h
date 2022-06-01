// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/core/vw_fwd.h"

#include <vector>

namespace VW
{
namespace generated_interactions
{
struct reduction_features
{
  std::vector<std::vector<VW::namespace_index>>* generated_interactions = nullptr;
  std::vector<std::vector<extent_term>>* generated_extent_interactions = nullptr;

  reduction_features() = default;

  void reset_to_default()
  {
    generated_interactions = nullptr;
    generated_extent_interactions = nullptr;
  }
};

}  // namespace generated_interactions
}  // namespace VW
