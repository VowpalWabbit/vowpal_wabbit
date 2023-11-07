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
 * @brief Wiki page: https://github.com/VowpalWabbit/vowpal_wabbit/wiki/FreeGrad
 */
std::shared_ptr<VW::LEARNER::learner> freegrad_setup(VW::setup_base_i& stack_builder);
}  // namespace reductions
}  // namespace VW