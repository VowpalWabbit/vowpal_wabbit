// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <cstdint>
#include <vector>

#include "label_parser.h"
#include "v_array.h"
#include "action_score.h"
// TODO: This header can be removed once type and explicit_included_actions are removed from the label
#include "ccb_reduction_features.h"

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
  example_type type;
  // Outcome may be unset.
  conditional_contextual_bandit_outcome* outcome = nullptr;
  v_array<uint32_t> explicit_included_actions;
  float weight;

  label() { reset_to_default(); }

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

  void reset_to_default()
  {
    // This is tested against nullptr, so unfortunately as things are this must be deleted when not used.
    if (outcome != nullptr)
    {
      delete outcome;
      outcome = nullptr;
    }

    explicit_included_actions.clear();
    type = example_type::unset;
    weight = 1.f;
  }
};

void default_label(CCB::label& ld);
void parse_label(parser* p, shared_data*, CCB::label& ld, std::vector<VW::string_view>& words, ::reduction_features&);
void cache_label(CCB::label& ld, io_buf& cache);
size_t read_cached_label(shared_data*, CCB::label& ld, io_buf& cache);

extern label_parser ccb_label_parser;
}  // namespace CCB
