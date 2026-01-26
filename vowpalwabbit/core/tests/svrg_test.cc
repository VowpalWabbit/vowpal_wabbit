// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/svrg.h"
#include "vw/core/vw.h"
#include "vw/io/io_adapter.h"
#include "vw/test_common/test_common.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cmath>
#include <memory>
#include <vector>

TEST(SVRG, BasicInitialization)
{
  auto vw = VW::initialize(vwtest::make_args("--svrg", "--quiet"));
  EXPECT_NE(vw, nullptr);
}

TEST(SVRG, CustomStageSize)
{
  auto vw = VW::initialize(vwtest::make_args("--svrg", "--stage_size", "3", "--quiet"));
  EXPECT_NE(vw, nullptr);
}

TEST(SVRG, BasicLearnAndPredict)
{
  auto vw = VW::initialize(vwtest::make_args("--svrg", "--quiet"));

  // Train on a few examples
  auto* ex1 = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex1);
  VW::finish_example(*vw, *ex1);

  auto* ex2 = VW::read_example(*vw, "-1 | d e f");
  vw->learn(*ex2);
  VW::finish_example(*vw, *ex2);

  // Predict on a new example
  auto* ex3 = VW::read_example(*vw, "| a b c");
  vw->predict(*ex3);
  float prediction = ex3->pred.scalar;
  VW::finish_example(*vw, *ex3);

  EXPECT_FALSE(std::isnan(prediction));
}

TEST(SVRG, MultiPassLearning)
{
  auto vw = VW::initialize(vwtest::make_args("--svrg", "--stage_size", "1", "--passes", "3", "--quiet",
      "--holdout_off", "--cache_file", "/tmp/svrg_test.cache", "--kill_cache"));

  // First pass - gradient computation stage
  auto* ex1 = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex1);
  VW::finish_example(*vw, *ex1);

  auto* ex2 = VW::read_example(*vw, "-1 | d e f");
  vw->learn(*ex2);
  VW::finish_example(*vw, *ex2);

  // Predict and verify result is valid
  auto* ex_pred = VW::read_example(*vw, "| a b c");
  vw->predict(*ex_pred);
  EXPECT_FALSE(std::isnan(ex_pred->pred.scalar));
  VW::finish_example(*vw, *ex_pred);
}

TEST(SVRG, SaveLoadModel)
{
  float prediction_before_save;
  auto backing_vector = std::make_shared<std::vector<char>>();

  // Train and save
  {
    auto vw = VW::initialize(vwtest::make_args("--svrg", "--quiet"));

    auto* ex1 = VW::read_example(*vw, "1 | a b c");
    vw->learn(*ex1);
    VW::finish_example(*vw, *ex1);

    auto* ex2 = VW::read_example(*vw, "-1 | d e f");
    vw->learn(*ex2);
    VW::finish_example(*vw, *ex2);

    auto* ex_pred = VW::read_example(*vw, "| a b c");
    vw->predict(*ex_pred);
    prediction_before_save = ex_pred->pred.scalar;
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
    EXPECT_FALSE(std::isnan(ex->pred.scalar));
    EXPECT_NEAR(ex->pred.scalar, prediction_before_save, 0.01f);
    VW::finish_example(*vw, *ex);
  }
}

TEST(SVRG, SaveLoadModelResume)
{
  float prediction_before_save;
  auto backing_vector = std::make_shared<std::vector<char>>();

  // Train and save with resume
  {
    auto vw = VW::initialize(vwtest::make_args("--svrg", "--quiet", "--save_resume"));

    auto* ex1 = VW::read_example(*vw, "1 | a b c");
    vw->learn(*ex1);
    VW::finish_example(*vw, *ex1);

    auto* ex2 = VW::read_example(*vw, "-1 | d e f");
    vw->learn(*ex2);
    VW::finish_example(*vw, *ex2);

    auto* ex_pred = VW::read_example(*vw, "| a b c");
    vw->predict(*ex_pred);
    prediction_before_save = ex_pred->pred.scalar;
    VW::finish_example(*vw, *ex_pred);

    // Save model to buffer
    VW::io_buf io_writer;
    io_writer.add_file(VW::io::create_vector_writer(backing_vector));
    VW::save_predictor(*vw, io_writer);
    io_writer.flush();
  }

  // Load resumed model and verify consistent prediction
  {
    auto vw = VW::initialize(vwtest::make_args("-t", "--quiet"),
        VW::io::create_buffer_view(backing_vector->data(), backing_vector->size()));

    auto* ex = VW::read_example(*vw, "| a b c");
    vw->predict(*ex);
    EXPECT_NEAR(ex->pred.scalar, prediction_before_save, 0.01f);
    VW::finish_example(*vw, *ex);
  }
}

TEST(SVRG, LearnWithWeightedExamples)
{
  auto vw = VW::initialize(vwtest::make_args("--svrg", "--quiet"));

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

TEST(SVRG, PredictWithoutLearning)
{
  auto vw = VW::initialize(vwtest::make_args("--svrg", "--quiet"));

  // Predict without prior learning - should still produce valid output
  auto* ex = VW::read_example(*vw, "| a b c");
  vw->predict(*ex);
  EXPECT_FALSE(std::isnan(ex->pred.scalar));
  VW::finish_example(*vw, *ex);
}

TEST(SVRG, LargeStageSize)
{
  auto vw = VW::initialize(vwtest::make_args("--svrg", "--stage_size", "10", "--quiet"));

  // With stage_size=10, first 10 passes are gradient computation
  for (int i = 0; i < 5; i++)
  {
    auto* ex = VW::read_example(*vw, "1 | a b c");
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }

  auto* ex_pred = VW::read_example(*vw, "| a b c");
  vw->predict(*ex_pred);
  EXPECT_FALSE(std::isnan(ex_pred->pred.scalar));
  VW::finish_example(*vw, *ex_pred);
}
