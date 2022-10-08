// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/core/label_parser.h"
#include "vw/core/v_array.h"
#include "vw/core/vw_fwd.h"

#include <cfloat>
#include <cstdint>
#include <utility>
#include <vector>

namespace VW
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

  bool operator==(const cb_class& j) const { return action == j.action; }

  constexpr bool has_observed_cost() const { return (cost != FLT_MAX && probability > .0); }
};

struct cb_label
{
  std::vector<cb_class> costs;
  float weight = 1.f;
};

extern VW::label_parser cb_label_parser_global;

// example headers look like "shared"
bool is_cb_example_header(VW::example const& ec);

std::pair<bool, cb_class> get_observed_cost_cb(const cb_label& ld);

struct cb_eval_label
{
  uint32_t action = 0;
  cb_label event;
};
extern VW::label_parser cb_eval_label_parser_global;  // for evaluation of an arbitrary policy.

namespace details
{
void print_cb_update(VW::workspace& all, bool is_test, const VW::example& ec, const VW::multi_ex* ec_seq,
    bool action_scores, const cb_class* known_cost);
}
}  // namespace VW

namespace VW
{
namespace model_utils
{
size_t read_model_field(io_buf&, cb_class&);
size_t write_model_field(io_buf&, const cb_class&, const std::string&, bool);
size_t read_model_field(io_buf&, cb_label&);
size_t write_model_field(io_buf&, const cb_label&, const std::string&, bool);
size_t read_model_field(io_buf&, cb_eval_label&);
size_t write_model_field(io_buf&, const cb_eval_label&, const std::string&, bool);
}  // namespace model_utils
}  // namespace VW

// Deprecated types

namespace CB  // NOLINT
{
using cb_class VW_DEPRECATED(
    "VW::cb_class renamed to VW::cb_class. VW::cb_class will be removed in VW 10.") = VW::cb_class;
using label VW_DEPRECATED(
    "VW::cb_label renamed to VW::cb_class. VW::cb_label will be removed in VW 10.") = VW::cb_label;

VW_DEPRECATED(
    "CB::ec_is_example_header renamed to VW::is_cb_example_header. CB::ec_is_example_header will be removed in VW 10.")
inline bool ec_is_example_header(VW::example const& ec) { return VW::is_cb_example_header(ec); }

VW_DEPRECATED(
    "CB::get_observed_cost_cb renamed to VW::get_observed_cost_cb. CB::get_observed_cost_cb will be removed in VW 10.")
inline std::pair<bool, cb_class> get_observed_cost_cb(const label& ld) { return VW::get_observed_cost_cb(ld); }
}  // namespace CB

namespace CB_EVAL  // NOLINT
{
using label VW_DEPRECATED(
    "CB_EVAL::label renamed to VW::cb_eval_label. CB_EVAL::label will be removed in VW 10.") = VW::cb_eval_label;
}  // namespace CB_EVAL