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


//TODO: Remove the elements that are in reduction_features
// ccb_label.cc will need a major revamp before that can happen
struct label
{
  example_type type;
  // Outcome may be unset.
  conditional_contextual_bandit_outcome* outcome;
  v_array<uint32_t> explicit_included_actions;
  float weight;

  // label() : type(example_type::unset), outcome(nullptr), explicit_included_actions({nullptr, nullptr, nullptr, 0}),
  // weight(1.f) {} label(example_type type, conditional_contextual_bandit_outcome* outcome, v_array<uint32_t>&
  // explicit_included_actions, float weight) : type(type), outcome(outcome),
  // explicit_included_actions(explicit_included_actions), weight(weight) {}
};

void default_label(CCB::label& ld);
void delete_label(CCB::label& ld);
void parse_label(parser* p, shared_data*, CCB::label& ld, std::vector<VW::string_view>& words, ::reduction_features&);
void cache_label(CCB::label& ld, io_buf& cache);
size_t read_cached_label(shared_data*, CCB::label& ld, io_buf& cache);
void copy_label(CCB::label& ldDst, CCB::label& ldSrc);

extern label_parser ccb_label_parser;
}  // namespace CCB
