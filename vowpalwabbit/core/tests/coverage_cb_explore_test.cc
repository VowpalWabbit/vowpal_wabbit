// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

// Batch 13: CB/Explore Reductions coverage tests targeting cbify.cc, cb_algs.cc,
// explore_internal.h, cb_explore_adf_synthcover.cc, oaa.cc, warm_cb.cc, cb_explore_adf_cover.cc

#include "vw/core/vw.h"
#include "vw/test_common/test_common.h"

#include <gtest/gtest.h>

#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

// ============================================================
// Helper: learn+finish a multi_ex for cb_explore_adf tests
// ============================================================
static void learn_cb_adf(VW::workspace& vw, const std::string& shared, const std::vector<std::string>& actions,
    int labeled_action_idx = 0, float cost = 1.0f, float prob = 0.5f)
{
  VW::multi_ex multi_ex;
  multi_ex.push_back(VW::read_example(vw, shared));
  for (int i = 0; i < static_cast<int>(actions.size()); ++i)
  {
    if (i == labeled_action_idx)
    {
      std::string label =
          std::to_string(labeled_action_idx) + ":" + std::to_string(cost) + ":" + std::to_string(prob) + " ";
      multi_ex.push_back(VW::read_example(vw, label + actions[i]));
    }
    else { multi_ex.push_back(VW::read_example(vw, actions[i])); }
  }
  multi_ex.push_back(VW::read_example(vw, ""));
  vw.learn(multi_ex);
  vw.finish_example(multi_ex);
}

static void predict_cb_adf(VW::workspace& vw, const std::string& shared, const std::vector<std::string>& actions)
{
  VW::multi_ex multi_ex;
  multi_ex.push_back(VW::read_example(vw, shared));
  for (const auto& a : actions) { multi_ex.push_back(VW::read_example(vw, a)); }
  multi_ex.push_back(VW::read_example(vw, ""));
  vw.predict(multi_ex);
  vw.finish_example(multi_ex);
}

// ============================================================
// Cbify (~25 tests)
// ============================================================

