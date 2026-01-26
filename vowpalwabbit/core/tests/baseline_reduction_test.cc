// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/baseline.h"

#include "vw/core/example.h"
#include "vw/core/vw.h"
#include "vw/io/io_adapter.h"
#include "vw/test_common/test_common.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cmath>
#include <memory>
#include <vector>

TEST(Baseline, BasicInitialization)
{
  auto vw = VW::initialize(vwtest::make_args("--baseline", "--quiet"));
  EXPECT_NE(vw, nullptr);
}

TEST(Baseline, GlobalOnlyOption)
{
  auto vw = VW::initialize(vwtest::make_args("--baseline", "--global_only", "--quiet"));
  EXPECT_NE(vw, nullptr);
}

TEST(Baseline, CheckEnabledOption)
{
  auto vw = VW::initialize(vwtest::make_args("--baseline", "--check_enabled", "--quiet"));
  EXPECT_NE(vw, nullptr);
}

TEST(Baseline, LrMultiplierOption)
{
  auto vw = VW::initialize(vwtest::make_args("--baseline", "--lr_multiplier", "2.0", "--quiet"));
  EXPECT_NE(vw, nullptr);
}

TEST(Baseline, BasicLearnAndPredict)
{
  auto vw = VW::initialize(vwtest::make_args("--baseline", "--quiet"));

  auto* ex1 = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex1);
  VW::finish_example(*vw, *ex1);

  auto* ex2 = VW::read_example(*vw, "-1 | d e f");
  vw->learn(*ex2);
  VW::finish_example(*vw, *ex2);

  auto* ex_pred = VW::read_example(*vw, "| a b c");
  vw->predict(*ex_pred);
  EXPECT_FALSE(std::isnan(ex_pred->pred.scalar));
  VW::finish_example(*vw, *ex_pred);
}

TEST(Baseline, GlobalOnlyLearnAndPredict)
{
  auto vw = VW::initialize(vwtest::make_args("--baseline", "--global_only", "--quiet"));

  auto* ex1 = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex1);
  VW::finish_example(*vw, *ex1);

  auto* ex2 = VW::read_example(*vw, "-1 | d e f");
  vw->learn(*ex2);
  VW::finish_example(*vw, *ex2);

  auto* ex_pred = VW::read_example(*vw, "| a b c");
  vw->predict(*ex_pred);
  EXPECT_FALSE(std::isnan(ex_pred->pred.scalar));
  VW::finish_example(*vw, *ex_pred);
}

TEST(Baseline, SetBaselineEnabled)
{
  VW::example ec;
  ec.indices.clear();

  // Initially not enabled
  EXPECT_FALSE(VW::reductions::baseline::baseline_enabled(&ec));

  // Enable baseline
  VW::reductions::baseline::set_baseline_enabled(&ec);
  EXPECT_TRUE(VW::reductions::baseline::baseline_enabled(&ec));

  // Setting again should not duplicate
  VW::reductions::baseline::set_baseline_enabled(&ec);
  EXPECT_TRUE(VW::reductions::baseline::baseline_enabled(&ec));
}

TEST(Baseline, ResetBaselineDisabled)
{
  VW::example ec;
  ec.indices.clear();

  // Enable and then disable
  VW::reductions::baseline::set_baseline_enabled(&ec);
  EXPECT_TRUE(VW::reductions::baseline::baseline_enabled(&ec));

  VW::reductions::baseline::reset_baseline_disabled(&ec);
  EXPECT_FALSE(VW::reductions::baseline::baseline_enabled(&ec));

  // Disabling again should be fine (no-op)
  VW::reductions::baseline::reset_baseline_disabled(&ec);
  EXPECT_FALSE(VW::reductions::baseline::baseline_enabled(&ec));
}

TEST(Baseline, CheckEnabledFlagBehavior)
{
  auto vw = VW::initialize(vwtest::make_args("--baseline", "--check_enabled", "--quiet"));

  // Without the enabled flag, baseline should be bypassed
  auto* ex1 = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex1);
  VW::finish_example(*vw, *ex1);

  auto* ex_pred = VW::read_example(*vw, "| a b c");
  vw->predict(*ex_pred);
  float pred_without_flag = ex_pred->pred.scalar;
  VW::finish_example(*vw, *ex_pred);

  EXPECT_FALSE(std::isnan(pred_without_flag));
}

TEST(Baseline, LrMultiplierBounds)
{
  // Test that lr_multiplier works with various values
  auto vw_low = VW::initialize(vwtest::make_args("--baseline", "--lr_multiplier", "0.0001", "--quiet"));
  EXPECT_NE(vw_low, nullptr);

  auto vw_high = VW::initialize(vwtest::make_args("--baseline", "--lr_multiplier", "1000", "--quiet"));
  EXPECT_NE(vw_high, nullptr);
}

