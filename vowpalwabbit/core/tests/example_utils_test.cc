// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

// Tests for utility functions in example.cc:
// swap_prediction, flatten_features, to_string for scalars,
// append/truncate example namespaces, clean_example, return_multiple_example

#include "vw/core/example.h"
#include "vw/core/global_data.h"
#include "vw/core/vw.h"
#include "vw/test_common/test_common.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cmath>
#include <sstream>

using namespace ::testing;

// --- swap_prediction ---

TEST(ExampleUtils, SwapPredictionScalar)
{
  VW::polyprediction a;
  VW::polyprediction b;
  a.scalar = 1.5f;
  b.scalar = 2.5f;

  VW::swap_prediction(a, b, VW::prediction_type_t::SCALAR);

  EXPECT_FLOAT_EQ(a.scalar, 2.5f);
  EXPECT_FLOAT_EQ(b.scalar, 1.5f);
}

TEST(ExampleUtils, SwapPredictionMulticlass)
{
  VW::polyprediction a;
  VW::polyprediction b;
  a.multiclass = 3;
  b.multiclass = 7;

  VW::swap_prediction(a, b, VW::prediction_type_t::MULTICLASS);

  EXPECT_EQ(a.multiclass, 7u);
  EXPECT_EQ(b.multiclass, 3u);
}

TEST(ExampleUtils, SwapPredictionScalars)
{
  VW::polyprediction a;
  VW::polyprediction b;
  a.scalars.push_back(1.0f);
  a.scalars.push_back(2.0f);
  b.scalars.push_back(3.0f);

  VW::swap_prediction(a, b, VW::prediction_type_t::SCALARS);

  EXPECT_EQ(a.scalars.size(), 1u);
  EXPECT_FLOAT_EQ(a.scalars[0], 3.0f);
  EXPECT_EQ(b.scalars.size(), 2u);
  EXPECT_FLOAT_EQ(b.scalars[0], 1.0f);
}

TEST(ExampleUtils, SwapPredictionActionScores)
{
  VW::polyprediction a;
  VW::polyprediction b;
  a.a_s.push_back({0, 0.5f});
  a.a_s.push_back({1, 0.5f});
  b.a_s.push_back({2, 1.0f});

  VW::swap_prediction(a, b, VW::prediction_type_t::ACTION_SCORES);

  EXPECT_EQ(a.a_s.size(), 1u);
  EXPECT_EQ(a.a_s[0].action, 2u);
  EXPECT_EQ(b.a_s.size(), 2u);
}

TEST(ExampleUtils, SwapPredictionActionProbs)
{
  VW::polyprediction a;
  VW::polyprediction b;
  a.a_s.push_back({0, 0.3f});
  b.a_s.push_back({1, 0.7f});

  VW::swap_prediction(a, b, VW::prediction_type_t::ACTION_PROBS);

  EXPECT_EQ(a.a_s.size(), 1u);
  EXPECT_FLOAT_EQ(a.a_s[0].score, 0.7f);
  EXPECT_EQ(b.a_s.size(), 1u);
  EXPECT_FLOAT_EQ(b.a_s[0].score, 0.3f);
}

TEST(ExampleUtils, SwapPredictionMultilabels)
{
  VW::polyprediction a;
  VW::polyprediction b;
  a.multilabels.label_v.push_back(1);
  a.multilabels.label_v.push_back(2);
  b.multilabels.label_v.push_back(3);

  VW::swap_prediction(a, b, VW::prediction_type_t::MULTILABELS);

  EXPECT_EQ(a.multilabels.label_v.size(), 1u);
  EXPECT_EQ(a.multilabels.label_v[0], 3u);
  EXPECT_EQ(b.multilabels.label_v.size(), 2u);
}

TEST(ExampleUtils, SwapPredictionProb)
{
  VW::polyprediction a;
  VW::polyprediction b;
  a.prob = 0.2f;
  b.prob = 0.8f;

  VW::swap_prediction(a, b, VW::prediction_type_t::PROB);

  EXPECT_FLOAT_EQ(a.prob, 0.8f);
  EXPECT_FLOAT_EQ(b.prob, 0.2f);
}

TEST(ExampleUtils, SwapPredictionMulticlassProbs)
{
  VW::polyprediction a;
  VW::polyprediction b;
  a.scalars.push_back(0.1f);
  a.scalars.push_back(0.9f);
  b.scalars.push_back(0.5f);
  b.scalars.push_back(0.5f);

  VW::swap_prediction(a, b, VW::prediction_type_t::MULTICLASS_PROBS);

  EXPECT_FLOAT_EQ(a.scalars[0], 0.5f);
  EXPECT_FLOAT_EQ(b.scalars[0], 0.1f);
}

TEST(ExampleUtils, SwapPredictionDecisionProbs)
{
  VW::polyprediction a;
  VW::polyprediction b;
  VW::action_scores as1;
  as1.push_back({0, 1.0f});
  VW::action_scores as2;
  as2.push_back({1, 0.5f});
  as2.push_back({2, 0.5f});
  a.decision_scores.push_back(as1);
  b.decision_scores.push_back(as2);

  VW::swap_prediction(a, b, VW::prediction_type_t::DECISION_PROBS);

  EXPECT_EQ(a.decision_scores.size(), 1u);
  EXPECT_EQ(a.decision_scores[0].size(), 2u);
  EXPECT_EQ(b.decision_scores.size(), 1u);
  EXPECT_EQ(b.decision_scores[0].size(), 1u);

  // Clean up to avoid leaks from decision_scores
  for (auto& ds : a.decision_scores) { ds.clear(); }
  a.decision_scores.clear();
  for (auto& ds : b.decision_scores) { ds.clear(); }
  b.decision_scores.clear();
}

