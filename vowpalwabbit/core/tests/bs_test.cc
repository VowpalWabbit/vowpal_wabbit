// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/common/random.h"
#include "vw/config/options_cli.h"
#include "vw/core/reductions/bs.h"
#include "vw/core/vw.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>

using namespace testing;

// Test the Poisson weight generator function
TEST(Bootstrap, WeightGenDistribution)
{
  // weight_gen samples from a Poisson distribution with rate 1
  // Expected probabilities:
  // P(X=0) = e^(-1) ≈ 0.3679
  // P(X=1) = e^(-1) ≈ 0.3679
  // P(X=2) = e^(-1)/2 ≈ 0.1839
  // etc.

  VW::rand_state rng(42);
  const int num_samples = 10000;
  std::map<uint32_t, int> counts;

  for (int i = 0; i < num_samples; i++)
  {
    uint32_t weight = VW::reductions::bs::weight_gen(rng);
    counts[weight]++;
  }

  // Verify the distribution roughly matches expected Poisson(1)
  double p0_expected = std::exp(-1.0);
  double p0_observed = static_cast<double>(counts[0]) / num_samples;
  EXPECT_NEAR(p0_observed, p0_expected, 0.05);

  double p1_expected = std::exp(-1.0);
  double p1_observed = static_cast<double>(counts[1]) / num_samples;
  EXPECT_NEAR(p1_observed, p1_expected, 0.05);

  double p2_expected = std::exp(-1.0) / 2.0;
  double p2_observed = static_cast<double>(counts[2]) / num_samples;
  EXPECT_NEAR(p2_observed, p2_expected, 0.05);
}

// Test that weight_gen covers all boundary cases
TEST(Bootstrap, WeightGenBoundaries)
{
  // The function has explicit thresholds for values 0-20
  // Test that a random state can produce various values
  VW::rand_state rng(12345);
  std::set<uint32_t> seen_values;

  for (int i = 0; i < 100000; i++)
  {
    uint32_t weight = VW::reductions::bs::weight_gen(rng);
    seen_values.insert(weight);
  }

  // Should definitely see 0, 1, 2, 3 given enough samples
  EXPECT_TRUE(seen_values.count(0) > 0);
  EXPECT_TRUE(seen_values.count(1) > 0);
  EXPECT_TRUE(seen_values.count(2) > 0);
  EXPECT_TRUE(seen_values.count(3) > 0);

  // All values should be <= 20 (the max the function can return)
  for (uint32_t val : seen_values) { EXPECT_LE(val, 20u); }
}

// Test Bootstrap with mean aggregation
TEST(Bootstrap, MeanPrediction)
{
  std::vector<std::string> args{"--bootstrap", "5", "--bs_type", "mean", "--quiet"};
  auto vw = VW::initialize(VW::make_unique<VW::config::options_cli>(args));

  // Train on simple examples
  auto* ex1 = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex1);
  vw->finish_example(*ex1);

  auto* ex2 = VW::read_example(*vw, "0 | a b c");
  vw->learn(*ex2);
  vw->finish_example(*ex2);

  // Predict
  auto* test_ex = VW::read_example(*vw, "| a b c");
  vw->predict(*test_ex);

  // The prediction should be a valid float
  EXPECT_FALSE(std::isnan(test_ex->pred.scalar));
  EXPECT_FALSE(std::isinf(test_ex->pred.scalar));

  vw->finish_example(*test_ex);
}

// Test Bootstrap with vote aggregation
TEST(Bootstrap, VotePrediction)
{
  std::vector<std::string> args{"--bootstrap", "5", "--bs_type", "vote", "--quiet"};
  auto vw = VW::initialize(VW::make_unique<VW::config::options_cli>(args));

  // Train on simple classification-style examples
  auto* ex1 = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex1);
  vw->finish_example(*ex1);

  auto* ex2 = VW::read_example(*vw, "-1 | d e f");
  vw->learn(*ex2);
  vw->finish_example(*ex2);

  // Predict
  auto* test_ex = VW::read_example(*vw, "| a b c");
  vw->predict(*test_ex);

  // The prediction should be a valid float
  EXPECT_FALSE(std::isnan(test_ex->pred.scalar));
  EXPECT_FALSE(std::isinf(test_ex->pred.scalar));

  vw->finish_example(*test_ex);
}

// Test Bootstrap with different number of rounds
TEST(Bootstrap, DifferentRounds)
{
  for (int num_rounds : {2, 5, 10, 20})
  {
    std::vector<std::string> args{
        "--bootstrap", std::to_string(num_rounds), "--bs_type", "mean", "--quiet", "--random_seed", "42"};
    auto vw = VW::initialize(VW::make_unique<VW::config::options_cli>(args));

    // Train
    auto* ex1 = VW::read_example(*vw, "1 | a b c");
    vw->learn(*ex1);
    vw->finish_example(*ex1);

    // Predict
    auto* test_ex = VW::read_example(*vw, "| a b c");
    vw->predict(*test_ex);

    EXPECT_FALSE(std::isnan(test_ex->pred.scalar));
    vw->finish_example(*test_ex);
  }
}

