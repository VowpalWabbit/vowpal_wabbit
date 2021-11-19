#pragma once
#include "global_data.h"
#include "reductions_fwd.h"

namespace VW
{
/**
 * @brief Wiki page: https://github.com/VowpalWabbit/vowpal_wabbit/wiki/FreeGrad
 */
VW::LEARNER::base_learner* freegrad_setup(VW::setup_base_i& stack_builder);
}  // namespace VW