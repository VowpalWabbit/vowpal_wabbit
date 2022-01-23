// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "reductions_fwd.h"
#include "distributionally_robust.h"

namespace VW
{
namespace ae
{
VW::LEARNER::base_learner* ae_setup(VW::setup_base_i&);
}
}  // namespace VW
