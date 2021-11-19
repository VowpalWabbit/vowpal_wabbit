// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once
#include "reductions_fwd.h"

namespace CSOAA
{
VW::LEARNER::base_learner* csoaa_setup(VW::setup_base_i& stack_builder);

VW::LEARNER::base_learner* csldf_setup(VW::setup_base_i& stack_builder);
struct csoaa;
void finish_example(VW::workspace& all, csoaa&, example& ec);
}  // namespace CSOAA
