/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
*/
#pragma once

namespace LEARNER
{ template<class T> struct learner;
typedef learner<char> base_learner;
}

LEARNER::base_learner* cb_explore_setup(vw& all);

namespace CB_EXPLORE
{
void safety(v_array<ACTION_SCORE::action_score>& distribution, float min_prob, bool zeros);
}
