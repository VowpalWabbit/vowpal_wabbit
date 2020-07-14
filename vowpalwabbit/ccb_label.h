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
  example_type type;
  // Outcome may be unset.
  conditional_contextual_bandit_outcome* outcome;
  v_array<uint32_t> explicit_included_actions;
  float weight;
};

extern label_parser ccb_label_parser;
}  // namespace CCB
