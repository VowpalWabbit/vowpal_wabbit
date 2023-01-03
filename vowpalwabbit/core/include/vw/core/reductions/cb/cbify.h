// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/core/feature_group.h"
#include "vw/core/multi_ex.h"
#include "vw/core/vw_fwd.h"

namespace VW
{
namespace reductions
{
class cbify_adf_data
{
public:
  VW::multi_ex ecs;
  size_t num_actions = 0;
  size_t increment = 0;
  uint64_t custom_index_mask = 0;

  void init_adf_data(std::size_t num_actions, size_t increment,
      std::vector<std::vector<VW::namespace_index>>& interactions,
      std::vector<std::vector<extent_term>>& extent_interactions);
  void copy_example_to_adf(parameters& weights, VW::example& ec);

  ~cbify_adf_data();
};

VW::LEARNER::base_learner* cbify_setup(VW::setup_base_i& stack_builder);
VW::LEARNER::base_learner* cbifyldf_setup(VW::setup_base_i& stack_builder);
}  // namespace reductions
}  // namespace VW