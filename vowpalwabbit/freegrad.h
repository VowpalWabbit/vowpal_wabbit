#pragma once
#include "global_data.h"
#include "reductions_fwd.h"

namespace VW
{
VW::LEARNER::base_learner* freegrad_setup(VW::setup_base_i& stack_builder);
}