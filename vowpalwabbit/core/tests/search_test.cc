// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/search/search.h"

#include "vw/core/vw.h"
#include "vw/test_common/test_common.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstdio>
#include <string>

namespace
{
// Passing -c gives every test the same cache path: the default is
// data_filename + ".cache", and with no data file that is just "./.cache".
// Tests running concurrently under `ctest --parallel` then write over each
// other's cache and fail the second pass with "need a cache file for multiple
// passes". Give each test its own file instead.
std::string test_cache_file()
{
  const auto* info = ::testing::UnitTest::GetInstance()->current_test_info();
  return std::string("./") + info->test_suite_name() + "." + info->name() + ".cache.tmp";
}

// The cache is written to <name>.writing and only renamed to <name> once the
// write is finalized, which these tests do not reach. Remove both.
void remove_cache_file(const std::string& path)
{
  std::remove(path.c_str());
  std::remove((path + ".writing").c_str());
}
}  // namespace

// Tests for search reduction options that were previously uncovered

// Test --search_no_caching option
TEST(Search, NoCachingOption)
{
  const auto cache_file = test_cache_file();
  auto vw = VW::initialize(vwtest::make_args("--search", "5", "--search_task", "sequence", "--search_no_caching",
      "--passes", "2", "--holdout_off", "-k", "--cache_file", cache_file, "--quiet"));
  ASSERT_NE(vw, nullptr);

  VW::multi_ex examples;
  examples.push_back(VW::read_example(*vw, "1 | a"));
  examples.push_back(VW::read_example(*vw, "2 | b"));
  examples.push_back(VW::read_example(*vw, "3 | c"));
  examples.push_back(VW::read_example(*vw, ""));

  vw->learn(examples);
  vw->finish_example(examples);
  remove_cache_file(cache_file);
}

// Test --search_xv option (cross-validation)
TEST(Search, CrossValidationOption)
{
  const auto cache_file = test_cache_file();
  auto vw = VW::initialize(vwtest::make_args("--search", "5", "--search_task", "sequence", "--search_xv", "--passes",
      "2", "--holdout_off", "-k", "--cache_file", cache_file, "--quiet"));
  ASSERT_NE(vw, nullptr);

  VW::multi_ex examples;
  examples.push_back(VW::read_example(*vw, "1 | a"));
  examples.push_back(VW::read_example(*vw, "2 | b"));
  examples.push_back(VW::read_example(*vw, "3 | c"));
  examples.push_back(VW::read_example(*vw, ""));

  vw->learn(examples);
  vw->finish_example(examples);
  remove_cache_file(cache_file);
}

// Test --search_subsample_time option with fixed steps
TEST(Search, SubsampleTimeFixedSteps)
{
  const auto cache_file = test_cache_file();
  auto vw = VW::initialize(vwtest::make_args("--search", "5", "--search_task", "sequence", "--search_subsample_time",
      "2", "--passes", "2", "--holdout_off", "-k", "--cache_file", cache_file, "--quiet"));
  ASSERT_NE(vw, nullptr);

  VW::multi_ex examples;
  examples.push_back(VW::read_example(*vw, "1 | a"));
  examples.push_back(VW::read_example(*vw, "2 | b"));
  examples.push_back(VW::read_example(*vw, "3 | c"));
  examples.push_back(VW::read_example(*vw, ""));

  vw->learn(examples);
  vw->finish_example(examples);
  remove_cache_file(cache_file);
}

// Test --search_metatask debug option
TEST(Search, DebugMetatask)
{
  const auto cache_file = test_cache_file();
  auto vw = VW::initialize(vwtest::make_args("--search", "5", "--search_task", "sequence", "--search_metatask", "debug",
      "--passes", "2", "--holdout_off", "-k", "--cache_file", cache_file, "--quiet"));
  ASSERT_NE(vw, nullptr);

  VW::multi_ex examples;
  examples.push_back(VW::read_example(*vw, "1 | a"));
  examples.push_back(VW::read_example(*vw, "2 | b"));
  examples.push_back(VW::read_example(*vw, "3 | c"));
  examples.push_back(VW::read_example(*vw, ""));

  vw->learn(examples);
  vw->finish_example(examples);
  remove_cache_file(cache_file);
}

// Test different rollout modes
TEST(Search, RolloutPolicy)
{
  const auto cache_file = test_cache_file();
  auto vw = VW::initialize(vwtest::make_args("--search", "5", "--search_task", "sequence", "--search_rollout", "policy",
      "--passes", "2", "--holdout_off", "-k", "--cache_file", cache_file, "--quiet"));
  ASSERT_NE(vw, nullptr);

  VW::multi_ex examples;
  examples.push_back(VW::read_example(*vw, "1 | a"));
  examples.push_back(VW::read_example(*vw, "2 | b"));
  examples.push_back(VW::read_example(*vw, ""));

  vw->learn(examples);
  vw->finish_example(examples);
  remove_cache_file(cache_file);
}