TEST(CoverageCbExplore, CbifyBasic2Actions)
{
  auto vw = VW::initialize(vwtest::make_args("--cbify", "2", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  EXPECT_GE(ex->pred.multiclass, 1u);
  EXPECT_LE(ex->pred.multiclass, 2u);
  vw->finish_example(*ex);
}

TEST(CoverageCbExplore, CbifyBasic3Actions)
{
  auto vw = VW::initialize(vwtest::make_args("--cbify", "3", "--quiet"));
  auto* ex = VW::read_example(*vw, "2 | a b c");
  vw->learn(*ex);
  EXPECT_GE(ex->pred.multiclass, 1u);
  EXPECT_LE(ex->pred.multiclass, 3u);
  vw->finish_example(*ex);
}

TEST(CoverageCbExplore, CbifyBasic5Actions)
{
  auto vw = VW::initialize(vwtest::make_args("--cbify", "5", "--quiet"));
  auto* ex = VW::read_example(*vw, "3 | a b c d e");
  vw->learn(*ex);
  EXPECT_GE(ex->pred.multiclass, 1u);
  EXPECT_LE(ex->pred.multiclass, 5u);
  vw->finish_example(*ex);
}

TEST(CoverageCbExplore, CbifyBasic10Actions)
{
  auto vw = VW::initialize(vwtest::make_args("--cbify", "10", "--quiet"));
  auto* ex = VW::read_example(*vw, "7 | a b c d e");
  vw->learn(*ex);
  EXPECT_GE(ex->pred.multiclass, 1u);
  EXPECT_LE(ex->pred.multiclass, 10u);
  vw->finish_example(*ex);
}

TEST(CoverageCbExplore, CbifyLossOption0Squared)
{
  auto vw = VW::initialize(vwtest::make_args("--cbify", "3", "--loss_option", "0", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageCbExplore, CbifyLossOption1Absolute)
{
  auto vw = VW::initialize(vwtest::make_args("--cbify", "3", "--loss_option", "1", "--quiet"));
  auto* ex = VW::read_example(*vw, "2 | a b");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageCbExplore, CbifyLossOption2ZeroOne)
{
  auto vw = VW::initialize(vwtest::make_args("--cbify", "3", "--loss_option", "2", "--quiet"));
  auto* ex = VW::read_example(*vw, "3 | a b");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageCbExplore, CbifyFlipLossSign)
{
  auto vw = VW::initialize(vwtest::make_args("--cbify", "3", "--flip_loss_sign", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageCbExplore, CbifyCustomLoss0Loss1)
{
  auto vw = VW::initialize(vwtest::make_args("--cbify", "3", "--loss0", "-0.5", "--loss1", "2.0", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageCbExplore, CbifyWithCostSensitive)
{
  auto vw = VW::initialize(vwtest::make_args("--cbify", "3", "--cbify_cs", "--quiet"));
  auto* ex = VW::read_example(*vw, "1:0.5 2:1.0 3:0.0 | a b c");
  vw->learn(*ex);
  EXPECT_GE(ex->pred.multiclass, 1u);
  EXPECT_LE(ex->pred.multiclass, 3u);
  vw->finish_example(*ex);
}

TEST(CoverageCbExplore, CbifyWithCostSensitiveMultipleExamples)
{
  auto vw = VW::initialize(vwtest::make_args("--cbify", "3", "--cbify_cs", "--quiet"));
  for (int i = 0; i < 10; i++)
  {
    auto* ex = VW::read_example(*vw, "1:0.5 2:1.0 3:0.0 | a b c");
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageCbExplore, CbifyRegDiscreteSquaredLoss)
{
  auto vw = VW::initialize(vwtest::make_args(
      "--cbify", "10", "--cbify_reg", "--cb_discrete", "--min_value", "0", "--max_value", "1", "--loss_option", "0",
      "--quiet"));
  auto* ex = VW::read_example(*vw, "0.5 | a b c");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  vw->finish_example(*ex);
}

TEST(CoverageCbExplore, CbifyRegDiscreteAbsoluteLoss)
{
  auto vw = VW::initialize(vwtest::make_args(
      "--cbify", "10", "--cbify_reg", "--cb_discrete", "--min_value", "0", "--max_value", "1", "--loss_option", "1",
      "--quiet"));
  auto* ex = VW::read_example(*vw, "0.5 | a b c");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  vw->finish_example(*ex);
}

TEST(CoverageCbExplore, CbifyRegDiscreteZeroOneLoss)
{
  auto vw = VW::initialize(vwtest::make_args(
      "--cbify", "10", "--cbify_reg", "--cb_discrete", "--min_value", "0", "--max_value", "1", "--loss_option", "2",
      "--loss_01_ratio", "0.2", "--quiet"));
  auto* ex = VW::read_example(*vw, "0.5 | a b c");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  vw->finish_example(*ex);
}

TEST(CoverageCbExplore, CbifyRegDiscreteLossReport1)
{
  auto vw = VW::initialize(vwtest::make_args(
      "--cbify", "10", "--cbify_reg", "--cb_discrete", "--min_value", "0", "--max_value", "1", "--loss_report", "1",
      "--quiet"));
  auto* ex = VW::read_example(*vw, "0.5 | a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageCbExplore, CbifyRegDiscreteLossReport1Absolute)
{
  auto vw = VW::initialize(vwtest::make_args(
      "--cbify", "10", "--cbify_reg", "--cb_discrete", "--min_value", "0", "--max_value", "1", "--loss_option", "1",
      "--loss_report", "1", "--quiet"));
  auto* ex = VW::read_example(*vw, "0.5 | a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageCbExplore, CbifyRegContinuousSquaredLoss)
{
  auto vw = VW::initialize(vwtest::make_args(
      "--cbify", "8", "--cbify_reg", "--min_value", "0", "--max_value", "1", "--loss_option", "0", "--quiet"));
  auto* ex = VW::read_example(*vw, "0.5 | a b c");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  vw->finish_example(*ex);
}

TEST(CoverageCbExplore, CbifyRegContinuousAbsoluteLoss)
{
  auto vw = VW::initialize(vwtest::make_args(
      "--cbify", "8", "--cbify_reg", "--min_value", "0", "--max_value", "1", "--loss_option", "1", "--quiet"));
  auto* ex = VW::read_example(*vw, "0.5 | a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageCbExplore, CbifyRegContinuousZeroOneLoss)
{
  auto vw = VW::initialize(vwtest::make_args(
      "--cbify", "8", "--cbify_reg", "--min_value", "0", "--max_value", "1", "--loss_option", "2", "--quiet"));
  auto* ex = VW::read_example(*vw, "0.5 | a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageCbExplore, CbifyRegContinuousLossReport1)
{
  auto vw = VW::initialize(vwtest::make_args(
      "--cbify", "8", "--cbify_reg", "--min_value", "0", "--max_value", "1", "--loss_report", "1", "--quiet"));
  auto* ex = VW::read_example(*vw, "0.5 | a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageCbExplore, CbifyRegContinuousLossReport1AbsLoss)
{
  auto vw = VW::initialize(vwtest::make_args(
      "--cbify", "8", "--cbify_reg", "--min_value", "0", "--max_value", "1", "--loss_option", "1", "--loss_report", "1",
      "--quiet"));
  auto* ex = VW::read_example(*vw, "0.5 | a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageCbExplore, CbifyMultipleExamples)
{
  auto vw = VW::initialize(vwtest::make_args("--cbify", "3", "--quiet"));
  for (int i = 0; i < 20; i++)
  {
    uint32_t label = (i % 3) + 1;
    auto* ex = VW::read_example(*vw, std::to_string(label) + " | a b c d");
    vw->learn(*ex);
    EXPECT_GE(ex->pred.multiclass, 1u);
    EXPECT_LE(ex->pred.multiclass, 3u);
    vw->finish_example(*ex);
  }
}

TEST(CoverageCbExplore, CbifyPredictOnly)
{
  auto vw = VW::initialize(vwtest::make_args("--cbify", "3", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->predict(*ex);
  EXPECT_GE(ex->pred.multiclass, 1u);
  EXPECT_LE(ex->pred.multiclass, 3u);
  vw->finish_example(*ex);
}

TEST(CoverageCbExplore, CbifyAdfMode)
{
  auto vw = VW::initialize(vwtest::make_args("--cbify", "3", "--cb_explore_adf", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  EXPECT_GE(ex->pred.multiclass, 1u);
  EXPECT_LE(ex->pred.multiclass, 3u);
  vw->finish_example(*ex);
}

TEST(CoverageCbExplore, CbifyAdfCsMode)
{
  auto vw = VW::initialize(vwtest::make_args("--cbify", "3", "--cb_explore_adf", "--cbify_cs", "--quiet"));
  auto* ex = VW::read_example(*vw, "1:0.5 2:1.0 3:0.0 | a b c");
  vw->learn(*ex);
  EXPECT_GE(ex->pred.multiclass, 1u);
  EXPECT_LE(ex->pred.multiclass, 3u);
  vw->finish_example(*ex);
}

TEST(CoverageCbExplore, CbifyRegDiscreteMultipleExamples)
{
  auto vw = VW::initialize(vwtest::make_args(
      "--cbify", "10", "--cbify_reg", "--cb_discrete", "--min_value", "0", "--max_value", "1", "--quiet"));
  for (int i = 0; i < 10; i++)
  {
    float label = static_cast<float>(i) / 10.0f;
    auto* ex = VW::read_example(*vw, std::to_string(label) + " | a b c");
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

// ============================================================
// OAA (~20 tests)
// ============================================================

TEST(CoverageCbExplore, OaaBasic2)
{
  auto vw = VW::initialize(vwtest::make_args("--oaa", "2", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  vw->predict(*ex);
  EXPECT_GE(ex->pred.multiclass, 1u);
  EXPECT_LE(ex->pred.multiclass, 2u);
  vw->finish_example(*ex);
}

TEST(CoverageCbExplore, OaaBasic3)
{
  auto vw = VW::initialize(vwtest::make_args("--oaa", "3", "--quiet"));
  auto* ex = VW::read_example(*vw, "2 | a b c");
  vw->learn(*ex);
  vw->predict(*ex);
  EXPECT_GE(ex->pred.multiclass, 1u);
  EXPECT_LE(ex->pred.multiclass, 3u);
  vw->finish_example(*ex);
}

TEST(CoverageCbExplore, OaaBasic5)
{
  auto vw = VW::initialize(vwtest::make_args("--oaa", "5", "--quiet"));
  auto* ex = VW::read_example(*vw, "3 | a b c d e");
  vw->learn(*ex);
  vw->predict(*ex);
  EXPECT_GE(ex->pred.multiclass, 1u);
  EXPECT_LE(ex->pred.multiclass, 5u);
  vw->finish_example(*ex);
}

TEST(CoverageCbExplore, OaaBasic10)
{
  auto vw = VW::initialize(vwtest::make_args("--oaa", "10", "--quiet"));
  auto* ex = VW::read_example(*vw, "7 | a b c d e");
  vw->learn(*ex);
  vw->predict(*ex);
  EXPECT_GE(ex->pred.multiclass, 1u);
  EXPECT_LE(ex->pred.multiclass, 10u);
  vw->finish_example(*ex);
}

TEST(CoverageCbExplore, OaaProbabilities)
{
  auto vw = VW::initialize(
      vwtest::make_args("--oaa", "3", "--probabilities", "--loss_function", "logistic", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  vw->predict(*ex);
  // probabilities mode outputs scalars
  EXPECT_EQ(ex->pred.scalars.size(), 3u);
  float sum = 0.f;
  for (size_t i = 0; i < ex->pred.scalars.size(); i++)
  {
    EXPECT_GE(ex->pred.scalars[i], 0.f);
    EXPECT_LE(ex->pred.scalars[i], 1.f);
    sum += ex->pred.scalars[i];
  }
  EXPECT_NEAR(sum, 1.0f, 0.01f);
  vw->finish_example(*ex);
}

TEST(CoverageCbExplore, OaaScores)
{
  auto vw = VW::initialize(vwtest::make_args("--oaa", "3", "--scores", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  vw->predict(*ex);
  EXPECT_EQ(ex->pred.scalars.size(), 3u);
  vw->finish_example(*ex);
}

TEST(CoverageCbExplore, OaaSubsample)
{
  auto vw = VW::initialize(vwtest::make_args("--oaa", "5", "--oaa_subsample", "2", "--quiet"));
  for (int i = 0; i < 20; i++)
  {
    uint32_t label = (i % 5) + 1;
    auto* ex = VW::read_example(*vw, std::to_string(label) + " | a b c d");
    vw->learn(*ex);
    EXPECT_GE(ex->pred.multiclass, 1u);
    EXPECT_LE(ex->pred.multiclass, 5u);
    vw->finish_example(*ex);
  }
}

TEST(CoverageCbExplore, OaaSubsampleEqualToK)
{
  // When subsample >= k, subsampling is turned off
  auto vw = VW::initialize(vwtest::make_args("--oaa", "3", "--oaa_subsample", "5", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageCbExplore, OaaMultipleExamples)
{
  auto vw = VW::initialize(vwtest::make_args("--oaa", "4", "--quiet"));
  for (int i = 0; i < 30; i++)
  {
    uint32_t label = (i % 4) + 1;
    auto* ex = VW::read_example(*vw, std::to_string(label) + " | a:" + std::to_string(i));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageCbExplore, OaaPredictAfterTrain)
{
  auto vw = VW::initialize(vwtest::make_args("--oaa", "3", "--quiet"));
  for (int i = 0; i < 20; i++)
  {
    uint32_t label = (i % 3) + 1;
    auto* ex = VW::read_example(*vw, std::to_string(label) + " | a:" + std::to_string(i));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
  auto* ex = VW::read_example(*vw, "| a:5");
  vw->predict(*ex);
  EXPECT_GE(ex->pred.multiclass, 1u);
  EXPECT_LE(ex->pred.multiclass, 3u);
  vw->finish_example(*ex);
}

TEST(CoverageCbExplore, OaaIndexing0)
{
  auto vw = VW::initialize(vwtest::make_args("--oaa", "3", "--indexing", "0", "--quiet"));
  auto* ex = VW::read_example(*vw, "0 | a b c");
  vw->learn(*ex);
  vw->predict(*ex);
  EXPECT_LE(ex->pred.multiclass, 2u);
  vw->finish_example(*ex);
}

TEST(CoverageCbExplore, OaaIndexing1)
{
  auto vw = VW::initialize(vwtest::make_args("--oaa", "3", "--indexing", "1", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  vw->predict(*ex);
  EXPECT_GE(ex->pred.multiclass, 1u);
  EXPECT_LE(ex->pred.multiclass, 3u);
  vw->finish_example(*ex);
}

TEST(CoverageCbExplore, OaaProbabilitiesMultipleExamples)
{
  auto vw = VW::initialize(
      vwtest::make_args("--oaa", "3", "--probabilities", "--loss_function", "logistic", "--quiet"));
  for (int i = 0; i < 15; i++)
  {
    uint32_t label = (i % 3) + 1;
    auto* ex = VW::read_example(*vw, std::to_string(label) + " | a:" + std::to_string(i));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageCbExplore, OaaScoresMultipleExamples)
{
  auto vw = VW::initialize(vwtest::make_args("--oaa", "4", "--scores", "--quiet"));
  for (int i = 0; i < 15; i++)
  {
    uint32_t label = (i % 4) + 1;
    auto* ex = VW::read_example(*vw, std::to_string(label) + " | a:" + std::to_string(i));
    vw->learn(*ex);
    EXPECT_EQ(ex->pred.scalars.size(), 4u);
    vw->finish_example(*ex);
  }
}

TEST(CoverageCbExplore, OaaSubsampleSmall)
{
  auto vw = VW::initialize(vwtest::make_args("--oaa", "10", "--oaa_subsample", "1", "--quiet"));
  for (int i = 0; i < 20; i++)
  {
    uint32_t label = (i % 10) + 1;
    auto* ex = VW::read_example(*vw, std::to_string(label) + " | a b c d");
    vw->learn(*ex);
    EXPECT_GE(ex->pred.multiclass, 1u);
    EXPECT_LE(ex->pred.multiclass, 10u);
    vw->finish_example(*ex);
  }
}

TEST(CoverageCbExplore, OaaSubsampleLarge)
{
  auto vw = VW::initialize(vwtest::make_args("--oaa", "10", "--oaa_subsample", "5", "--quiet"));
  for (int i = 0; i < 20; i++)
  {
    uint32_t label = (i % 10) + 1;
    auto* ex = VW::read_example(*vw, std::to_string(label) + " | a b c d");
    vw->learn(*ex);
    EXPECT_GE(ex->pred.multiclass, 1u);
    EXPECT_LE(ex->pred.multiclass, 10u);
    vw->finish_example(*ex);
  }
}

TEST(CoverageCbExplore, OaaSmallBits)
{
  auto vw = VW::initialize(vwtest::make_args("--oaa", "3", "-b", "10", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageCbExplore, OaaLowLearningRate)
{
  auto vw = VW::initialize(vwtest::make_args("--oaa", "3", "-l", "0.01", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageCbExplore, OaaHighLearningRate)
{
  auto vw = VW::initialize(vwtest::make_args("--oaa", "3", "-l", "1.0", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

// ============================================================
// CB Explore ADF - Epsilon Greedy (~8 tests)
// ============================================================

TEST(CoverageCbExplore, CbExploreAdfEpsilonBasic)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--epsilon", "0.1", "--quiet"));
  learn_cb_adf(*vw, "shared | s_1 s_2", {"| a_1 a_2", "| b_1 b_2", "| c_1 c_2"}, 0, 1.0f, 0.5f);
}

TEST(CoverageCbExplore, CbExploreAdfEpsilonZero)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--epsilon", "0.0", "--quiet"));
  learn_cb_adf(*vw, "shared | s_1", {"| a_1", "| b_1"}, 0, 1.0f, 0.5f);
}

TEST(CoverageCbExplore, CbExploreAdfEpsilonOne)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--epsilon", "1.0", "--quiet"));
  learn_cb_adf(*vw, "shared | s_1", {"| a_1", "| b_1", "| c_1"}, 0, 1.0f, 0.5f);
}

TEST(CoverageCbExplore, CbExploreAdfEpsilonMultipleExamples)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--epsilon", "0.2", "--quiet"));
  for (int i = 0; i < 20; i++)
  {
    int action = i % 3;
    learn_cb_adf(*vw, "shared | s_" + std::to_string(i), {"| a_1", "| b_1", "| c_1"}, action, 1.0f, 0.33f);
  }
}

TEST(CoverageCbExplore, CbExploreAdfEpsilonPredictOnly)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--epsilon", "0.1", "--quiet"));
  learn_cb_adf(*vw, "shared | s_1", {"| a_1", "| b_1"}, 0, 1.0f, 0.5f);
  predict_cb_adf(*vw, "shared | s_1", {"| a_1", "| b_1"});
}

TEST(CoverageCbExplore, CbExploreAdfEpsilonManyActions)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--epsilon", "0.05", "--quiet"));
  learn_cb_adf(*vw, "shared | s_1",
      {"| a_1", "| a_2", "| a_3", "| a_4", "| a_5", "| a_6", "| a_7", "| a_8", "| a_9", "| a_10"}, 0, 1.0f, 0.1f);
}

TEST(CoverageCbExplore, CbExploreAdfEpsilonZeroCost)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--epsilon", "0.1", "--quiet"));
  learn_cb_adf(*vw, "shared | s_1", {"| a_1", "| b_1", "| c_1"}, 0, 0.0f, 0.33f);
}

TEST(CoverageCbExplore, CbExploreAdfEpsilonHighCost)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--epsilon", "0.1", "--quiet"));
  learn_cb_adf(*vw, "shared | s_1", {"| a_1", "| b_1", "| c_1"}, 0, 100.0f, 0.33f);
}

// ============================================================
// CB Explore ADF - Softmax (~5 tests)
// ============================================================

TEST(CoverageCbExplore, CbExploreAdfSoftmaxBasic)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--softmax", "--lambda", "1.0", "--quiet"));
  learn_cb_adf(*vw, "shared | s_1", {"| a_1", "| b_1", "| c_1"}, 0, 1.0f, 0.33f);
}

TEST(CoverageCbExplore, CbExploreAdfSoftmaxHighLambda)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--softmax", "--lambda", "10.0", "--quiet"));
  learn_cb_adf(*vw, "shared | s_1", {"| a_1", "| b_1"}, 0, 1.0f, 0.5f);
}

TEST(CoverageCbExplore, CbExploreAdfSoftmaxLowLambda)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--softmax", "--lambda", "0.01", "--quiet"));
  learn_cb_adf(*vw, "shared | s_1", {"| a_1", "| b_1"}, 0, 1.0f, 0.5f);
}

TEST(CoverageCbExplore, CbExploreAdfSoftmaxNegativeLambda)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--softmax", "--lambda", "-1.0", "--quiet"));
  learn_cb_adf(*vw, "shared | s_1", {"| a_1", "| b_1"}, 0, 1.0f, 0.5f);
}

TEST(CoverageCbExplore, CbExploreAdfSoftmaxMultipleExamples)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--softmax", "--lambda", "1.0", "--quiet"));
  for (int i = 0; i < 15; i++)
  {
    learn_cb_adf(*vw, "shared | s_" + std::to_string(i), {"| a_1", "| b_1", "| c_1"}, i % 3, 1.0f, 0.33f);
  }
}

// ============================================================
// CB Explore ADF - Bag (~5 tests)
// ============================================================

TEST(CoverageCbExplore, CbExploreAdfBag2)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--bag", "2", "--quiet"));
  learn_cb_adf(*vw, "shared | s_1", {"| a_1", "| b_1"}, 0, 1.0f, 0.5f);
}

TEST(CoverageCbExplore, CbExploreAdfBag5)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--bag", "5", "--quiet"));
  learn_cb_adf(*vw, "shared | s_1", {"| a_1", "| b_1", "| c_1"}, 0, 1.0f, 0.33f);
}

TEST(CoverageCbExplore, CbExploreAdfBag10)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--bag", "10", "--quiet"));
  learn_cb_adf(*vw, "shared | s_1", {"| a_1", "| b_1"}, 0, 1.0f, 0.5f);
}

TEST(CoverageCbExplore, CbExploreAdfBagMultipleExamples)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--bag", "3", "--quiet"));
  for (int i = 0; i < 15; i++)
  {
    learn_cb_adf(*vw, "shared | s_" + std::to_string(i), {"| a_1", "| b_1", "| c_1"}, i % 3, 1.0f, 0.33f);
  }
}

TEST(CoverageCbExplore, CbExploreAdfBagPredict)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--bag", "3", "--quiet"));
  learn_cb_adf(*vw, "shared | s_1", {"| a_1", "| b_1"}, 0, 1.0f, 0.5f);
  predict_cb_adf(*vw, "shared | s_1", {"| a_1", "| b_1"});
}

// ============================================================
// CB Explore ADF - Cover (~10 tests)
// ============================================================

TEST(CoverageCbExplore, CbExploreAdfCover2)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--cover", "2", "--quiet"));
  learn_cb_adf(*vw, "shared | s_1", {"| a_1", "| b_1"}, 0, 1.0f, 0.5f);
}

TEST(CoverageCbExplore, CbExploreAdfCover3)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--cover", "3", "--quiet"));
  learn_cb_adf(*vw, "shared | s_1", {"| a_1", "| b_1", "| c_1"}, 0, 1.0f, 0.33f);
}

TEST(CoverageCbExplore, CbExploreAdfCover5)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--cover", "5", "--quiet"));
  learn_cb_adf(*vw, "shared | s_1", {"| a_1", "| b_1"}, 0, 1.0f, 0.5f);
}

TEST(CoverageCbExplore, CbExploreAdfCover10)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--cover", "10", "--quiet"));
  learn_cb_adf(*vw, "shared | s_1", {"| a_1", "| b_1", "| c_1"}, 0, 1.0f, 0.33f);
}

TEST(CoverageCbExplore, CbExploreAdfCoverNounif)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--cover", "3", "--nounif", "--quiet"));
  learn_cb_adf(*vw, "shared | s_1", {"| a_1", "| b_1"}, 0, 1.0f, 0.5f);
}

TEST(CoverageCbExplore, CbExploreAdfCoverFirstOnly)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--cover", "3", "--first_only", "--quiet"));
  learn_cb_adf(*vw, "shared | s_1", {"| a_1", "| b_1"}, 0, 1.0f, 0.5f);
}

TEST(CoverageCbExplore, CbExploreAdfCoverPsi)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--cover", "3", "--psi", "0.5", "--quiet"));
  learn_cb_adf(*vw, "shared | s_1", {"| a_1", "| b_1", "| c_1"}, 0, 1.0f, 0.33f);
}

TEST(CoverageCbExplore, CbExploreAdfCoverMultipleExamples)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--cover", "3", "--quiet"));
  for (int i = 0; i < 15; i++)
  {
    learn_cb_adf(*vw, "shared | s_" + std::to_string(i), {"| a_1", "| b_1", "| c_1"}, i % 3, 1.0f, 0.33f);
  }
}

TEST(CoverageCbExplore, CbExploreAdfCoverCbTypeIps)
{
  auto vw =
      VW::initialize(vwtest::make_args("--cb_explore_adf", "--cover", "3", "--cb_type", "ips", "--quiet"));
  learn_cb_adf(*vw, "shared | s_1", {"| a_1", "| b_1"}, 0, 1.0f, 0.5f);
}

TEST(CoverageCbExplore, CbExploreAdfCoverCbTypeDr)
{
  auto vw =
      VW::initialize(vwtest::make_args("--cb_explore_adf", "--cover", "3", "--cb_type", "dr", "--quiet"));
  learn_cb_adf(*vw, "shared | s_1", {"| a_1", "| b_1"}, 0, 1.0f, 0.5f);
}

// ============================================================
// CB Explore ADF - Synthcover (~7 tests)
// ============================================================

TEST(CoverageCbExplore, CbExploreAdfSynthcoverBasic)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--synthcover", "--quiet"));
  learn_cb_adf(*vw, "shared | s_1", {"| a_1", "| b_1"}, 0, 1.0f, 0.5f);
}

TEST(CoverageCbExplore, CbExploreAdfSynthcoverSmallSize)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--synthcover", "--synthcoversize", "10", "--quiet"));
  learn_cb_adf(*vw, "shared | s_1", {"| a_1", "| b_1", "| c_1"}, 0, 1.0f, 0.33f);
}

TEST(CoverageCbExplore, CbExploreAdfSynthcoverLargeSize)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--synthcover", "--synthcoversize", "500", "--quiet"));
  learn_cb_adf(*vw, "shared | s_1", {"| a_1", "| b_1"}, 0, 1.0f, 0.5f);
}

TEST(CoverageCbExplore, CbExploreAdfSynthcoverPsi)
{
  auto vw = VW::initialize(
      vwtest::make_args("--cb_explore_adf", "--synthcover", "--synthcoverpsi", "0.5", "--quiet"));
  learn_cb_adf(*vw, "shared | s_1", {"| a_1", "| b_1"}, 0, 1.0f, 0.5f);
}

TEST(CoverageCbExplore, CbExploreAdfSynthcoverEpsilon)
{
  auto vw = VW::initialize(
      vwtest::make_args("--cb_explore_adf", "--synthcover", "--epsilon", "0.2", "--quiet"));
  learn_cb_adf(*vw, "shared | s_1", {"| a_1", "| b_1", "| c_1"}, 0, 1.0f, 0.33f);
}

TEST(CoverageCbExplore, CbExploreAdfSynthcoverMultipleExamples)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--synthcover", "--quiet"));
  for (int i = 0; i < 20; i++)
  {
    learn_cb_adf(*vw, "shared | s_" + std::to_string(i), {"| a_1", "| b_1", "| c_1"}, i % 3, 1.0f, 0.33f);
  }
}

TEST(CoverageCbExplore, CbExploreAdfSynthcoverPredict)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--synthcover", "--quiet"));
  learn_cb_adf(*vw, "shared | s_1", {"| a_1", "| b_1"}, 0, 1.0f, 0.5f);
  predict_cb_adf(*vw, "shared | s_1", {"| a_1", "| b_1"});
}

// ============================================================
// Warm CB (~15 tests)
// ============================================================

TEST(CoverageCbExplore, WarmCbBasic2Actions)
{
  auto vw = VW::initialize(vwtest::make_args("--warm_cb", "2", "--cb_explore_adf", "--epsilon", "0.1", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageCbExplore, WarmCbBasic3Actions)
{
  auto vw = VW::initialize(vwtest::make_args("--warm_cb", "3", "--cb_explore_adf", "--epsilon", "0.1", "--quiet"));
  auto* ex = VW::read_example(*vw, "2 | a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageCbExplore, WarmCbBasic5Actions)
{
  auto vw = VW::initialize(vwtest::make_args("--warm_cb", "5", "--cb_explore_adf", "--epsilon", "0.1", "--quiet"));
  auto* ex = VW::read_example(*vw, "3 | a b c d e");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageCbExplore, WarmCbWarmStartPeriod)
{
  auto vw = VW::initialize(
      vwtest::make_args("--warm_cb", "3", "--cb_explore_adf", "--epsilon", "0.1", "--warm_start", "5", "--quiet"));
  // First 5 examples are warm start
  for (int i = 0; i < 10; i++)
  {
    auto* ex = VW::read_example(*vw, std::to_string((i % 3) + 1) + " | a:" + std::to_string(i));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageCbExplore, WarmCbInteractionPeriod)
{
  auto vw = VW::initialize(vwtest::make_args(
      "--warm_cb", "3", "--cb_explore_adf", "--epsilon", "0.1", "--warm_start", "3", "--interaction", "5", "--quiet"));
  for (int i = 0; i < 15; i++)
  {
    auto* ex = VW::read_example(*vw, std::to_string((i % 3) + 1) + " | a:" + std::to_string(i));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageCbExplore, WarmCbWarmStartUpdate)
{
  auto vw = VW::initialize(vwtest::make_args(
      "--warm_cb", "3", "--cb_explore_adf", "--epsilon", "0.1", "--warm_start", "5", "--warm_start_update", "--quiet"));
  for (int i = 0; i < 10; i++)
  {
    auto* ex = VW::read_example(*vw, std::to_string((i % 3) + 1) + " | a:" + std::to_string(i));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageCbExplore, WarmCbInteractionUpdate)
{
  auto vw = VW::initialize(vwtest::make_args("--warm_cb", "3", "--cb_explore_adf", "--epsilon", "0.1", "--warm_start",
      "3", "--interaction", "10", "--interaction_update", "--quiet"));
  for (int i = 0; i < 20; i++)
  {
    auto* ex = VW::read_example(*vw, std::to_string((i % 3) + 1) + " | a:" + std::to_string(i));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageCbExplore, WarmCbBothUpdates)
{
  auto vw = VW::initialize(vwtest::make_args("--warm_cb", "3", "--cb_explore_adf", "--epsilon", "0.1", "--warm_start",
      "3", "--interaction", "10", "--warm_start_update", "--interaction_update", "--quiet"));
  for (int i = 0; i < 20; i++)
  {
    auto* ex = VW::read_example(*vw, std::to_string((i % 3) + 1) + " | a:" + std::to_string(i));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageCbExplore, WarmCbChoicesLambda3)
{
  auto vw = VW::initialize(vwtest::make_args("--warm_cb", "3", "--cb_explore_adf", "--epsilon", "0.1", "--warm_start",
      "3", "--interaction", "10", "--choices_lambda", "3", "--warm_start_update", "--interaction_update", "--quiet"));
  for (int i = 0; i < 20; i++)
  {
    auto* ex = VW::read_example(*vw, std::to_string((i % 3) + 1) + " | a:" + std::to_string(i));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageCbExplore, WarmCbChoicesLambda5)
{
  auto vw = VW::initialize(vwtest::make_args("--warm_cb", "3", "--cb_explore_adf", "--epsilon", "0.1", "--warm_start",
      "3", "--interaction", "10", "--choices_lambda", "5", "--warm_start_update", "--interaction_update", "--quiet"));
  for (int i = 0; i < 20; i++)
  {
    auto* ex = VW::read_example(*vw, std::to_string((i % 3) + 1) + " | a:" + std::to_string(i));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageCbExplore, WarmCbLambdaScheme2)
{
  auto vw = VW::initialize(vwtest::make_args("--warm_cb", "3", "--cb_explore_adf", "--epsilon", "0.1", "--warm_start",
      "3", "--interaction", "10", "--choices_lambda", "3", "--lambda_scheme", "2", "--warm_start_update",
      "--interaction_update", "--quiet"));
  for (int i = 0; i < 20; i++)
  {
    auto* ex = VW::read_example(*vw, std::to_string((i % 3) + 1) + " | a:" + std::to_string(i));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageCbExplore, WarmCbLambdaScheme3)
{
  auto vw = VW::initialize(vwtest::make_args("--warm_cb", "3", "--cb_explore_adf", "--epsilon", "0.1", "--warm_start",
      "3", "--interaction", "10", "--choices_lambda", "3", "--lambda_scheme", "3", "--warm_start_update",
      "--interaction_update", "--quiet"));
  for (int i = 0; i < 20; i++)
  {
    auto* ex = VW::read_example(*vw, std::to_string((i % 3) + 1) + " | a:" + std::to_string(i));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageCbExplore, WarmCbLambdaScheme4)
{
  auto vw = VW::initialize(vwtest::make_args("--warm_cb", "3", "--cb_explore_adf", "--epsilon", "0.1", "--warm_start",
      "3", "--interaction", "10", "--choices_lambda", "3", "--lambda_scheme", "4", "--warm_start_update",
      "--interaction_update", "--quiet"));
  for (int i = 0; i < 20; i++)
  {
    auto* ex = VW::read_example(*vw, std::to_string((i % 3) + 1) + " | a:" + std::to_string(i));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageCbExplore, WarmCbSimBandit)
{
  auto vw = VW::initialize(vwtest::make_args("--warm_cb", "3", "--cb_explore_adf", "--epsilon", "0.1", "--warm_start",
      "5", "--interaction", "10", "--sim_bandit", "--warm_start_update", "--interaction_update", "--quiet"));
  for (int i = 0; i < 20; i++)
  {
    auto* ex = VW::read_example(*vw, std::to_string((i % 3) + 1) + " | a:" + std::to_string(i));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageCbExplore, WarmCbCorruptUar)
{
  auto vw = VW::initialize(vwtest::make_args("--warm_cb", "3", "--cb_explore_adf", "--epsilon", "0.1", "--warm_start",
      "5", "--interaction", "10", "--corrupt_type_warm_start", "1", "--corrupt_prob_warm_start", "0.5",
      "--warm_start_update", "--interaction_update", "--quiet"));
  for (int i = 0; i < 20; i++)
  {
    auto* ex = VW::read_example(*vw, std::to_string((i % 3) + 1) + " | a:" + std::to_string(i));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

// ============================================================
// CB Algorithms (~10 tests)
// ============================================================

TEST(CoverageCbExplore, CbAlgsIps)
{
  auto vw = VW::initialize(vwtest::make_args("--cb", "3", "--cb_type", "ips", "--cb_force_legacy", "--quiet"));
  auto* ex = VW::read_example(*vw, "1:1.0:0.5 | a b c");
  vw->learn(*ex);
  EXPECT_GE(ex->pred.multiclass, 1u);
  EXPECT_LE(ex->pred.multiclass, 3u);
  vw->finish_example(*ex);
}

TEST(CoverageCbExplore, CbAlgsDm)
{
  auto vw = VW::initialize(vwtest::make_args("--cb", "3", "--cb_type", "dm", "--cb_force_legacy", "--quiet"));
  auto* ex = VW::read_example(*vw, "1:1.0:0.5 | a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageCbExplore, CbAlgsDr)
{
  auto vw = VW::initialize(vwtest::make_args("--cb", "3", "--cb_type", "dr", "--cb_force_legacy", "--quiet"));
  auto* ex = VW::read_example(*vw, "1:1.0:0.5 | a b c");
  vw->learn(*ex);
  EXPECT_GE(ex->pred.multiclass, 1u);
  EXPECT_LE(ex->pred.multiclass, 3u);
  vw->finish_example(*ex);
}

TEST(CoverageCbExplore, CbAlgsIpsMultipleExamples)
{
  auto vw = VW::initialize(vwtest::make_args("--cb", "3", "--cb_type", "ips", "--cb_force_legacy", "--quiet"));
  for (int i = 0; i < 20; i++)
  {
    uint32_t action = (i % 3) + 1;
    auto* ex = VW::read_example(
        *vw, std::to_string(action) + ":1.0:0.33 | a:" + std::to_string(i) + " b:" + std::to_string(i * 2));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageCbExplore, CbAlgsDrMultipleExamples)
{
  auto vw = VW::initialize(vwtest::make_args("--cb", "3", "--cb_type", "dr", "--cb_force_legacy", "--quiet"));
  for (int i = 0; i < 20; i++)
  {
    uint32_t action = (i % 3) + 1;
    auto* ex = VW::read_example(
        *vw, std::to_string(action) + ":1.0:0.33 | a:" + std::to_string(i) + " b:" + std::to_string(i * 2));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageCbExplore, CbAlgsTestLabel)
{
  auto vw = VW::initialize(vwtest::make_args("--cb", "3", "--cb_type", "dr", "--cb_force_legacy", "--quiet"));
  // Test label (no cost observed)
  auto* ex = VW::read_example(*vw, "| a b c");
  vw->predict(*ex);
  EXPECT_GE(ex->pred.multiclass, 1u);
  EXPECT_LE(ex->pred.multiclass, 3u);
  vw->finish_example(*ex);
}

TEST(CoverageCbExplore, CbAlgsIpsPredictAfterTrain)
{
  auto vw = VW::initialize(vwtest::make_args("--cb", "3", "--cb_type", "ips", "--cb_force_legacy", "--quiet"));
  for (int i = 0; i < 10; i++)
  {
    auto* ex = VW::read_example(*vw, "1:1.0:0.5 | a b c");
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
  auto* ex = VW::read_example(*vw, "| a b c");
  vw->predict(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageCbExplore, CbAlgsDmMultipleExamples)
{
  auto vw = VW::initialize(vwtest::make_args("--cb", "3", "--cb_type", "dm", "--cb_force_legacy", "--quiet"));
  for (int i = 0; i < 10; i++)
  {
    auto* ex = VW::read_example(*vw, std::to_string((i % 3) + 1) + ":1.0:0.33 | a:" + std::to_string(i));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageCbExplore, CbAlgs2Actions)
{
  auto vw = VW::initialize(vwtest::make_args("--cb", "2", "--cb_type", "dr", "--cb_force_legacy", "--quiet"));
  auto* ex = VW::read_example(*vw, "1:1.0:0.5 | a b");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageCbExplore, CbAlgs5Actions)
{
  auto vw = VW::initialize(vwtest::make_args("--cb", "5", "--cb_type", "dr", "--cb_force_legacy", "--quiet"));
  auto* ex = VW::read_example(*vw, "3:1.0:0.2 | a b c d e");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

// ============================================================
// CB Explore ADF - MTR cb_type (~5 tests)
// ============================================================

TEST(CoverageCbExplore, CbExploreAdfMtr)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--cb_type", "mtr", "--quiet"));
  learn_cb_adf(*vw, "shared | s_1", {"| a_1", "| b_1"}, 0, 1.0f, 0.5f);
}

TEST(CoverageCbExplore, CbExploreAdfDr)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--cb_type", "dr", "--quiet"));
  learn_cb_adf(*vw, "shared | s_1", {"| a_1", "| b_1"}, 0, 1.0f, 0.5f);
}

TEST(CoverageCbExplore, CbExploreAdfIps)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--cb_type", "ips", "--quiet"));
  learn_cb_adf(*vw, "shared | s_1", {"| a_1", "| b_1"}, 0, 1.0f, 0.5f);
}

TEST(CoverageCbExplore, CbExploreAdfMtrMultipleExamples)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--cb_type", "mtr", "--quiet"));
  for (int i = 0; i < 15; i++)
  {
    learn_cb_adf(*vw, "shared | s_" + std::to_string(i), {"| a_1", "| b_1", "| c_1"}, i % 3, 1.0f, 0.33f);
  }
}

TEST(CoverageCbExplore, CbExploreAdfDrMultipleExamples)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--cb_type", "dr", "--quiet"));
  for (int i = 0; i < 15; i++)
  {
    learn_cb_adf(*vw, "shared | s_" + std::to_string(i), {"| a_1", "| b_1", "| c_1"}, i % 3, 1.0f, 0.33f);
  }
}

// ============================================================
// CB Explore ADF - Metrics (~3 tests)
// ============================================================

TEST(CoverageCbExplore, CbExploreAdfMetrics)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--epsilon", "0.1", "--extra_metrics", "/dev/null", "--quiet"));
  learn_cb_adf(*vw, "shared | s_1", {"| a_1", "| b_1", "| c_1"}, 0, 1.0f, 0.33f);
}

TEST(CoverageCbExplore, CbExploreAdfMetricsMultipleExamples)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--epsilon", "0.1", "--extra_metrics", "/dev/null", "--quiet"));
  for (int i = 0; i < 10; i++)
  {
    learn_cb_adf(*vw, "shared | s_" + std::to_string(i), {"| a_1", "| b_1", "| c_1"}, i % 3, 1.0f, 0.33f);
  }
}

TEST(CoverageCbExplore, CbExploreAdfMetricsPredictInLearn)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--epsilon", "0.1", "--extra_metrics", "/dev/null", "--quiet"));
  // First, a labeled example
  learn_cb_adf(*vw, "shared | s_1", {"| a_1", "| b_1"}, 0, 1.0f, 0.5f);
  // Then, a test example through learn path (no label -> predict_in_learn)
  predict_cb_adf(*vw, "shared | s_2", {"| a_1", "| b_1"});
}

// ============================================================
// CB Explore ADF - Single action edge case (~2 tests)
// ============================================================

TEST(CoverageCbExplore, CbExploreAdfSingleAction)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--epsilon", "0.1", "--quiet"));
  // Only one action
  VW::multi_ex multi_ex;
  multi_ex.push_back(VW::read_example(*vw, "shared | s_1"));
  multi_ex.push_back(VW::read_example(*vw, "0:1.0:1.0 | a_1"));
  multi_ex.push_back(VW::read_example(*vw, ""));
  vw->learn(multi_ex);
  vw->finish_example(multi_ex);
}

TEST(CoverageCbExplore, CbExploreAdfSynthcoverSingleAction)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--synthcover", "--quiet"));
  VW::multi_ex multi_ex;
  multi_ex.push_back(VW::read_example(*vw, "shared | s_1"));
  multi_ex.push_back(VW::read_example(*vw, "0:1.0:1.0 | a_1"));
  multi_ex.push_back(VW::read_example(*vw, ""));
  vw->learn(multi_ex);
  vw->finish_example(multi_ex);
}

// ============================================================
// Warm CB - CostSensitive mode (~3 tests)
// ============================================================

TEST(CoverageCbExplore, WarmCbCostSensitive)
{
  auto vw = VW::initialize(
      vwtest::make_args("--warm_cb", "3", "--warm_cb_cs", "--cb_explore_adf", "--epsilon", "0.1", "--quiet"));
  auto* ex = VW::read_example(*vw, "1:0.5 2:1.0 3:0.0 | a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageCbExplore, WarmCbCostSensitiveMultiple)
{
  auto vw = VW::initialize(
      vwtest::make_args("--warm_cb", "3", "--warm_cb_cs", "--cb_explore_adf", "--epsilon", "0.1", "--quiet"));
  for (int i = 0; i < 10; i++)
  {
    auto* ex = VW::read_example(*vw, "1:0.5 2:1.0 3:0.0 | a:" + std::to_string(i));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageCbExplore, WarmCbNoEpsilon)
{
  // Should default epsilon to 0.05
  auto vw = VW::initialize(vwtest::make_args("--warm_cb", "3", "--cb_explore_adf", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

// ============================================================
// CB Explore ADF - Cover with epsilon decay (~2 tests)
// ============================================================

TEST(CoverageCbExplore, CbExploreAdfCoverEpsilonDecay)
{
  // Not supplying --epsilon triggers epsilon_decay mode (epsilon=1 with decay)
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--cover", "3", "--quiet"));
  for (int i = 0; i < 20; i++)
  {
    learn_cb_adf(*vw, "shared | s_" + std::to_string(i), {"| a_1", "| b_1", "| c_1"}, i % 3, 1.0f, 0.33f);
  }
}

TEST(CoverageCbExplore, CbExploreAdfCoverNounifMultiple)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--cover", "3", "--nounif", "--quiet"));
  for (int i = 0; i < 15; i++)
  {
    learn_cb_adf(*vw, "shared | s_" + std::to_string(i), {"| a_1", "| b_1"}, i % 2, 1.0f, 0.5f);
  }
}

// ============================================================
// Cbify ADF predict path (~2 tests)
// ============================================================

TEST(CoverageCbExplore, CbifyAdfPredictOnly)
{
  auto vw = VW::initialize(vwtest::make_args("--cbify", "3", "--cb_explore_adf", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->predict(*ex);
  EXPECT_GE(ex->pred.multiclass, 1u);
  EXPECT_LE(ex->pred.multiclass, 3u);
  vw->finish_example(*ex);
}

TEST(CoverageCbExplore, CbifyAdfMultipleExamples)
{
  auto vw = VW::initialize(vwtest::make_args("--cbify", "3", "--cb_explore_adf", "--quiet"));
  for (int i = 0; i < 15; i++)
  {
    auto* ex = VW::read_example(*vw, std::to_string((i % 3) + 1) + " | a:" + std::to_string(i));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

// ============================================================
// CB Explore ADF - Regcb and Squarecb (~4 tests)
// ============================================================

TEST(CoverageCbExplore, CbExploreAdfRegcb)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--regcb", "--quiet"));
  learn_cb_adf(*vw, "shared | s_1", {"| a_1", "| b_1"}, 0, 1.0f, 0.5f);
}

TEST(CoverageCbExplore, CbExploreAdfSquarecb)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--squarecb", "--quiet"));
  learn_cb_adf(*vw, "shared | s_1", {"| a_1", "| b_1"}, 0, 1.0f, 0.5f);
}

TEST(CoverageCbExplore, CbExploreAdfRegcbMultiple)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--regcb", "--quiet"));
  for (int i = 0; i < 10; i++)
  {
    learn_cb_adf(*vw, "shared | s_" + std::to_string(i), {"| a_1", "| b_1", "| c_1"}, i % 3, 1.0f, 0.33f);
  }
}

TEST(CoverageCbExplore, CbExploreAdfSquarecbMultiple)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--squarecb", "--quiet"));
  for (int i = 0; i < 10; i++)
  {
    learn_cb_adf(*vw, "shared | s_" + std::to_string(i), {"| a_1", "| b_1", "| c_1"}, i % 3, 1.0f, 0.33f);
  }
}

// ============================================================
// Cbify with baseline (~2 tests)
// ============================================================

TEST(CoverageCbExplore, CbifyBaseline)
{
  auto vw = VW::initialize(vwtest::make_args("--cbify", "3", "--baseline", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageCbExplore, CbifyAdfBaseline)
{
  auto vw = VW::initialize(vwtest::make_args("--cbify", "3", "--cb_explore_adf", "--baseline", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

// ============================================================
// Warm CB with baseline (~1 test)
// ============================================================

TEST(CoverageCbExplore, WarmCbBaseline)
{
  auto vw = VW::initialize(
      vwtest::make_args("--warm_cb", "3", "--cb_explore_adf", "--epsilon", "0.1", "--baseline", "--quiet"));
  for (int i = 0; i < 10; i++)
  {
    auto* ex = VW::read_example(*vw, std::to_string((i % 3) + 1) + " | a:" + std::to_string(i));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}
