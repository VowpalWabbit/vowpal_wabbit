// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "simulator.h"

#include <gtest/gtest.h>

TEST(tutorial, cpp_simulator_without_interaction)
{
  auto ctr = simulator::_test_helper("--cb_explore_adf --quiet --epsilon 0.2 --random_seed 5");
  EXPECT_GT(ctr.back(), 0.37f);
  EXPECT_LT(ctr.back(), 0.49f);
}

TEST(tutorial, cpp_simulator_with_interaction)
{
  auto ctr = simulator::_test_helper("--cb_explore_adf -q UA --quiet --epsilon 0.2 --random_seed 5");
  float without_save = ctr.back();
  EXPECT_GT(without_save, 0.7f);

  ctr = simulator::_test_helper_save_load("--cb_explore_adf -q UA --quiet --epsilon 0.2 --random_seed 5");
  float with_save = ctr.back();
  EXPECT_GT(with_save, 0.7f);

  EXPECT_FLOAT_EQ(without_save, with_save);
}