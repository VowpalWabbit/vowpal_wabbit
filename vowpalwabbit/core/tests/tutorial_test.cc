// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "simulator.h"

#include <gtest/gtest.h>

TEST(Tutorial, CppSimulatorWithoutInteraction)
{
  auto ctr = simulator::_test_helper(
      std::vector<std::string>{"--cb_explore_adf", "--quiet", "--epsilon=0.2", "--random_seed=5"});
  EXPECT_GT(ctr.back(), 0.37f);
  EXPECT_LT(ctr.back(), 0.49f);
}

TEST(Tutorial, CppSimulatorWithInteraction)
{
  auto ctr = simulator::_test_helper(
      std::vector<std::string>{"--cb_explore_adf", "--quadratic=UA", "--quiet", "--epsilon=0.2", "--random_seed=5"});
  float without_save = ctr.back();
  EXPECT_GT(without_save, 0.7f);

  ctr = simulator::_test_helper_save_load(
      std::vector<std::string>{"--cb_explore_adf", "--quadratic=UA", "--quiet", "--epsilon=0.2", "--random_seed=5"});
  float with_save = ctr.back();
  EXPECT_GT(with_save, 0.7f);

  EXPECT_FLOAT_EQ(without_save, with_save);
}
TEST(Tutorial, GaussianRewardSimulation) {
  // Set up the parameters for the Gaussian reward simulation
  float mean = 0.0f;
  float stddev = 1.0f;

  // Create a cb_sim instance
  simulator::cb_sim sim;

  // Generate a Gaussian reward using the new method
  float reward = sim.get_gaussian_reward(mean, stddev);

  // Perform any necessary checks to validate the generated reward
  // For example, check if the reward falls within a specific range
  EXPECT_LE(reward, mean + 3 * stddev);
  EXPECT_GE(reward, mean - 3 * stddev);
}
