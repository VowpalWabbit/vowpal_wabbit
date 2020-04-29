#pragma once
#include "learner.h"
#include "options.h"

namespace VW { namespace continuous_action {
  // CATS - Continuous actions tree with smoothing
  LEARNER::base_learner* cats_setup(config::options_i& options, vw& all);
}}
