// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/core/learner_fwd.h"
#include "vw/core/vw_fwd.h"

#include <cstddef>
#include <memory>

namespace VW
{
namespace reductions
{
class cb_actions_mask
{
  // this reduction is used to get the actions mask from VW::actions_mask::reduction_features and apply it to the
  // outcoming predictions
public:
  void update_predictions(multi_ex& examples, size_t initial_action_size);

private:
  template <bool is_learn>
  void learn_or_predict(VW::LEARNER::learner& base, multi_ex& examples);
};

std::shared_ptr<VW::LEARNER::learner> cb_actions_mask_setup(VW::setup_base_i&);
}  // namespace reductions
}  // namespace VW
