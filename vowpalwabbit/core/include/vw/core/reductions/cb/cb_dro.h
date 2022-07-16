#pragma once

#include "vw/core/vw_fwd.h"

namespace VW
{
namespace reductions
{
VW::LEARNER::base_learner* cb_dro_setup(VW::setup_base_i& stack_builder);
}
}  // namespace VW