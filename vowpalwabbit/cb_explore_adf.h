/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
*/
#pragma once

#include <vector>
struct example;

LEARNER::base_learner* cb_explore_adf_setup(VW::config::options_i& options, vw& all);

namespace CB_EXPLORE_ADF
{
example* test_adf_sequence(std::vector<example*>& ec_seq);
}
