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

//TODO: Remove the elements that are in predict_data
// ccb_label.cc will need a major revamp before that can happen
struct label
{
  example_type type;
  // Outcome may be unset.
  conditional_contextual_bandit_outcome* outcome;
  v_array<uint32_t> explicit_included_actions;
  float weight;
};

struct predict_data
{
  example_type type;
  v_array<uint32_t> explicit_included_actions;

  void clear() { explicit_included_actions.clear(); }
};

extern label_parser ccb_label_parser;
}  // namespace CCB
