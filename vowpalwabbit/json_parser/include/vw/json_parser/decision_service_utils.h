// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace VW
{
namespace parsers
{
namespace json
{
// Decision Service JSON header information - required to construct final label
class decision_service_interaction
{
public:
  std::string event_id;
  std::string timestamp;
  std::vector<unsigned> actions;
  std::vector<float> probabilities;
  std::vector<unsigned> baseline_actions;
  float probability_of_drop = 0.f;
  float original_label_cost = 0.f;
  float original_label_cost_first_slot = 0.f;
  bool skip_learn{false};
};
}  // namespace json
}  // namespace parsers
}  // namespace VW