TEST(Baseline, LogisticLossNoLrScaling)
{
  // With logistic loss, lr_scaling should be disabled
  auto vw = VW::initialize(vwtest::make_args("--baseline", "--loss_function", "logistic", "--quiet"));

  auto* ex1 = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex1);
  VW::finish_example(*vw, *ex1);

  auto* ex_pred = VW::read_example(*vw, "| a b c");
  vw->predict(*ex_pred);
  EXPECT_FALSE(std::isnan(ex_pred->pred.scalar));
  VW::finish_example(*vw, *ex_pred);
}

TEST(Baseline, SquaredLossWithLrScaling)
{
  // Default squared loss should have lr_scaling enabled
  auto vw = VW::initialize(vwtest::make_args("--baseline", "--loss_function", "squared", "--quiet"));

  auto* ex1 = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex1);
  VW::finish_example(*vw, *ex1);

  auto* ex2 = VW::read_example(*vw, "10 | d e f");
  vw->learn(*ex2);
  VW::finish_example(*vw, *ex2);

  auto* ex_pred = VW::read_example(*vw, "| a b c");
  vw->predict(*ex_pred);
  EXPECT_FALSE(std::isnan(ex_pred->pred.scalar));
  VW::finish_example(*vw, *ex_pred);
}

TEST(Baseline, SaveLoadModel)
{
  float pred_before_save;
  auto backing_vector = std::make_shared<std::vector<char>>();

  // Train and save
  {
    auto vw = VW::initialize(vwtest::make_args("--baseline", "--quiet"));

    auto* ex1 = VW::read_example(*vw, "1 | a b c");
    vw->learn(*ex1);
    VW::finish_example(*vw, *ex1);

    auto* ex2 = VW::read_example(*vw, "-1 | d e f");
    vw->learn(*ex2);
    VW::finish_example(*vw, *ex2);

    auto* ex_pred = VW::read_example(*vw, "| a b c");
    vw->predict(*ex_pred);
    pred_before_save = ex_pred->pred.scalar;
    VW::finish_example(*vw, *ex_pred);

    // Save model to buffer
    VW::io_buf io_writer;
    io_writer.add_file(VW::io::create_vector_writer(backing_vector));
    VW::save_predictor(*vw, io_writer);
    io_writer.flush();
  }

  // Load and predict
  {
    auto vw = VW::initialize(vwtest::make_args("-t", "--quiet"),
        VW::io::create_buffer_view(backing_vector->data(), backing_vector->size()));

    auto* ex = VW::read_example(*vw, "| a b c");
    vw->predict(*ex);
    EXPECT_NEAR(ex->pred.scalar, pred_before_save, 0.01f);
    VW::finish_example(*vw, *ex);
  }
}

TEST(Baseline, GlobalOnlyWithCheckEnabled)
{
  auto vw = VW::initialize(vwtest::make_args("--baseline", "--global_only", "--check_enabled", "--quiet"));

  auto* ex1 = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex1);
  VW::finish_example(*vw, *ex1);

  auto* ex_pred = VW::read_example(*vw, "| a b c");
  vw->predict(*ex_pred);
  EXPECT_FALSE(std::isnan(ex_pred->pred.scalar));
  VW::finish_example(*vw, *ex_pred);
}

TEST(Baseline, MultipleExamplesLearning)
{
  auto vw = VW::initialize(vwtest::make_args("--baseline", "--quiet"));

  // Train on multiple examples
  for (int i = 0; i < 10; i++)
  {
    auto label = (i % 2 == 0) ? "1" : "-1";
    auto features = (i % 2 == 0) ? "a b c" : "d e f";
    std::string example_str = std::string(label) + " | " + features;
    auto* ex = VW::read_example(*vw, example_str);
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }

  auto* ex_pred = VW::read_example(*vw, "| a b c");
  vw->predict(*ex_pred);
  EXPECT_FALSE(std::isnan(ex_pred->pred.scalar));
  VW::finish_example(*vw, *ex_pred);
}

TEST(Baseline, WithWeightedExamples)
{
  auto vw = VW::initialize(vwtest::make_args("--baseline", "--quiet"));

  // Train with weighted examples
  auto* ex1 = VW::read_example(*vw, "1 2.0 | a b c");  // weight = 2.0
  vw->learn(*ex1);
  VW::finish_example(*vw, *ex1);

  auto* ex2 = VW::read_example(*vw, "-1 0.5 | d e f");  // weight = 0.5
  vw->learn(*ex2);
  VW::finish_example(*vw, *ex2);

  auto* ex_pred = VW::read_example(*vw, "| a b c");
  vw->predict(*ex_pred);
  EXPECT_FALSE(std::isnan(ex_pred->pred.scalar));
  VW::finish_example(*vw, *ex_pred);
}
