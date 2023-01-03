// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/vw_math.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(MathTests, MathFactorial)
{
  EXPECT_EQ(VW::math::factorial(0), 1);
  EXPECT_EQ(VW::math::factorial(1), 1);
  EXPECT_EQ(VW::math::factorial(2), 2);
  EXPECT_EQ(VW::math::factorial(3), 6);
  EXPECT_EQ(VW::math::factorial(4), 24);
  EXPECT_EQ(VW::math::factorial(5), 120);
  EXPECT_EQ(VW::math::factorial(10), 3628800);
}

TEST(MathTests, MathNumberOfCombinationsWithRepetition)
{
  EXPECT_EQ(VW::math::number_of_combinations_with_repetition(1, 1), 1);
  EXPECT_EQ(VW::math::number_of_combinations_with_repetition(5, 1), 5);
  EXPECT_EQ(VW::math::number_of_combinations_with_repetition(1, 5), 1);
  EXPECT_EQ(VW::math::number_of_combinations_with_repetition(5, 5), 126);
  EXPECT_EQ(VW::math::number_of_combinations_with_repetition(10, 2), 55);
}

TEST(MathTests, MathNumberOfPermutationsWithRepetition)
{
  EXPECT_EQ(VW::math::number_of_permutations_with_repetition(1, 1), 1);
  EXPECT_EQ(VW::math::number_of_permutations_with_repetition(5, 1), 5);
  EXPECT_EQ(VW::math::number_of_permutations_with_repetition(1, 5), 1);
  EXPECT_EQ(VW::math::number_of_permutations_with_repetition(5, 5), 3125);
  EXPECT_EQ(VW::math::number_of_permutations_with_repetition(10, 2), 100);
}

TEST(MathTests, MathSign)
{
  EXPECT_FLOAT_EQ(VW::math::sign(1.f), 1.f);
  EXPECT_FLOAT_EQ(VW::math::sign(-1.f), -1.f);
  EXPECT_FLOAT_EQ(VW::math::sign(0.f), -1.f);
  EXPECT_FLOAT_EQ(VW::math::sign(0.1f), 1.f);
  EXPECT_FLOAT_EQ(VW::math::sign(1000.f), 1.f);
  EXPECT_FLOAT_EQ(VW::math::sign(-999.f), -1.f);
}

TEST(MathTests, MathChoose)
{
  EXPECT_EQ(VW::math::choose(1, 1), 1);
  EXPECT_EQ(VW::math::choose(4, 2), 6);
  EXPECT_EQ(VW::math::choose(2, 4), 0);
  EXPECT_EQ(VW::math::choose(0, 0), 1);
  EXPECT_EQ(VW::math::choose(0, 1), 0);
  EXPECT_EQ(VW::math::choose(1, 0), 1);
}