// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
#include "reductions_fwd.h"
#include "example.h"

template <typename T>
void v_move(v_array<T>& dst, v_array<T>& src)
{
  dst = src;
  src = v_init<T>();
}

struct cbify_adf_data
{
  multi_ex ecs;
  size_t num_actions;

  void init_adf_data(const size_t num_actions, std::vector<std::vector<namespace_index>>& interactions);
  void copy_example_to_adf(parameters& weights, example& ec);

  ~cbify_adf_data();
};

VW::LEARNER::base_learner* cbify_setup(VW::config::options_i& options, vw& all);
VW::LEARNER::base_learner* cbifyldf_setup(VW::config::options_i& options, vw& all);
