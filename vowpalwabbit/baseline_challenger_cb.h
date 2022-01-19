// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "reductions_fwd.h"

constexpr double DEFAULT_TAU = 0.999;
constexpr double DEFAULT_ALPHA = 0.05;

VW::LEARNER::base_learner* baseline_challenger_cb_setup(VW::setup_base_i&);