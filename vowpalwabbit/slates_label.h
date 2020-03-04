// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <cstdint>

#include "label_parser.h"
#include "action_score.h"

namespace slates
{

enum example_type : uint8_t
{
  unset = 0,
  shared = 1,
  action = 2,
  slot = 3
};

struct label
{
  // General data
  example_type type;
  float weight;
  bool labeled;

  // For shared examples
  // Only valid if labeled
  float cost;

  // For action examples
  size_t slot_id;

  // For slot examples
  // Only valid if labeled
  ACTION_SCORE::action_scores probabilities;
};

extern label_parser slates_label_parser;
}  // namespace slates
