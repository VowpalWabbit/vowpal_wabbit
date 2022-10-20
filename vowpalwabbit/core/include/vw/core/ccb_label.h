// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/common/future_compat.h"
#include "vw/common/string_view.h"
#include "vw/core/action_score.h"
#include "vw/core/ccb_reduction_features.h"
#include "vw/core/label_parser.h"
#include "vw/core/v_array.h"

#include <fmt/format.h>

#include <cstdint>
#include <string>
#include <vector>

// TODO: The ccb_reduction_features.h header can be removed once type and
// explicit_included_actions are removed from the label

namespace VW
{
class ccb_outcome
{
public:
  // The cost of this class
  float cost = 0.f;

  // Either probability for top action or for all actions in action set.
  // Top action is always in first position.
  VW::action_scores probabilities;
};

// TODO: Remove the elements that are in reduction_features
// ccb_label.cc will need a major revamp before that can happen
class ccb_label
{
public:
  ccb_example_type type = ccb_example_type::UNSET;
  // Outcome may be unset.
  ccb_outcome* outcome = nullptr;
  VW::v_array<uint32_t> explicit_included_actions;
  float weight = 0.f;

  ccb_label() = default;

  ccb_label(ccb_label&& other) noexcept
  {
    type = ccb_example_type::UNSET;
    outcome = nullptr;
    weight = 0.f;

    std::swap(type, other.type);
    std::swap(outcome, other.outcome);
    std::swap(explicit_included_actions, other.explicit_included_actions);
    std::swap(weight, other.weight);
  }

  ccb_label& operator=(ccb_label&& other) noexcept
  {
    std::swap(type, other.type);
    std::swap(outcome, other.outcome);
    std::swap(explicit_included_actions, other.explicit_included_actions);
    std::swap(weight, other.weight);
    return *this;
  }

  ccb_label(const ccb_label& other)
  {
    type = other.type;
    outcome = nullptr;
    if (other.outcome)
    {
      outcome = new ccb_outcome();
      *outcome = *other.outcome;
    }
    explicit_included_actions = other.explicit_included_actions;
    weight = other.weight;
  }

  ccb_label& operator=(const ccb_label& other)
  {
    if (this == &other) { return *this; }

    if (outcome)
    {
      delete outcome;
      outcome = nullptr;
    }

    type = other.type;
    outcome = nullptr;
    if (other.outcome)
    {
      outcome = new ccb_outcome();
      *outcome = *other.outcome;
    }
    explicit_included_actions = other.explicit_included_actions;
    weight = other.weight;
    return *this;
  }

  ~ccb_label()
  {
    if (outcome)
    {
      delete outcome;
      outcome = nullptr;
    }
  }
};

void default_ccb_label(ccb_label& ld);
void parse_ccb_label(ccb_label& ld, VW::label_parser_reuse_mem& reuse_mem, const std::vector<VW::string_view>& words,
    VW::io::logger& logger);

extern VW::label_parser ccb_label_parser_global;
}  // namespace VW

namespace VW
{
namespace model_utils
{
size_t read_model_field(io_buf&, ccb_outcome&);
size_t write_model_field(io_buf&, const ccb_outcome&, const std::string&, bool);
size_t read_model_field(io_buf&, ccb_label&);
size_t write_model_field(io_buf&, const ccb_label&, const std::string&, bool);
}  // namespace model_utils
}  // namespace VW

namespace fmt
{
template <>
class formatter<VW::ccb_example_type> : public formatter<std::string>
{
public:
  auto format(VW::ccb_example_type c, format_context& ctx) -> decltype(ctx.out())
  {
    return formatter<std::string>::format(std::string{VW::to_string(c)}, ctx);
  }
};
}  // namespace fmt

namespace CCB
{
using conditional_contextual_bandit_outcome VW_DEPRECATED(
    "CCB::conditional_contextual_bandit_outcome renamed to VW::ccb_outcome. CCB::conditional_contextual_bandit_outcome "
    "will be removed in VW 10.") = VW::ccb_outcome;
using label VW_DEPRECATED("CCB::label renamed to VW::ccb_label. CCB::label will be removed in VW 10.") = VW::ccb_label;

VW_DEPRECATED("CCB::default_label renamed to VW::default_ccb_label. CCB::default_label will be removed in VW 10.")
inline void default_label(VW::ccb_label& ld) { VW::default_ccb_label(ld); }

VW_DEPRECATED("CCB::parse_label renamed to VW::parse_ccb_label. CCB::parse_label will be removed in VW 10.")
inline void parse_label(VW::ccb_label& ld, VW::label_parser_reuse_mem& reuse_mem,
    const std::vector<VW::string_view>& words, VW::io::logger& logger)
{
  VW::parse_ccb_label(ld, reuse_mem, words, logger);
}
}  // namespace CCB