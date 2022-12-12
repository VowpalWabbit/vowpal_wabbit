// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
#include "vw/core/vw_fwd.h"

namespace VW
{
namespace reductions
{
/**
 * Setup interaction grounded learning reduction. Wiki page:
 * https://github.com/VowpalWabbit/vowpal_wabbit/wiki/Interaction-Grounded-Learning
 *
 * @param stack_builder Stack builder to use for setup.
 * @return VW::LEARNER::base_learner* learner if this reduction is active, nullptr otherwise
 */
VW::LEARNER::base_learner* interaction_ground_setup(VW::setup_base_i& stack_builder);
}  // namespace reductions
}  // namespace VW