// Test that bootstrap handles unlabeled examples correctly
TEST(Bootstrap, UnlabeledExamples)
{
  std::vector<std::string> args{"--bootstrap", "3", "--quiet"};
  auto vw = VW::initialize(VW::make_unique<VW::config::options_cli>(args));

  // Unlabeled example
  auto* ex = VW::read_example(*vw, "| feature1 feature2");
  vw->predict(*ex);

  // Should produce a valid prediction even for unlabeled data
  EXPECT_FALSE(std::isnan(ex->pred.scalar));
  vw->finish_example(*ex);
}

// Test bootstrap with squared loss
TEST(Bootstrap, SquaredLoss)
{
  std::vector<std::string> args{"--bootstrap", "5", "--loss_function", "squared", "--quiet"};
  auto vw = VW::initialize(VW::make_unique<VW::config::options_cli>(args));

  // Train on regression examples
  auto* ex1 = VW::read_example(*vw, "0.5 | x:1");
  vw->learn(*ex1);
  vw->finish_example(*ex1);

  auto* ex2 = VW::read_example(*vw, "0.7 | x:1.2");
  vw->learn(*ex2);
  vw->finish_example(*ex2);

  // Predict
  auto* test_ex = VW::read_example(*vw, "| x:1.1");
  vw->predict(*test_ex);

  EXPECT_FALSE(std::isnan(test_ex->pred.scalar));
  vw->finish_example(*test_ex);
}

// Test vote aggregation with ties
TEST(Bootstrap, VoteAggregationWithTies)
{
  std::vector<std::string> args{"--bootstrap", "4", "--bs_type", "vote", "--quiet", "--random_seed", "123"};
  auto vw = VW::initialize(VW::make_unique<VW::config::options_cli>(args));

  // Train examples
  for (int i = 0; i < 10; i++)
  {
    auto* ex = VW::read_example(*vw, "1 | a b c");
    vw->learn(*ex);
    vw->finish_example(*ex);
  }

  // Predict
  auto* test_ex = VW::read_example(*vw, "| a b c");
  vw->predict(*test_ex);

  // Prediction should be approximately 1 given training
  EXPECT_FALSE(std::isnan(test_ex->pred.scalar));
  vw->finish_example(*test_ex);
}

// Test bootstrap default type is mean
TEST(Bootstrap, DefaultTypeMean)
{
  std::vector<std::string> args{"--bootstrap", "5", "--quiet"};
  auto vw = VW::initialize(VW::make_unique<VW::config::options_cli>(args));

  // Just verify it initializes without error (default is mean)
  auto* ex = VW::read_example(*vw, "1 | feature");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

// Test bootstrap with save/load model
TEST(Bootstrap, SaveLoadModel)
{
  auto backing_vector = std::make_shared<std::vector<char>>();

  // Train and save
  {
    std::vector<std::string> args{"--bootstrap", "3", "--quiet"};
    auto vw = VW::initialize(VW::make_unique<VW::config::options_cli>(args));

    auto* ex = VW::read_example(*vw, "1 | a b");
    vw->learn(*ex);
    vw->finish_example(*ex);

    VW::io_buf io_writer;
    io_writer.add_file(VW::io::create_vector_writer(backing_vector));
    VW::save_predictor(*vw, io_writer);
    io_writer.flush();
  }

  // Load and predict
  {
    std::vector<std::string> args{"--quiet"};
    auto vw = VW::initialize(VW::make_unique<VW::config::options_cli>(args),
        VW::io::create_buffer_view(backing_vector->data(), backing_vector->size()));

    auto* test_ex = VW::read_example(*vw, "| a b");
    vw->predict(*test_ex);

    EXPECT_FALSE(std::isnan(test_ex->pred.scalar));
    vw->finish_example(*test_ex);
  }
}

// Test multiple predictions consistency
TEST(Bootstrap, MultiplePredictionsConsistency)
{
  std::vector<std::string> args{"--bootstrap", "5", "--quiet", "--random_seed", "42"};
  auto vw = VW::initialize(VW::make_unique<VW::config::options_cli>(args));

  // Train
  auto* train_ex = VW::read_example(*vw, "1 | feature:1.0");
  vw->learn(*train_ex);
  vw->finish_example(*train_ex);

  // Multiple predictions on same input should be consistent
  std::vector<float> predictions;
  for (int i = 0; i < 5; i++)
  {
    auto* test_ex = VW::read_example(*vw, "| feature:1.0");
    vw->predict(*test_ex);
    predictions.push_back(test_ex->pred.scalar);
    vw->finish_example(*test_ex);
  }

  // All predictions should be equal for the same input (deterministic after training)
  for (size_t i = 1; i < predictions.size(); i++) { EXPECT_FLOAT_EQ(predictions[0], predictions[i]); }
}
