// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/search/search.h"
#include "vw/core/vw.h"
#include "vw/test_common/test_common.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

// Tests for search reduction options that were previously uncovered

// Test --search_no_caching option
TEST(Search, NoCachingOption)
{
  auto vw = VW::initialize(vwtest::make_args("--search", "5", "--search_task", "sequence", "--search_no_caching",
      "--passes", "2", "--holdout_off", "-k", "-c", "--quiet"));
  ASSERT_NE(vw, nullptr);

  VW::multi_ex examples;
  examples.push_back(VW::read_example(*vw, "1 | a"));
  examples.push_back(VW::read_example(*vw, "2 | b"));
  examples.push_back(VW::read_example(*vw, "3 | c"));
  examples.push_back(VW::read_example(*vw, ""));

  vw->learn(examples);
  vw->finish_example(examples);
}

// Test --search_xv option (cross-validation)
TEST(Search, CrossValidationOption)
{
  auto vw = VW::initialize(vwtest::make_args("--search", "5", "--search_task", "sequence", "--search_xv", "--passes",
      "2", "--holdout_off", "-k", "-c", "--quiet"));
  ASSERT_NE(vw, nullptr);

  VW::multi_ex examples;
  examples.push_back(VW::read_example(*vw, "1 | a"));
  examples.push_back(VW::read_example(*vw, "2 | b"));
  examples.push_back(VW::read_example(*vw, "3 | c"));
  examples.push_back(VW::read_example(*vw, ""));

  vw->learn(examples);
  vw->finish_example(examples);
}

// Test --search_subsample_time option with fixed steps
TEST(Search, SubsampleTimeFixedSteps)
{
  auto vw = VW::initialize(vwtest::make_args("--search", "5", "--search_task", "sequence", "--search_subsample_time",
      "2", "--passes", "2", "--holdout_off", "-k", "-c", "--quiet"));
  ASSERT_NE(vw, nullptr);

  VW::multi_ex examples;
  examples.push_back(VW::read_example(*vw, "1 | a"));
  examples.push_back(VW::read_example(*vw, "2 | b"));
  examples.push_back(VW::read_example(*vw, "3 | c"));
  examples.push_back(VW::read_example(*vw, ""));

  vw->learn(examples);
  vw->finish_example(examples);
}

// Test --search_metatask debug option
TEST(Search, DebugMetatask)
{
  auto vw = VW::initialize(vwtest::make_args("--search", "5", "--search_task", "sequence", "--search_metatask", "debug",
      "--passes", "2", "--holdout_off", "-k", "-c", "--quiet"));
  ASSERT_NE(vw, nullptr);

  VW::multi_ex examples;
  examples.push_back(VW::read_example(*vw, "1 | a"));
  examples.push_back(VW::read_example(*vw, "2 | b"));
  examples.push_back(VW::read_example(*vw, "3 | c"));
  examples.push_back(VW::read_example(*vw, ""));

  vw->learn(examples);
  vw->finish_example(examples);
}

// Test different rollout modes
TEST(Search, RolloutPolicy)
{
  auto vw = VW::initialize(vwtest::make_args("--search", "5", "--search_task", "sequence", "--search_rollout", "policy",
      "--passes", "2", "--holdout_off", "-k", "-c", "--quiet"));
  ASSERT_NE(vw, nullptr);

  VW::multi_ex examples;
  examples.push_back(VW::read_example(*vw, "1 | a"));
  examples.push_back(VW::read_example(*vw, "2 | b"));
  examples.push_back(VW::read_example(*vw, ""));

  vw->learn(examples);
  vw->finish_example(examples);
}

TEST(Search, RolloutLearn)
{
  auto vw = VW::initialize(vwtest::make_args("--search", "5", "--search_task", "sequence", "--search_rollout", "learn",
      "--passes", "2", "--holdout_off", "-k", "-c", "--quiet"));
  ASSERT_NE(vw, nullptr);

  VW::multi_ex examples;
  examples.push_back(VW::read_example(*vw, "1 | a"));
  examples.push_back(VW::read_example(*vw, "2 | b"));
  examples.push_back(VW::read_example(*vw, ""));

  vw->learn(examples);
  vw->finish_example(examples);
}

TEST(Search, RolloutNone)
{
  auto vw = VW::initialize(vwtest::make_args("--search", "5", "--search_task", "sequence", "--search_rollout", "none",
      "--passes", "2", "--holdout_off", "-k", "-c", "--quiet"));
  ASSERT_NE(vw, nullptr);

  VW::multi_ex examples;
  examples.push_back(VW::read_example(*vw, "1 | a"));
  examples.push_back(VW::read_example(*vw, "2 | b"));
  examples.push_back(VW::read_example(*vw, ""));

  vw->learn(examples);
  vw->finish_example(examples);
}

// Test different rollin modes
TEST(Search, RollinPolicy)
{
  auto vw = VW::initialize(vwtest::make_args("--search", "5", "--search_task", "sequence", "--search_rollin", "policy",
      "--passes", "2", "--holdout_off", "-k", "-c", "--quiet"));
  ASSERT_NE(vw, nullptr);

  VW::multi_ex examples;
  examples.push_back(VW::read_example(*vw, "1 | a"));
  examples.push_back(VW::read_example(*vw, "2 | b"));
  examples.push_back(VW::read_example(*vw, ""));

  vw->learn(examples);
  vw->finish_example(examples);
}

TEST(Search, RollinLearn)
{
  auto vw = VW::initialize(vwtest::make_args("--search", "5", "--search_task", "sequence", "--search_rollin", "learn",
      "--passes", "2", "--holdout_off", "-k", "-c", "--quiet"));
  ASSERT_NE(vw, nullptr);

  VW::multi_ex examples;
  examples.push_back(VW::read_example(*vw, "1 | a"));
  examples.push_back(VW::read_example(*vw, "2 | b"));
  examples.push_back(VW::read_example(*vw, ""));

  vw->learn(examples);
  vw->finish_example(examples);
}

// Test argmax task
TEST(Search, ArgmaxTask)
{
  auto vw = VW::initialize(vwtest::make_args(
      "--search", "2", "--search_task", "argmax", "--passes", "2", "--holdout_off", "-k", "-c", "--quiet"));
  ASSERT_NE(vw, nullptr);

  VW::multi_ex examples;
  examples.push_back(VW::read_example(*vw, "1 | a"));
  examples.push_back(VW::read_example(*vw, "2 | b"));
  examples.push_back(VW::read_example(*vw, ""));

  vw->learn(examples);
  vw->finish_example(examples);
}

// Test combined options
TEST(Search, CombinedOptions)
{
  auto vw = VW::initialize(vwtest::make_args("--search", "5", "--search_task", "sequence", "--search_no_caching",
      "--search_linear_ordering", "--search_rollout", "oracle", "--search_rollin", "oracle", "--passes", "2",
      "--holdout_off", "-k", "-c", "--quiet"));
  ASSERT_NE(vw, nullptr);

  VW::multi_ex examples;
  examples.push_back(VW::read_example(*vw, "1 | a"));
  examples.push_back(VW::read_example(*vw, "2 | b"));
  examples.push_back(VW::read_example(*vw, "3 | c"));
  examples.push_back(VW::read_example(*vw, ""));

  vw->learn(examples);
  vw->finish_example(examples);
}