TEST(Search, RolloutLearn)
{
  const auto cache_file = test_cache_file();
  auto vw = VW::initialize(vwtest::make_args("--search", "5", "--search_task", "sequence", "--search_rollout", "learn",
      "--passes", "2", "--holdout_off", "-k", "--cache_file", cache_file, "--quiet"));
  ASSERT_NE(vw, nullptr);

  VW::multi_ex examples;
  examples.push_back(VW::read_example(*vw, "1 | a"));
  examples.push_back(VW::read_example(*vw, "2 | b"));
  examples.push_back(VW::read_example(*vw, ""));

  vw->learn(examples);
  vw->finish_example(examples);
  remove_cache_file(cache_file);
}

TEST(Search, RolloutNone)
{
  const auto cache_file = test_cache_file();
  auto vw = VW::initialize(vwtest::make_args("--search", "5", "--search_task", "sequence", "--search_rollout", "none",
      "--passes", "2", "--holdout_off", "-k", "--cache_file", cache_file, "--quiet"));
  ASSERT_NE(vw, nullptr);

  VW::multi_ex examples;
  examples.push_back(VW::read_example(*vw, "1 | a"));
  examples.push_back(VW::read_example(*vw, "2 | b"));
  examples.push_back(VW::read_example(*vw, ""));

  vw->learn(examples);
  vw->finish_example(examples);
  remove_cache_file(cache_file);
}

// Test different rollin modes
TEST(Search, RollinPolicy)
{
  const auto cache_file = test_cache_file();
  auto vw = VW::initialize(vwtest::make_args("--search", "5", "--search_task", "sequence", "--search_rollin", "policy",
      "--passes", "2", "--holdout_off", "-k", "--cache_file", cache_file, "--quiet"));
  ASSERT_NE(vw, nullptr);

  VW::multi_ex examples;
  examples.push_back(VW::read_example(*vw, "1 | a"));
  examples.push_back(VW::read_example(*vw, "2 | b"));
  examples.push_back(VW::read_example(*vw, ""));

  vw->learn(examples);
  vw->finish_example(examples);
  remove_cache_file(cache_file);
}

TEST(Search, RollinLearn)
{
  const auto cache_file = test_cache_file();
  auto vw = VW::initialize(vwtest::make_args("--search", "5", "--search_task", "sequence", "--search_rollin", "learn",
      "--passes", "2", "--holdout_off", "-k", "--cache_file", cache_file, "--quiet"));
  ASSERT_NE(vw, nullptr);

  VW::multi_ex examples;
  examples.push_back(VW::read_example(*vw, "1 | a"));
  examples.push_back(VW::read_example(*vw, "2 | b"));
  examples.push_back(VW::read_example(*vw, ""));

  vw->learn(examples);
  vw->finish_example(examples);
  remove_cache_file(cache_file);
}

// Test argmax task
TEST(Search, ArgmaxTask)
{
  const auto cache_file = test_cache_file();
  auto vw = VW::initialize(vwtest::make_args("--search", "2", "--search_task", "argmax", "--passes", "2",
      "--holdout_off", "-k", "--cache_file", cache_file, "--quiet"));
  ASSERT_NE(vw, nullptr);

  VW::multi_ex examples;
  examples.push_back(VW::read_example(*vw, "1 | a"));
  examples.push_back(VW::read_example(*vw, "2 | b"));
  examples.push_back(VW::read_example(*vw, ""));

  vw->learn(examples);
  vw->finish_example(examples);
  remove_cache_file(cache_file);
}

// Test combined options
TEST(Search, CombinedOptions)
{
  const auto cache_file = test_cache_file();
  auto vw = VW::initialize(vwtest::make_args("--search", "5", "--search_task", "sequence", "--search_no_caching",
      "--search_linear_ordering", "--search_rollout", "oracle", "--search_rollin", "oracle", "--passes", "2",
      "--holdout_off", "-k", "--cache_file", cache_file, "--quiet"));
  ASSERT_NE(vw, nullptr);

  VW::multi_ex examples;
  examples.push_back(VW::read_example(*vw, "1 | a"));
  examples.push_back(VW::read_example(*vw, "2 | b"));
  examples.push_back(VW::read_example(*vw, "3 | c"));
  examples.push_back(VW::read_example(*vw, ""));

  vw->learn(examples);
  vw->finish_example(examples);
  remove_cache_file(cache_file);
}
