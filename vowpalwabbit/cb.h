// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include <vector>
#include <cfloat>
#include <cstdint>
#include <utility>

#include "reductions_fwd.h"
#include "label_parser.h"
#include "v_array.h"
#include "io_buf.h"

struct example;
using multi_ex = std::vector<example*>;

namespace CB
{
// By default a cb class does not contain an observed cost.
struct cb_class
{
  float cost = FLT_MAX;      // the cost of this class
  uint32_t action = 0;       // the index of this class
  float probability = -1.f;  // new for bandit setting, specifies the probability the data collection policy chose this
                             // class for importance weighting
  float partial_prediction = 0.f;  // essentially a return value

  cb_class() = default;
  cb_class(float cost, uint32_t action, float probability)
      : cost(cost), action(action), probability(probability), partial_prediction(0.f)
  {
  }

  bool operator==(cb_class j) const { return action == j.action; }

  constexpr bool has_observed_cost() const { return (cost != FLT_MAX && probability > .0); }
};

struct label
{
  std::vector<cb_class> costs;
  float weight = 1.f;
};

extern label_parser cb_label;                  // for learning
bool ec_is_example_header(example const& ec);  // example headers look like "shared"

std::pair<bool, cb_class> get_observed_cost_cb(const label& ld);

void print_update(VW::workspace& all, bool is_test, const example& ec, const multi_ex* ec_seq, bool action_scores,
    const CB::cb_class* known_cost);
}  // namespace CB

namespace CB_EVAL
{
struct label
{
  uint32_t action = 0;
  CB::label event;
};

extern label_parser cb_eval;  // for evaluation of an arbitrary policy.
}  // namespace CB_EVAL

namespace VW
{
namespace model_utils
{
size_t read_model_field(io_buf&, CB::cb_class&);
size_t write_model_field(io_buf&, const CB::cb_class&, const std::string&, bool);
size_t read_model_field(io_buf&, CB::label&);
size_t write_model_field(io_buf&, const CB::label&, const std::string&, bool);
size_t read_model_field(io_buf&, CB_EVAL::label&);
size_t write_model_field(io_buf&, const CB_EVAL::label&, const std::string&, bool);
}  // namespace model_utils
}  // namespace VW
