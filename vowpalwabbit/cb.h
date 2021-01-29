// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include <vector>
#include <cstdint>

#include "reductions_fwd.h"
#include "label_parser.h"
#include "v_array.h"

struct example;
using multi_ex = std::vector<example*>;

namespace CB
{

// By default a cb class does not contain an observed cost.
struct cb_class
{
  float cost = FLT_MAX;             // the cost of this class
  uint32_t action = std::numeric_limits<uint32_t>::max();              // the index of this class
  float probability = 0.f;          // new for bandit setting, specifies the probability the data collection policy chose this class
                                    // for importance weighting
  float partial_prediction = 0.f;   // essentially a return value

  bool operator==(cb_class j) const { return action == j.action; }

  constexpr bool has_observed_cost() const {
    return (cost != FLT_MAX && probability > .0);
  }
};

struct label
{
  v_array<cb_class> costs;
  float weight;
};

extern label_parser cb_label;                  // for learning
bool ec_is_example_header(example const& ec);  // example headers look like "shared"

cb_class get_observed_cost_or_default_cb(const label& ld);

void print_update(vw& all, bool is_test, example& ec, std::vector<example*>* ec_seq, bool action_scores);
}  // namespace CB

namespace CB_EVAL
{
struct label
{
  uint32_t action;
  CB::label event;
};

extern label_parser cb_eval;  // for evaluation of an arbitrary policy.
}  // namespace CB_EVAL
