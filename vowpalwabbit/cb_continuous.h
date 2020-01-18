/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#pragma once

#include "label_parser.h"
#include <vector>

struct example;

namespace VW { namespace cb_continuous
{
  struct continuous_label_elm
  {
    float action;       // the continuous action
    float cost;         // the cost of this class
    float probability;  // new for bandit setting, specifies the probability the data collection policy chose this class
                        // for importance weighting
    float partial_prediction;  // essentially a return value
    bool operator==(continuous_label_elm j) { return action == j.action; }
  };

  struct continuous_label
  {
    v_array<continuous_label_elm> costs;
  };

  extern label_parser cb_cont_label;            // for learning
  bool ec_is_example_header(example& ec);  // example headers look like "0:-1" or just "shared"

  void print_update(vw& all, bool is_test, example& ec, std::vector<example*>* ec_seq, bool action_scores); // TODO: why it says it has not defined
}}  // namespace VW::cb_continuous
