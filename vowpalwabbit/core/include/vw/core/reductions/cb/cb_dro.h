#pragma once

#include "vw/core/vw_fwd.h"

#include <memory>

namespace VW
{
namespace reductions
{
std::shared_ptr<VW::LEARNER::learner> cb_dro_setup(VW::setup_base_i& stack_builder);
}
}  // namespace VW