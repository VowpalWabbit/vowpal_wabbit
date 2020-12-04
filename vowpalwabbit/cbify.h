// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
#include "reductions_fwd.h"
#include "example.h"

struct cbify_adf_data
{
  multi_ex ecs;
  size_t num_actions;
};

void copy_example_to_adf(cbify_adf_data& adf_data, parameters& weights, example& ec);
void init_adf_data(
    cbify_adf_data& adf_data, const size_t num_actions, std::vector<std::vector<namespace_index>>& interactions);

VW::LEARNER::base_learner* cbify_setup(VW::config::options_i& options, vw& all);
VW::LEARNER::base_learner* cbifyldf_setup(VW::config::options_i& options, vw& all);
