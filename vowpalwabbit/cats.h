#pragma once
#include "learner.h"
#include "options.h"

namespace VW { namespace continuous_action { namespace cats {

  LEARNER::base_learner* setup(config::options_i& options, vw& all);

}}}  // namespace VW