TEST(ExampleUtils, SwapPredictionNoPred)
{
  VW::polyprediction a;
  VW::polyprediction b;
  // NOPRED should be a no-op, just verify it doesn't crash
  VW::swap_prediction(a, b, VW::prediction_type_t::NOPRED);
}

// --- to_string for scalars ---

TEST(ExampleUtils, ToStringScalars)
{
  VW::v_array<float> scalars;
  scalars.push_back(1.5f);
  scalars.push_back(2.3f);
  scalars.push_back(0.1f);

  std::string result = VW::to_string(scalars, 6);
  EXPECT_THAT(result, HasSubstr("1.5"));
  EXPECT_THAT(result, HasSubstr("2.3"));
  EXPECT_THAT(result, HasSubstr("0.1"));
  // Should be comma-separated
  EXPECT_THAT(result, HasSubstr(","));
}

TEST(ExampleUtils, ToStringScalarsEmpty)
{
  VW::v_array<float> scalars;
  std::string result = VW::to_string(scalars, 6);
  EXPECT_TRUE(result.empty());
}

TEST(ExampleUtils, ToStringSingleScalar)
{
  VW::v_array<float> scalars;
  scalars.push_back(3.14f);

  std::string result = VW::to_string(scalars, 6);
  EXPECT_THAT(result, HasSubstr("3.14"));
  // No comma for single element
  EXPECT_THAT(result, Not(HasSubstr(",")));
}

// --- flatten_features ---

TEST(ExampleUtils, FlattenFeatures)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* ex = VW::read_example(*vw, "1 |a x:1 y:2 |b z:3");

  VW::features flat_fs;
  VW::flatten_features(*vw, *ex, flat_fs);

  // Should have at least x, y, z plus constant
  EXPECT_GE(flat_fs.size(), 3u);
  // Sum of feature squared should be set
  EXPECT_GT(flat_fs.sum_feat_sq, 0.0f);

  VW::finish_example(*vw, *ex);
}

// --- append_example_namespace / truncate_example_namespace ---

TEST(ExampleUtils, AppendAndTruncateExampleNamespace)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* ex = VW::read_example(*vw, "1 |a x:1 y:2");

  // Create features to append
  VW::features new_fs;
  new_fs.push_back(3.0f, 100);
  new_fs.push_back(4.0f, 200);

  size_t features_before = ex->num_features;

  // Append to namespace 'b'
  VW::details::append_example_namespace(*ex, 'b', new_fs);
  EXPECT_EQ(ex->num_features, features_before + 2);

  // Namespace 'b' should now be in indices
  bool found = false;
  for (auto idx : ex->indices)
  {
    if (idx == 'b') { found = true; }
  }
  EXPECT_TRUE(found);

  // Truncate back
  VW::details::truncate_example_namespace(*ex, 'b', new_fs);
  EXPECT_EQ(ex->num_features, features_before);

  VW::finish_example(*vw, *ex);
}

TEST(ExampleUtils, AppendNamespacesFromExample)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* target = VW::read_example(*vw, "1 |a x:1");
  auto* source = VW::read_example(*vw, "|b y:2 z:3");

  size_t target_features_before = target->num_features;

  VW::details::append_example_namespaces_from_example(*target, *source);
  EXPECT_GT(target->num_features, target_features_before);

  VW::details::truncate_example_namespaces_from_example(*target, *source);
  EXPECT_EQ(target->num_features, target_features_before);

  VW::finish_example(*vw, *target);
  VW::finish_example(*vw, *source);
}

TEST(ExampleUtils, TruncateNamespacesFromEmptySource)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* target = VW::read_example(*vw, "1 |a x:1");
  VW::example empty_source;

  // Should be a no-op since source is empty
  size_t features_before = target->num_features;
  VW::details::truncate_example_namespaces_from_example(*target, empty_source);
  EXPECT_EQ(target->num_features, features_before);

  VW::finish_example(*vw, *target);
}

// --- return_multiple_example ---

TEST(ExampleUtils, ReturnMultipleExample)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  VW::multi_ex examples;
  examples.push_back(&VW::get_unused_example(&(*vw)));
  examples.push_back(&VW::get_unused_example(&(*vw)));

  EXPECT_EQ(examples.size(), 2u);
  VW::return_multiple_example(*vw, examples);
  EXPECT_EQ(examples.size(), 0u);
}

// --- get_total_sum_feat_sq ---

TEST(ExampleUtils, GetTotalSumFeatSq)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a:1 b:2 c:3");

  float sum_sq = ex->get_total_sum_feat_sq();
  // Should be at least 1^2 + 2^2 + 3^2 = 14 (without constant and interactions)
  EXPECT_GT(sum_sq, 0.0f);

  // Second call should use cached value
  float sum_sq2 = ex->get_total_sum_feat_sq();
  EXPECT_FLOAT_EQ(sum_sq, sum_sq2);

  VW::finish_example(*vw, *ex);
}
