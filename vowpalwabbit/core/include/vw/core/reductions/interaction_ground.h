// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
#include "vw/core/vw_fwd.h"

#include <memory>

namespace VW
{
namespace reductions
{
/**
 * Setup interaction grounded learning reduction. Wiki page:
 * https://github.com/VowpalWabbit/vowpal_wabbit/wiki/Interaction-Grounded-Learning
 *
 * @param stack_builder Stack builder to use for setup.
 * @return VW::LEARNER::learner* learner if this reduction is active, nullptr otherwise
 */
std::shared_ptr<VW::LEARNER::learner> interaction_ground_setup(VW::setup_base_i& stack_builder);
}  // namespace reductions
}  // namespace VW