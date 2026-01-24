// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/config/options_cli.h"
#include "vw/core/multi_ex.h"
#include "vw/core/vw.h"
#include "vw/io/io_adapter.h"
#include "vw/io/logger.h"
#include "vw/test_common/test_common.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <sstream>
#include <vector>

// Tests for VW::initialize overloads

TEST(Initialize, BasicInitialization)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  ASSERT_NE(vw, nullptr);
  EXPECT_NE(vw->l, nullptr);
}

TEST(Initialize, WithOptionsObject)
{
  auto options = VW::make_unique<VW::config::options_cli>(std::vector<std::string>{"--quiet", "--cb_explore_adf"});
  auto vw = VW::initialize(std::move(options));
  ASSERT_NE(vw, nullptr);
  EXPECT_NE(vw->l, nullptr);
}

TEST(Initialize, WithVectorOfArgs)
{
  std::vector<std::string> args = {"--quiet", "--cb_explore_adf", "--epsilon", "0.1"};
  auto options = VW::make_unique<VW::config::options_cli>(args);
  auto vw = VW::initialize(std::move(options));
  ASSERT_NE(vw, nullptr);
  EXPECT_NE(vw->l, nullptr);
}

// Test VW::initialize with driver output function
TEST(Initialize, WithDriverOutputFunc)
{
  std::vector<std::string> captured_messages;
  auto driver_output = [](void* context, const std::string& msg)
  {
    auto* messages = static_cast<std::vector<std::string>*>(context);
    messages->push_back(msg);
  };

  auto options = VW::make_unique<VW::config::options_cli>(std::vector<std::string>{"--cb_explore_adf"});
  auto vw = VW::initialize(std::move(options), nullptr, driver_output, &captured_messages);
  ASSERT_NE(vw, nullptr);

  // The driver output should have captured some messages during initialization
  EXPECT_FALSE(captured_messages.empty());
}

// Test VW::initialize with model override reader
TEST(Initialize, WithModelOverrideReader)
{
  std::vector<char> model_buffer;

  // First, train and save a model
  {
    auto vw = VW::initialize(vwtest::make_args("--quiet"));

    auto* ex = VW::read_example(*vw, "1 |f a b c");
    vw->learn(*ex);
    vw->finish_example(*ex);

    // Save model to buffer
    auto backing_vector = std::make_shared<std::vector<char>>();
    VW::io_buf io_writer;
    io_writer.add_file(VW::io::create_vector_writer(backing_vector));
    VW::save_predictor(*vw, io_writer);
    io_writer.flush();
    model_buffer = *backing_vector;
  }

  // Load with model override reader
  {
    auto model_reader = VW::io::create_buffer_view(model_buffer.data(), model_buffer.size());
    auto options = VW::make_unique<VW::config::options_cli>(std::vector<std::string>{"--quiet"});
    auto vw = VW::initialize(std::move(options), std::move(model_reader));
    ASSERT_NE(vw, nullptr);

    // Verify model loaded correctly by making a prediction
    auto* ex = VW::read_example(*vw, "|f a b c");
    vw->predict(*ex);
    // Model should have learned something, prediction should be non-zero
    EXPECT_NE(ex->pred.scalar, 0.0f);
    vw->finish_example(*ex);
  }
}

// Test VW::initialize_experimental
TEST(InitializeExperimental, BasicUsage)
{
  auto options = VW::make_unique<VW::config::options_cli>(std::vector<std::string>{"--quiet", "--cb_explore_adf"});
  auto vw = VW::initialize_experimental(std::move(options));
  ASSERT_NE(vw, nullptr);
  EXPECT_NE(vw->l, nullptr);
}

TEST(InitializeExperimental, WithAllParameters)
{
  std::vector<std::string> captured_messages;
  auto driver_output = [](void* context, const std::string& msg)
  {
    auto* messages = static_cast<std::vector<std::string>*>(context);
    messages->push_back(msg);
  };

  auto options = VW::make_unique<VW::config::options_cli>(std::vector<std::string>{"--cb_explore_adf"});
  auto vw = VW::initialize_experimental(
      std::move(options), nullptr /* model reader */, driver_output, &captured_messages, nullptr /* custom logger */);
  ASSERT_NE(vw, nullptr);
  EXPECT_FALSE(captured_messages.empty());
}

