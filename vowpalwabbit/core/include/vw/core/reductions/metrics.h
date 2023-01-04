#pragma once
// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/vw_fwd.h"

namespace VW
{
namespace reductions
{
VW::LEARNER::base_learner* metrics_setup(VW::setup_base_i& stack_builder);
void output_metrics(VW::workspace& all);
void additional_metrics(VW::workspace& all, VW::metric_sink& sink);
}  // namespace reductions
}  // namespace VW
