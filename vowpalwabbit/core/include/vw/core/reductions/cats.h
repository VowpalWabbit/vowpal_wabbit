// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/core/api_status.h"
#include "vw/core/cb_continuous_label.h"
#include "vw/core/learner_fwd.h"
#include "vw/core/vw_fwd.h"

#include <memory>

namespace VW
{
namespace reductions
{
std::shared_ptr<VW::LEARNER::learner> cats_setup(setup_base_i& stack_builder);

namespace cats
{
class cats
{
public:
  uint32_t num_actions = 0;
  float bandwidth = 0.f;
  float min_value = 0.f;
  float max_value = 0.f;

  cats(LEARNER::learner* p_base);

  int learn(example& ec, experimental::api_status* status);
  int predict(example& ec, experimental::api_status* status);
  float get_loss(const VW::cb_continuous::continuous_label& cb_cont_costs, float predicted_action) const;

private:
  LEARNER::learner* _base = nullptr;
};
}  // namespace cats
}  // namespace reductions
}  // namespace VW
