// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/common/random.h"

#include "vw/common/hash.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(Rand, ReproduceMaxBoundaryIssue)
{
  uint64_t seed = 58587211;
  const uint64_t new_random_seed =
      VW::uniform_hash(reinterpret_cast<const char*>(&seed), sizeof(seed), static_cast<uint32_t>(seed));
  EXPECT_EQ(new_random_seed, 2244123448);

  float random_draw = VW::details::merand48_noadvance(new_random_seed);
  EXPECT_NEAR(random_draw, 0.99999f, 0.001f);

  const float range_max = 7190.0f;
  const float range_min = -121830.0f;
  const float interval_size = (range_max - range_min) / (32);
  float chosen_value = interval_size * (random_draw + 31) + range_min;
  EXPECT_FLOAT_EQ(chosen_value, range_max);
}

TEST(Rand, CheckRandStateCrossPlatform)
{
  VW::rand_state random_state;
  random_state.set_random_state(10);
  EXPECT_FLOAT_EQ(random_state.get_and_update_random(), 0.0170166492);
  EXPECT_FLOAT_EQ(random_state.get_and_update_random(), 0.370730162);
  EXPECT_FLOAT_EQ(random_state.get_and_update_random(), 0.166691661);
  EXPECT_FLOAT_EQ(random_state.get_and_update_random(), 0.362691283);
  EXPECT_FLOAT_EQ(random_state.get_and_update_random(), 0.969445825);
}
