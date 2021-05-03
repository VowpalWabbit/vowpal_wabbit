// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "learner.h"
#include "global_data.h"
#include "options.h"

VW::LEARNER::base_learner* count_interactions_setup(VW::config::options_i& options, vw& all);
