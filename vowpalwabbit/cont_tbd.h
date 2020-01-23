#pragma once
#include "learner.h"
#include "options.h"

namespace VW { namespace continuous_action {
  LEARNER::base_learner* cont_tbd_setup(config::options_i& options, vw& all);
}}
