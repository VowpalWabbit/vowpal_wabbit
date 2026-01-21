// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/vw.h"
#include "vw/test_common/test_common.h"

#include <gtest/gtest.h>

// Test that dropout is disabled during inference (--testonly mode)
// This ensures deterministic predictions when using a trained model
// Issue #4067: https://github.com/VowpalWabbit/vowpal_wabbit/issues/4067
TEST(Nn, DropoutDisabledDuringInference)
{
  // Train a model with dropout
  auto vw_train = VW::initialize(vwtest::make_args("--nn", "2", "--dropout", "--quiet", "--random_seed", "42"));

  for (int i = 0; i < 10; ++i)
  {
    auto* ex = VW::read_example(*vw_train, "1 | a b c");
    vw_train->learn(*ex);
    vw_train->finish_example(*ex);

    ex = VW::read_example(*vw_train, "-1 | d e f");
    vw_train->learn(*ex);
    vw_train->finish_example(*ex);
  }

  // Save model
  auto model_buffer = std::make_shared<std::vector<char>>();
  VW::io_buf model_io;
  model_io.add_file(VW::io::create_vector_writer(model_buffer));
  VW::save_predictor(*vw_train, model_io);
  model_io.flush();
  vw_train->finish();

  // Load model in test-only mode and verify predictions are deterministic
  auto model_reader = VW::io::create_buffer_view(model_buffer->data(), model_buffer->size());
  auto vw_test = VW::initialize(vwtest::make_args("--quiet", "--testonly"), std::move(model_reader));

  // Run predictions multiple times - they should be identical if dropout is disabled
  std::vector<float> predictions;
  for (int i = 0; i < 5; ++i)
  {
    auto* ex = VW::read_example(*vw_test, "| a b c");
    vw_test->predict(*ex);
    predictions.push_back(ex->pred.scalar);
    vw_test->finish_example(*ex);
  }

  // All predictions should be identical (no random dropout during inference)
  for (size_t i = 1; i < predictions.size(); ++i)
  {
    EXPECT_FLOAT_EQ(predictions[0], predictions[i])
        << "Prediction " << i << " differs from prediction 0, indicating dropout is still active during inference";
  }

  vw_test->finish();
}

// Test that dropout still works during training (predictions may vary)
TEST(Nn, DropoutActiveDuringTraining)
{
  auto vw = VW::initialize(vwtest::make_args("--nn", "2", "--dropout", "--quiet", "--random_seed", "42"));

  // Train with some examples first
  for (int i = 0; i < 5; ++i)
  {
    auto* ex = VW::read_example(*vw, "1 | a b c");
    vw->learn(*ex);
    vw->finish_example(*ex);
  }

  // During learn(), dropout should cause different internal states
  // We can't easily test this directly, but we can verify the model trains
  auto* ex = VW::read_example(*vw, "| a b c");
  vw->predict(*ex);
  float pred = ex->pred.scalar;
  vw->finish_example(*ex);

  // The prediction should be non-zero after training
  EXPECT_NE(pred, 0.0f) << "Model should have learned something";

  vw->finish();
}
