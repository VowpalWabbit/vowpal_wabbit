/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */

#pragma once

#include "reductions.h"
#include <queue>
#include <utility>

LEARNER::base_learner* topk_setup(VW::config::options_i& options, vw& all);

namespace VW
{
  class topk
  {
    using container_t = std::multimap<float, v_array<char>>;
  public:
    using const_iterator_t = container_t::const_iterator;
    topk(uint32_t k_num);

    void predict(LEARNER::single_learner& base, multi_ex& ec_seq);
    void learn(LEARNER::single_learner& base, multi_ex& ec_seq);
    std::pair<const_iterator_t, const_iterator_t> get_container_view();
    void clear_container();
  private:
    void update_priority_queue(float pred, v_array<char>& tag);

    const uint32_t _k_num;
    container_t _pr_queue;
  };
}
