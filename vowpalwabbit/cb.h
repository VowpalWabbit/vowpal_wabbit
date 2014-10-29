/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#pragma once
namespace CB {

  struct cb_class {
    float cost;  // the cost of this class
    uint32_t action;  // the index of this class
    float probability; //new for bandit setting, specifies the probability the data collection policy chose this class for importance weighting
    float partial_prediction;//essentially a return value
    bool operator==(cb_class j){return action == j.action;}
  };

  struct label {
    v_array<cb_class> costs;
    uint32_t prediction;
  };

  extern label_parser cb_label;//for learning
}

namespace CB_EVAL {
  extern label_parser cb_eval;//for evaluation of an arbitrary policy.
}
