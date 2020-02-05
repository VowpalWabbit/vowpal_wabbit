// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
#include "reductions_fwd.h"

LEARNER::base_learner* cbify_setup(VW::config::options_i& options, vw& all);
LEARNER::base_learner* cbifyldf_setup(VW::config::options_i& options, vw& all);
