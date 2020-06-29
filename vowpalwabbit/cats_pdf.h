#pragma once
#include "learner.h"

namespace VW { namespace continuous_action { namespace cats_pdf {

  // Setup reduction in stack
  LEARNER::base_learner* setup(config::options_i& options, vw& all);

}}}  // namespace VW
