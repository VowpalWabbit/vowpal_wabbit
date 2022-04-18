// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
#include "vw/core/cb.h"
#include "vw/core/vw_fwd.h"

#include <vector>

namespace VW
{
namespace reductions
{
VW::LEARNER::base_learner* cb_adf_setup(VW::setup_base_i& stack_builder);
}
}  // namespace VW

// TODO: Move these functions into VW lib and not reductions
namespace CB_ADF
{
VW::example* test_adf_sequence(const VW::multi_ex& ec_seq);
CB::cb_class get_observed_cost_or_default_cb_adf(const VW::multi_ex& examples);
}  // namespace CB_ADF
