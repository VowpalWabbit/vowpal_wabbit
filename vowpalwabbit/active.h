#pragma once
#include <memory>

struct rand_state;

struct active
{
  float active_c0;
  vw* all;  // statistics, loss
  std::shared_ptr<rand_state> m_random_state;
};

LEARNER::base_learner* active_setup(VW::config::options_i& options, vw& all);
