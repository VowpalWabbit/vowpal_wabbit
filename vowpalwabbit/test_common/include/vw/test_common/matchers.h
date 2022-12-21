// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>

MATCHER(ActionScoreEqual, "")
{
  auto lhs_action = std::get<0>(arg).action;
  auto rhs_action = std::get<1>(arg).action;
  testing::Matcher<uint32_t> action_matcher = testing::Eq(lhs_action);

  auto lhs_score = std::get<0>(arg).score;
  auto rhs_score = std::get<1>(arg).score;
  testing::Matcher<float> score_matcher = testing::FloatEq(lhs_score);
  return action_matcher.MatchAndExplain(rhs_action, result_listener) &&
      score_matcher.MatchAndExplain(rhs_score, result_listener);
}
