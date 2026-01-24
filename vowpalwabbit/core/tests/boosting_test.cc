// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/vw.h"
#include "vw/test_common/test_common.h"

#include <gtest/gtest.h>

// Test Online BBM (Boost-by-Majority) algorithm - the default
TEST(Boosting, BBMLearnAndPredict)
{
  auto vw = VW::initialize(vwtest::make_args("--boosting", "3", "--quiet"));

  // Train with positive examples
  for (int i = 0; i < 20; ++i)
  {
    auto* ex = VW::read_example(*vw, "1 |f a b c");
    vw->learn(*ex);
    vw->finish_example(*ex);
  }

  // Train with negative examples
  for (int i = 0; i < 20; ++i)
  {
    auto* ex = VW::read_example(*vw, "-1 |f d e f");
    vw->learn(*ex);
    vw->finish_example(*ex);
  }

  // Predict on positive-like example
  {
    auto* ex = VW::read_example(*vw, "|f a b c");
    vw->predict(*ex);
    EXPECT_GT(ex->pred.scalar, 0.0f) << "Should predict positive for positive-like features";
    vw->finish_example(*ex);
  }

  // Predict on negative-like example
  {
    auto* ex = VW::read_example(*vw, "|f d e f");
    vw->predict(*ex);
    EXPECT_LT(ex->pred.scalar, 0.0f) << "Should predict negative for negative-like features";
    vw->finish_example(*ex);
  }

  vw->finish();
}

// Test BBM with custom gamma parameter
TEST(Boosting, BBMWithGamma)
{
  auto vw = VW::initialize(vwtest::make_args("--boosting", "5", "--gamma", "0.2", "--quiet"));

  for (int i = 0; i < 30; ++i)
  {
    auto* ex = VW::read_example(*vw, "1 |f x y z");
    vw->learn(*ex);
    vw->finish_example(*ex);
  }

  auto* ex = VW::read_example(*vw, "|f x y z");
  vw->predict(*ex);
  EXPECT_GT(ex->pred.scalar, 0.0f);
  vw->finish_example(*ex);

  vw->finish();
}

// Test Logistic boosting (AdaBoost.OL.W)
TEST(Boosting, LogisticLearnAndPredict)
{
  auto vw = VW::initialize(vwtest::make_args("--boosting", "3", "--alg", "logistic", "--quiet"));

  // Train with positive examples
  for (int i = 0; i < 20; ++i)
  {
    auto* ex = VW::read_example(*vw, "1 |f a b c");
    vw->learn(*ex);
    vw->finish_example(*ex);
  }

  // Train with negative examples
  for (int i = 0; i < 20; ++i)
  {
    auto* ex = VW::read_example(*vw, "-1 |f d e f");
    vw->learn(*ex);
    vw->finish_example(*ex);
  }

  // Predict
  {
    auto* ex = VW::read_example(*vw, "|f a b c");
    vw->predict(*ex);
    EXPECT_GT(ex->pred.scalar, 0.0f) << "Should predict positive for positive-like features";
    vw->finish_example(*ex);
  }

  vw->finish();
}

// Test Adaptive boosting (AdaBoost.OL)
TEST(Boosting, AdaptiveLearnAndPredict)
{
  auto vw = VW::initialize(vwtest::make_args("--boosting", "3", "--alg", "adaptive", "--quiet", "--random_seed", "42"));

  // Train with positive examples
  for (int i = 0; i < 20; ++i)
  {
    auto* ex = VW::read_example(*vw, "1 |f a b c");
    vw->learn(*ex);
    vw->finish_example(*ex);
  }

  // Train with negative examples
  for (int i = 0; i < 20; ++i)
  {
    auto* ex = VW::read_example(*vw, "-1 |f d e f");
    vw->learn(*ex);
    vw->finish_example(*ex);
  }

  // Predict
  {
    auto* ex = VW::read_example(*vw, "|f a b c");
    vw->predict(*ex);
    // Adaptive boosting uses early stopping, prediction may vary
    EXPECT_NE(ex->pred.scalar, 0.0f) << "Model should have learned something";
    vw->finish_example(*ex);
  }

  vw->finish();
}

// Test model save/load for logistic boosting (which saves alpha values)
TEST(Boosting, LogisticSaveLoad)
{
  float prediction_before_save;

  auto model_buffer = std::make_shared<std::vector<char>>();

  // Train and save model
  {
    auto vw = VW::initialize(vwtest::make_args("--boosting", "3", "--alg", "logistic", "--quiet"));

    for (int i = 0; i < 20; ++i)
    {
      auto* ex = VW::read_example(*vw, "1 |f a b c");
      vw->learn(*ex);
      vw->finish_example(*ex);
    }

    // Get prediction before save
    auto* ex = VW::read_example(*vw, "|f a b c");
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

    auto* ex = VW::read_example(*vw, "|f a b c");
    vw->predict(*ex);
    EXPECT_FLOAT_EQ(ex->pred.scalar, prediction_before_save);
    vw->finish_example(*ex);

    vw->finish();
  }
}

// Test model save/load for adaptive boosting (which saves alpha and v values)
TEST(Boosting, AdaptiveSaveLoad)
{
  auto model_buffer = std::make_shared<std::vector<char>>();

  // Train and save model
  {
    auto vw =
        VW::initialize(vwtest::make_args("--boosting", "3", "--alg", "adaptive", "--quiet", "--random_seed", "42"));

    for (int i = 0; i < 20; ++i)
    {
      auto* ex = VW::read_example(*vw, "1 |f a b c");
      vw->learn(*ex);
      vw->finish_example(*ex);
    }

    // Save model
    VW::io_buf model_io;
    model_io.add_file(VW::io::create_vector_writer(model_buffer));
    VW::save_predictor(*vw, model_io);
    model_io.flush();

    vw->finish();
  }

  // Load model and verify it can make predictions
  {
    auto model_reader = VW::io::create_buffer_view(model_buffer->data(), model_buffer->size());
    // Don't pass --random_seed here as it's saved in the model
    auto vw = VW::initialize(vwtest::make_args("--quiet"), std::move(model_reader));

    auto* ex = VW::read_example(*vw, "|f a b c");
    vw->predict(*ex);
    // Model should have learned something positive for this example
    EXPECT_NE(ex->pred.scalar, 0.0f);
    vw->finish_example(*ex);

    vw->finish();
  }
}

// Test multiple weak learners
TEST(Boosting, ManyWeakLearners)
{
  auto vw = VW::initialize(vwtest::make_args("--boosting", "10", "--quiet"));

  for (int i = 0; i < 50; ++i)
  {
    auto* ex = VW::read_example(*vw, "1 |f a b");
    vw->learn(*ex);
    vw->finish_example(*ex);

    ex = VW::read_example(*vw, "-1 |f c d");
    vw->learn(*ex);
    vw->finish_example(*ex);
  }

  auto* ex = VW::read_example(*vw, "|f a b");
  vw->predict(*ex);
  EXPECT_GT(ex->pred.scalar, 0.0f);
  vw->finish_example(*ex);

  vw->finish();
}
