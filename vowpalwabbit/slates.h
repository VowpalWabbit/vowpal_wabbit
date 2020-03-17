// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "reductions_fwd.h"

#include <string>

namespace slates
{
LEARNER::base_learner* slates_setup(VW::config::options_i& options, vw& all);
std::string generate_slates_label_printout(const std::vector<example*>& slots);
}  // namespace slates
