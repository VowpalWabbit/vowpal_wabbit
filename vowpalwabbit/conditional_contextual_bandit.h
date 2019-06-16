#pragma once

#include <cstdint>
#include <vector>

#include "v_array.h"
#include "action_score.h"
#include "options.h"


namespace LEARNER {
  template<class T, class E> struct learner;
  using base_learner = learner<char, char>;
}

struct vw;

namespace CCB {
  // Each positon in outer array is implicitly the decision corresponding to that index. Each inner array is the result of CB for that call.
  typedef v_array<ACTION_SCORE::action_scores> decision_scores_t;
  
  LEARNER::base_learner* ccb_explore_adf_setup(VW::config::options_i& options, vw& all);
}
