#pragma once
#include "learner.h"

namespace VW
{
namespace continuous_action
{
  // Setup reduction in stack
  LEARNER::base_learner* get_pmf_setup(config::options_i& options, vw& all);
}  // namespace continuous_action
}  // namespace VW
