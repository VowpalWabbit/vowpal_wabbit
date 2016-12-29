/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
*/
#pragma once

LEARNER::base_learner* cb_explore_adf_setup(vw& all);

namespace CB_EXPLORE_ADF
{
example* test_adf_sequence(v_array<example*>& ec_seq);
}
