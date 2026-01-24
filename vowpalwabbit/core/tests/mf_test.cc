// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/vw.h"
#include "vw/test_common/test_common.h"

#include <gtest/gtest.h>

// Test basic matrix factorization with rank 1
TEST(MatrixFactorization, BasicLearnAndPredict)
{
  // new_mf requires -q (pairs) to be specified
  auto vw = VW::initialize(vwtest::make_args("--new_mf", "1", "-q", "ab", "--quiet"));

  // Train with examples that have features in both namespaces
  for (int i = 0; i < 20; ++i)
  {
    auto* ex = VW::read_example(*vw, "1 |a x1 x2 |b y1 y2");
    vw->learn(*ex);
    vw->finish_example(*ex);
  }

  for (int i = 0; i < 20; ++i)
  {
    auto* ex = VW::read_example(*vw, "-1 |a x3 x4 |b y3 y4");
    vw->learn(*ex);
    vw->finish_example(*ex);
  }

  // Predict on similar examples
  {
    auto* ex = VW::read_example(*vw, "|a x1 x2 |b y1 y2");
    vw->predict(*ex);
    EXPECT_GT(ex->pred.scalar, 0.0f) << "Should predict positive for positive-like features";
    vw->finish_example(*ex);
  }

  {
    auto* ex = VW::read_example(*vw, "|a x3 x4 |b y3 y4");
    vw->predict(*ex);
    EXPECT_LT(ex->pred.scalar, 0.0f) << "Should predict negative for negative-like features";
    vw->finish_example(*ex);
  }

  vw->finish();
}

// Test matrix factorization with higher rank
TEST(MatrixFactorization, HigherRank)
{
  auto vw = VW::initialize(vwtest::make_args("--new_mf", "3", "-q", "ab", "--quiet"));

  for (int i = 0; i < 30; ++i)
  {
    auto* ex = VW::read_example(*vw, "1 |a feature1 feature2 |b feat1 feat2");
    vw->learn(*ex);
    vw->finish_example(*ex);
  }

  auto* ex = VW::read_example(*vw, "|a feature1 feature2 |b feat1 feat2");
  vw->predict(*ex);
  EXPECT_GT(ex->pred.scalar, 0.0f);
  vw->finish_example(*ex);

  vw->finish();
}

// Test matrix factorization prediction without learning (predict-only)
TEST(MatrixFactorization, PredictOnly)
{
  auto vw = VW::initialize(vwtest::make_args("--new_mf", "2", "-q", "ab", "--quiet"));

  // Just predict without training - should give some prediction
  auto* ex = VW::read_example(*vw, "|a x y z |b p q r");
  vw->predict(*ex);
  // With random positive weights, prediction will be non-zero
  // (new_mf sets random_positive_weights = true)
  vw->finish_example(*ex);

  vw->finish();
}

// Test with empty namespace (edge case)
TEST(MatrixFactorization, EmptyNamespace)
{
  auto vw = VW::initialize(vwtest::make_args("--new_mf", "1", "-q", "ab", "--quiet"));

  // Example with features only in one namespace
  auto* ex = VW::read_example(*vw, "1 |a x y z");
  vw->learn(*ex);
  vw->finish_example(*ex);

  // Prediction should still work
  ex = VW::read_example(*vw, "|a x y z");
  vw->predict(*ex);
  vw->finish_example(*ex);

  vw->finish();
}

// Test model save/load
TEST(MatrixFactorization, SaveLoad)
{
  float prediction_before_save;

  auto model_buffer = std::make_shared<std::vector<char>>();

  // Train and save model
  {
    auto vw = VW::initialize(vwtest::make_args("--new_mf", "2", "-q", "ab", "--quiet"));

    for (int i = 0; i < 30; ++i)
    {
      auto* ex = VW::read_example(*vw, "1 |a x1 x2 |b y1 y2");
      vw->learn(*ex);
      vw->finish_example(*ex);
    }

    // Get prediction before save
    auto* ex = VW::read_example(*vw, "|a x1 x2 |b y1 y2");
    vw->predict(*ex);
    prediction_before_save = ex->pred.scalar;
    vw->finish_example(*ex);

    // Save model
    VW::io_buf model_io;
    model_io.add_file(VW::io::create_vector_writer(model_buffer));
    VW::save_predictor(*vw, model_io);
    model_io.flush();

    vw->finish();
  }

  // Load model and verify prediction
  {
    auto model_reader = VW::io::create_buffer_view(model_buffer->data(), model_buffer->size());
    auto vw = VW::initialize(vwtest::make_args("--quiet"), std::move(model_reader));

    auto* ex = VW::read_example(*vw, "|a x1 x2 |b y1 y2");
    vw->predict(*ex);
    EXPECT_FLOAT_EQ(ex->pred.scalar, prediction_before_save);
    vw->finish_example(*ex);

    vw->finish();
  }
}

// Test with multiple pairs
TEST(MatrixFactorization, MultiplePairs)
{
  auto vw = VW::initialize(vwtest::make_args("--new_mf", "1", "-q", "ab", "-q", "cd", "--quiet"));

  for (int i = 0; i < 20; ++i)
  {
    auto* ex = VW::read_example(*vw, "1 |a a1 |b b1 |c c1 |d d1");
    vw->learn(*ex);
    vw->finish_example(*ex);
  }

  auto* ex = VW::read_example(*vw, "|a a1 |b b1 |c c1 |d d1");
  vw->predict(*ex);
  EXPECT_GT(ex->pred.scalar, 0.0f);
  vw->finish_example(*ex);

  vw->finish();
}
