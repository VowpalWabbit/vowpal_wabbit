// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <cstdint>
#include <vector>
#include <string>

#include "action_score.h"
#include "ccb_reduction_features.h"
#include "label_parser.h"
#include "v_array.h"
#include "vw_string_view.h"

// TODO: This header can be removed once type and explicit_included_actions are removed from the label
#include <fmt/format.h>

namespace CCB
{
struct conditional_contextual_bandit_outcome
{
  // The cost of this class
  float cost;

  // Either probability for top action or for all actions in action set.
  // Top action is always in first position.
  ACTION_SCORE::action_scores probabilities;
};

// TODO: Remove the elements that are in reduction_features
// ccb_label.cc will need a major revamp before that can happen
struct label
{
  example_type type = CCB::example_type::unset;
  // Outcome may be unset.
  conditional_contextual_bandit_outcome* outcome = nullptr;
  VW::v_array<uint32_t> explicit_included_actions;
  float weight = 0.f;

  label() = default;

  label(label&& other) noexcept
  {
    type = example_type::unset;
    outcome = nullptr;
    weight = 0.f;

    std::swap(type, other.type);
    std::swap(outcome, other.outcome);
    std::swap(explicit_included_actions, other.explicit_included_actions);
    std::swap(weight, other.weight);
  }

  label& operator=(label&& other) noexcept
  {
    std::swap(type, other.type);
    std::swap(outcome, other.outcome);
    std::swap(explicit_included_actions, other.explicit_included_actions);
    std::swap(weight, other.weight);
    return *this;
  }

  label(const label& other)
  {
    type = other.type;
    outcome = nullptr;
    if (other.outcome)
    {
      outcome = new conditional_contextual_bandit_outcome();
      *outcome = *other.outcome;
    }
    explicit_included_actions = other.explicit_included_actions;
    weight = other.weight;
  }

  label& operator=(const label& other)
  {
    if (this == &other) return *this;

    if (outcome)
    {
      delete outcome;
      outcome = nullptr;
    }

    type = other.type;
    outcome = nullptr;
    if (other.outcome)
    {
      outcome = new conditional_contextual_bandit_outcome();
      *outcome = *other.outcome;
    }
    explicit_included_actions = other.explicit_included_actions;
    weight = other.weight;
    return *this;
  }

  ~label()
  {
    if (outcome)
    {
      delete outcome;
      outcome = nullptr;
    }
  }
};

void default_label(CCB::label& ld);
void parse_label(label& ld, VW::label_parser_reuse_mem& reuse_mem, const std::vector<VW::string_view>& words,
    VW::io::logger& logger);

extern VW::label_parser ccb_label_parser;
}  // namespace CCB

namespace VW
{
namespace model_utils
{
size_t read_model_field(io_buf&, CCB::conditional_contextual_bandit_outcome&);
size_t write_model_field(io_buf&, const CCB::conditional_contextual_bandit_outcome&, const std::string&, bool);
size_t read_model_field(io_buf&, CCB::label&);
size_t write_model_field(io_buf&, const CCB::label&, const std::string&, bool);
}  // namespace model_utils
}  // namespace VW

namespace fmt
{
template <>
struct formatter<CCB::example_type> : formatter<std::string>
{
  auto format(CCB::example_type c, format_context& ctx) -> decltype(ctx.out())
  {
    return formatter<std::string>::format(std::string{VW::to_string(c)}, ctx);
  }
};
}  // namespace fmt
