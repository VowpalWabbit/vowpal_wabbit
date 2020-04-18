#pragma once

#include "reductions.h"

namespace VW
{
namespace continuous_cb
{
VW::LEARNER::base_learner* setup(VW::config::options_i& options, vw& all);

} // namespace continuous_cb
}
