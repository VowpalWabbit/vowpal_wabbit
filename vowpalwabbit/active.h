#pragma once

struct active
{
  float active_c0;
  vw* all;  // statistics, loss
  rand_state* m_random_state;
};

float query_decision(active& a, example& ec, float k);
LEARNER::base_learner* active_setup(VW::config::options_i& options, vw& all);
