#pragma once
// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "reductions_fwd.h"

namespace VW
{
namespace metrics
{
VW::LEARNER::base_learner* metrics_setup(VW::setup_base_i& stack_builder);
void output_metrics(vw& all);
}  // namespace metrics
}  // namespace VW
