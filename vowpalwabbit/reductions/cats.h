// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
#include "api_status.h"
#include "config/options.h"
#include "learner.h"

namespace VW
{
namespace reductions
{
LEARNER::base_learner* cats_setup(setup_base_i& stack_builder);

namespace cats
{
struct cats
{
  uint32_t num_actions = 0;
  float bandwidth = 0.f;
  float min_value = 0.f;
  float max_value = 0.f;

  cats(LEARNER::single_learner* p_base);

  int learn(example& ec, experimental::api_status* status);
  int predict(example& ec, experimental::api_status* status);
  float get_loss(const VW::cb_continuous::continuous_label& cb_cont_costs, float predicted_action) const;

private:
  LEARNER::single_learner* _base = nullptr;
};
}  // namespace cats
}  // namespace reductions
}  // namespace VW
