// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/fast_pow10.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

bool are_same(float a, float b) { return std::abs(a - b) < std::numeric_limits<float>::epsilon(); }

TEST(Pow10, EqualityWithStdPow)
{
  // In reality most of these tests would all evaluate to the same. The reason is that the epsilon is usually
  // around 1.2e-7
  const float base = 10;
  EXPECT_TRUE(are_same(VW::fast_pow10(-127), static_cast<float>(std::pow(base, -127))));
  EXPECT_TRUE(are_same(VW::fast_pow10(-46), static_cast<float>(std::pow(base, -46))));
  EXPECT_TRUE(are_same(VW::fast_pow10(-45), static_cast<float>(std::pow(base, -45))));
  EXPECT_TRUE(are_same(VW::fast_pow10(-44), static_cast<float>(std::pow(base, -44))));
  EXPECT_TRUE(are_same(VW::fast_pow10(-40), static_cast<float>(std::pow(base, -40))));
  EXPECT_TRUE(are_same(VW::fast_pow10(-38), static_cast<float>(std::pow(base, -38))));
  EXPECT_TRUE(are_same(VW::fast_pow10(-37), static_cast<float>(std::pow(base, -37))));
  EXPECT_TRUE(are_same(VW::fast_pow10(-10), static_cast<float>(std::pow(base, -10))));
  EXPECT_TRUE(are_same(VW::fast_pow10(-5), static_cast<float>(std::pow(base, -5))));
  EXPECT_FLOAT_EQ(VW::fast_pow10(0), 1.f);
  EXPECT_FLOAT_EQ(VW::fast_pow10(5), 1e5f);
  EXPECT_FLOAT_EQ(VW::fast_pow10(10), 1e10f);
  EXPECT_FLOAT_EQ(VW::fast_pow10(37), 1e37f);
  EXPECT_FLOAT_EQ(VW::fast_pow10(38), 1e38f);
  EXPECT_TRUE(std::isinf(VW::fast_pow10(39)));
  EXPECT_TRUE(std::isinf(VW::fast_pow10(127)));
}
