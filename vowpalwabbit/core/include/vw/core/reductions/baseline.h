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
std::shared_ptr<VW::LEARNER::learner> baseline_setup(VW::setup_base_i& stack_builder);

namespace baseline
{
// utility functions for disabling baseline on a given example
void set_baseline_enabled(VW::example* ec);
void reset_baseline_disabled(VW::example* ec);
bool baseline_enabled(const VW::example* ec);
}  // namespace baseline
}  // namespace reductions
}  // namespace VW