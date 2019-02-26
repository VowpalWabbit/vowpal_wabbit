#pragma once
#include "learner.h"
#include "options.h"
#include "min_depth_tree.h"

namespace VW { namespace offset_tree {

  LEARNER::base_learner* offset_tree_setup(config::options_i& options, vw& all);
  
  struct offset_tree
  {
    void init(uint32_t num_actions);
    int32_t learner_count() const;
    const std::vector<float>& predict(LEARNER::single_learner& base, example& ec);

   private:
    min_depth_binary_tree binary_tree;
  };

}}
