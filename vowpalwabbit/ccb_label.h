// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <cstdint>
#include <vector>

#include "label_parser.h"
#include "v_array.h"
#include "action_score.h"

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

enum example_type : uint8_t
{
  unset = 0,
  shared = 1,
  action = 2,
  slot = 3
};

struct label
{
  example_type type = example_type::unset;
  // Outcome may be unset.
  conditional_contextual_bandit_outcome* outcome = nullptr;
  v_array<uint32_t> explicit_included_actions;
  float weight = 0.f;

  label() = default;
  label(example_type type, conditional_contextual_bandit_outcome* outcome, v_array<uint32_t>& explicit_included_actions,
      float weight)
      : type(type), outcome(outcome), explicit_included_actions(explicit_included_actions), weight(weight)
  {
  }

  label(label&& other)
  {
    type = example_type::unset;
    std::swap(type, other.type);
    outcome = nullptr;
    std::swap(outcome, other.outcome);
    explicit_included_actions.clear();
    std::swap(explicit_included_actions, other.explicit_included_actions);
    weight = 0.f;
    std::swap(weight, other.weight);
  }
  label& operator=(label&& other)
  {
    type = example_type::unset;
    std::swap(type, other.type);
    delete outcome;
    outcome = nullptr;
    std::swap(outcome, other.outcome);

    explicit_included_actions.clear();
    std::swap(explicit_included_actions, other.explicit_included_actions);

    weight = 0.f;
    std::swap(weight, other.weight);

    return *this;
  }

  label(const label& other)
  {
    type = other.type;
    // todo copyconstructor of outcome
    outcome = other.outcome;
    explicit_included_actions.clear();
    copy_array(explicit_included_actions, other.explicit_included_actions);
    weight = other.weight;
  }

  label& operator=(const label& other)
  {
    type = other.type;
    delete outcome;
    outcome = other.outcome;
    explicit_included_actions.clear();
    copy_array(explicit_included_actions, other.explicit_included_actions);
    weight = other.weight;
    return *this;
  }

  ~label() { delete outcome; }
};

extern label_parser ccb_label_parser;

// struct v_ccb_label_parser : public v_label_parser
// {
//   void default_label(new_polylabel&) override;
//   void parse_label(parser*, shared_data*, new_polylabel&, v_array<substring>&) override;
//   void cache_label(new_polylabel&, io_buf& cache) override;
//   void read_cached_label(shared_data*, new_polylabel&, io_buf& cache) override;
//   void delete_label(new_polylabel&) override;
//   float get_weight(new_polylabel&) override;
//   void copy_label(new_polylabel&, new_polylabel&) override;
//   bool test_label(new_polylabel&) override;
//   size_t get_label_size() override;

// };

}  // namespace CCB
