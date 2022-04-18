// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "vw/core/reductions/cb/cb_explore_adf_common.h"
#include "vw/core/vw_fwd.h"

namespace VW
{
namespace reductions
{
VW::LEARNER::base_learner* cb_explore_adf_regcb_setup(VW::setup_base_i& stack_builder);
}  // namespace regcb
}  // namespace cb_explore_adf
