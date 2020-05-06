/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#pragma once

#include "label_parser.h"

struct example;

namespace VW { namespace cb_continuous
{
  struct continuous_label_elm
  {
    float action;       // the continuous action
    float cost;         // the cost of this class
    float probability;  // new for bandit setting, specifies the probability the data collection policy chose this class

    bool operator==(continuous_label_elm j) { return action == j.action; }
  };

  struct continuous_label
  {
    v_array<continuous_label_elm> costs;
  };

  extern label_parser the_label_parser;

  std::string to_string(const continuous_label_elm& elm);
  std::string to_string(continuous_label& lbl);
  }  // namespace cb_continuous
  }  // namespace VW
