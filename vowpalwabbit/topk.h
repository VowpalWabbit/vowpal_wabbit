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
  struct topk
  {
    using scored_example = std::pair<float, v_array<char>>;

    topk(uint32_t K);

    void predict(LEARNER::single_learner& base, multi_ex& ec_seq);
    void learn(LEARNER::single_learner& base, multi_ex& ec_seq);
    std::vector<scored_example> drain_queue();
  private:
    struct compare_scored_examples
    {
      constexpr bool operator()(scored_example const& a, scored_example const& b) const { return a.first > b.first; }
    };

    void update_priority_queue(float pred, v_array<char>& tag);

    // rec number
    const uint32_t m_K;
    std::priority_queue<scored_example, std::vector<scored_example>, compare_scored_examples> m_pr_queue;
  };
}