// Test VW::seed_vw_model
TEST(SeedVwModel, BasicSeeding)
{
  // Create original model
  auto vw_original = VW::initialize(vwtest::make_args("--quiet"));

  // Train on some examples
  auto* ex = VW::read_example(*vw_original, "1 |f a b c");
  vw_original->learn(*ex);
  vw_original->finish_example(*ex);

  ex = VW::read_example(*vw_original, "-1 |f d e f");
  vw_original->learn(*ex);
  vw_original->finish_example(*ex);

  // Seed a new model
  auto vw_seeded = VW::seed_vw_model(*vw_original, std::vector<std::string>{});
  ASSERT_NE(vw_seeded, nullptr);

  // Predictions should be similar since weights are shared
  auto* ex1 = VW::read_example(*vw_original, "|f a b c");
  vw_original->predict(*ex1);
  float original_pred = ex1->pred.scalar;
  vw_original->finish_example(*ex1);

  auto* ex2 = VW::read_example(*vw_seeded, "|f a b c");
  vw_seeded->predict(*ex2);
  float seeded_pred = ex2->pred.scalar;
  vw_seeded->finish_example(*ex2);

  EXPECT_FLOAT_EQ(original_pred, seeded_pred);
}

TEST(SeedVwModel, WithExtraArgs)
{
  auto vw_original = VW::initialize(vwtest::make_args("--quiet"));

  auto* ex = VW::read_example(*vw_original, "1 |f a b c");
  vw_original->learn(*ex);
  vw_original->finish_example(*ex);

  // Seed with extra arguments (e.g., testonly mode)
  auto vw_seeded = VW::seed_vw_model(*vw_original, std::vector<std::string>{"-t"});
  ASSERT_NE(vw_seeded, nullptr);

  // Seeded model should be in test mode
  auto* ex2 = VW::read_example(*vw_seeded, "1 |f a b c");
  vw_seeded->learn(*ex2);  // Should not update weights in test mode
  vw_seeded->finish_example(*ex2);
}

TEST(SeedVwModel, SharedDataIsShared)
{
  auto vw_original = VW::initialize(vwtest::make_args("--quiet"));

  auto* ex = VW::read_example(*vw_original, "1 |f a b c");
  vw_original->learn(*ex);
  vw_original->finish_example(*ex);

  auto vw_seeded = VW::seed_vw_model(*vw_original, std::vector<std::string>{});

  // shared_data should be the same object
  EXPECT_EQ(vw_original->sd.get(), vw_seeded->sd.get());
}

// Test that initialize handles different reduction stacks
TEST(Initialize, WithCbExploreAdf)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--quiet"));
  ASSERT_NE(vw, nullptr);

  // cb_explore_adf requires multi-line examples
  VW::multi_ex examples;
  examples.push_back(VW::read_example(*vw, "shared |s user"));
  examples.push_back(VW::read_example(*vw, "0:1:0.5 |a action1"));
  examples.push_back(VW::read_example(*vw, "|a action2"));
  examples.push_back(VW::read_example(*vw, ""));

  vw->predict(examples);
  vw->finish_example(examples);
}

TEST(Initialize, WithCsoaaLdf)
{
  auto vw = VW::initialize(vwtest::make_args("--csoaa_ldf", "m", "--quiet"));
  ASSERT_NE(vw, nullptr);
}

TEST(Initialize, WithMulticlass)
{
  auto vw = VW::initialize(vwtest::make_args("--oaa", "3", "--quiet"));
  ASSERT_NE(vw, nullptr);

  auto* ex = VW::read_example(*vw, "1 |f a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

// Test custom logger
TEST(Initialize, WithCustomLogger)
{
  std::vector<std::string> log_messages;
  auto logger_func = [](void* context, VW::io::log_level, const std::string& msg)
  {
    auto* messages = static_cast<std::vector<std::string>*>(context);
    messages->push_back(msg);
  };
  VW::io::logger custom_logger = VW::io::create_custom_sink_logger(&log_messages, logger_func);

  auto options = VW::make_unique<VW::config::options_cli>(std::vector<std::string>{"--cb_explore_adf"});
  auto vw = VW::initialize(std::move(options), nullptr, nullptr, nullptr, &custom_logger);
  ASSERT_NE(vw, nullptr);
}

// Test error handling
TEST(Initialize, InvalidOptionThrows)
{
  auto options = VW::make_unique<VW::config::options_cli>(std::vector<std::string>{"--invalid_option_xyz"});
  EXPECT_THROW(VW::initialize(std::move(options)), VW::vw_exception);
}

TEST(Initialize, ConflictingOptionsThrows)
{
  // cb and cb_explore_adf are conflicting
  auto options = VW::make_unique<VW::config::options_cli>(std::vector<std::string>{"--cb", "2", "--oaa", "3", "--quiet"});
  EXPECT_THROW(VW::initialize(std::move(options)), VW::vw_exception);
}
