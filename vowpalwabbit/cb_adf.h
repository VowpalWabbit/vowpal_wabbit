// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
#include <vector>
#include "reductions_fwd.h"
#include "io/logger.h"

namespace CB
{
struct cb_class;
}

namespace VW
{
namespace io
{
struct writer;
}
}  // namespace VW

VW::LEARNER::base_learner* cb_adf_setup(VW::setup_base_i& stack_builder);

namespace CB_ADF
{
example* test_adf_sequence(const multi_ex& ec_seq);
CB::cb_class get_observed_cost_or_default_cb_adf(const multi_ex& examples);
}  // namespace CB_ADF
