/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#pragma once

#include "label_parser.h"

namespace CB
{
struct cb_class
{ float cost;  // the cost of this class
  uint32_t action;  // the index of this class
  float probability; //new for bandit setting, specifies the probability the data collection policy chose this class for importance weighting
  float partial_prediction;//essentially a return value
  bool operator==(cb_class j) {return action == j.action;}
};

struct label
{ v_array<cb_class> costs;
};

bool is_test_label(CB::label& ld);

extern label_parser cb_label;//for learning
bool example_is_test(example& ec);
bool ec_is_example_header(example& ec);  // example headers look like "0:-1" or just "shared"

void print_update(vw& all, bool is_test, example& ec, v_array<example*>* ec_seq, bool multilabel);
}

namespace CB_EVAL
{
struct label
{ uint32_t action;
  CB::label event;
};

extern label_parser cb_eval;//for evaluation of an arbitrary policy.
}
