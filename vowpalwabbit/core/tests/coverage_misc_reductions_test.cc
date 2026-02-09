// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

// Batch 17: Remaining Reductions coverage tests targeting csoaa_ldf.cc, ect.cc,
// bs.cc, nn.cc, ftrl.cc, active.cc, lrq.cc, interact.cc, explore_eval.cc,
// freegrad.cc, cbzo.cc, confidence.cc

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
static void learn_cb_adf_b17(VW::workspace& vw, const std::string& shared, const std::vector<std::string>& actions,
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
  vw.learn(multi_ex);
  vw.finish_example(multi_ex);
  auto* empty = VW::read_example(vw, "");
  VW::finish_example(vw, *empty);
}

// Helper: learn+finish a csoaa_ldf multi_ex
// Following established pattern: shared example + action lines, no empty terminator in multi_ex.
// Empty terminator is finished separately.
static void learn_csoaa_ldf_multi(
    VW::workspace& vw, const std::string& shared_line, const std::vector<std::string>& action_lines)
{
  VW::multi_ex multi_ex;
  multi_ex.push_back(VW::read_example(vw, shared_line));
  for (const auto& line : action_lines) { multi_ex.push_back(VW::read_example(vw, line)); }
  vw.learn(multi_ex);
  vw.finish_example(multi_ex);
  auto* empty = VW::read_example(vw, "");
  VW::finish_example(vw, *empty);
}

static void predict_csoaa_ldf_multi(
    VW::workspace& vw, const std::string& shared_line, const std::vector<std::string>& action_lines)
{
  VW::multi_ex multi_ex;
  multi_ex.push_back(VW::read_example(vw, shared_line));
  for (const auto& line : action_lines) { multi_ex.push_back(VW::read_example(vw, line)); }
  vw.predict(multi_ex);
  vw.finish_example(multi_ex);
  auto* empty = VW::read_example(vw, "");
  VW::finish_example(vw, *empty);
}

// ============================================================
// CSOAA LDF (~15 tests)
// ============================================================

TEST(CoverageMiscReductions, CsoaaLdfMcBasic)
{
  auto vw = VW::initialize(vwtest::make_args("--csoaa_ldf", "mc", "--quiet"));
  learn_csoaa_ldf_multi(*vw, "shared | s", {"1:0.5 | a b", "2:1.0 | c d"});
}

TEST(CoverageMiscReductions, CsoaaLdfMultilineBasic)
{
  auto vw = VW::initialize(vwtest::make_args("--csoaa_ldf", "m", "--quiet"));
  learn_csoaa_ldf_multi(*vw, "shared | s", {"1:0.5 | a b", "2:1.0 | c d"});
}

TEST(CoverageMiscReductions, CsoaaLdfMcThreeActions)
{
  auto vw = VW::initialize(vwtest::make_args("--csoaa_ldf", "mc", "--quiet"));
  learn_csoaa_ldf_multi(*vw, "shared | s", {"1:0.0 | a", "2:1.0 | b", "3:2.0 | c"});
}

TEST(CoverageMiscReductions, CsoaaLdfMcFiveActions)
{
  auto vw = VW::initialize(vwtest::make_args("--csoaa_ldf", "mc", "--quiet"));
  learn_csoaa_ldf_multi(*vw, "shared | s", {"1:0.0 | a", "2:0.5 | b", "3:1.0 | c", "4:1.5 | d", "5:2.0 | e"});
}

TEST(CoverageMiscReductions, CsoaaLdfMcPredictOnly)
{
  auto vw = VW::initialize(vwtest::make_args("--csoaa_ldf", "mc", "--quiet"));
  // Learn first
  learn_csoaa_ldf_multi(*vw, "shared | s", {"1:0.5 | a b", "2:1.0 | c d"});
  // Then predict (test label)
  predict_csoaa_ldf_multi(*vw, "shared | s", {"1:0.5 | a b", "2:1.0 | c d"});
}

TEST(CoverageMiscReductions, CsoaaLdfMcMultipleExamples)
{
  auto vw = VW::initialize(vwtest::make_args("--csoaa_ldf", "mc", "--quiet"));
  for (int i = 0; i < 10; i++)
  {
    learn_csoaa_ldf_multi(*vw, "shared | s:" + std::to_string(i),
        {"1:" + std::to_string(i * 0.1f) + " | a", "2:" + std::to_string(1.0f - i * 0.1f) + " | b"});
  }
}

TEST(CoverageMiscReductions, CsoaaLdfMcEqualCosts)
{
  auto vw = VW::initialize(vwtest::make_args("--csoaa_ldf", "mc", "--quiet"));
  learn_csoaa_ldf_multi(*vw, "shared | s", {"1:1.0 | a", "2:1.0 | b", "3:1.0 | c"});
}

TEST(CoverageMiscReductions, CsoaaLdfMcZeroCosts)
{
  auto vw = VW::initialize(vwtest::make_args("--csoaa_ldf", "mc", "--quiet"));
  learn_csoaa_ldf_multi(*vw, "shared | s", {"1:0.0 | a", "2:0.0 | b"});
}

TEST(CoverageMiscReductions, WapLdfBasic)
{
  auto vw = VW::initialize(vwtest::make_args("--wap_ldf", "m", "--quiet"));
  learn_csoaa_ldf_multi(*vw, "shared | s", {"1:0.5 | a b", "2:1.0 | c d"});
}

TEST(CoverageMiscReductions, WapLdfMcBasic)
{
  auto vw = VW::initialize(vwtest::make_args("--wap_ldf", "mc", "--quiet"));
  learn_csoaa_ldf_multi(*vw, "shared | s", {"1:0.5 | a b", "2:1.0 | c d"});
}

TEST(CoverageMiscReductions, WapLdfThreeActions)
{
  auto vw = VW::initialize(vwtest::make_args("--wap_ldf", "m", "--quiet"));
  learn_csoaa_ldf_multi(*vw, "shared | s", {"1:0.0 | a", "2:1.0 | b", "3:2.0 | c"});
}

TEST(CoverageMiscReductions, WapLdfFiveActions)
{
  auto vw = VW::initialize(vwtest::make_args("--wap_ldf", "m", "--quiet"));
  learn_csoaa_ldf_multi(*vw, "shared | s", {"1:0.0 | a", "2:0.5 | b", "3:1.0 | c", "4:1.5 | d", "5:2.0 | e"});
}

TEST(CoverageMiscReductions, WapLdfMultipleExamples)
{
  auto vw = VW::initialize(vwtest::make_args("--wap_ldf", "m", "--quiet"));
  for (int i = 0; i < 10; i++)
  {
    learn_csoaa_ldf_multi(*vw, "shared | s:" + std::to_string(i),
        {"1:" + std::to_string(i * 0.1f) + " | a", "2:" + std::to_string(1.0f - i * 0.1f) + " | b"});
  }
}

TEST(CoverageMiscReductions, CsoaaLdfRank)
{
  auto vw = VW::initialize(vwtest::make_args("--csoaa_ldf", "mc", "--csoaa_rank", "--quiet"));
  learn_csoaa_ldf_multi(*vw, "shared | s", {"1:0.5 | a", "2:1.0 | b", "3:0.0 | c"});
}

TEST(CoverageMiscReductions, CsoaaLdfProbabilities)
{
  auto vw = VW::initialize(
      vwtest::make_args("--csoaa_ldf", "mc", "--probabilities", "--loss_function", "logistic", "--quiet"));
  learn_csoaa_ldf_multi(*vw, "shared | s", {"1:0.5 | a", "2:1.0 | b", "3:0.0 | c"});
}

// ============================================================
// ECT (~10 tests)
// ============================================================

TEST(CoverageMiscReductions, EctBasic2Classes)
{
  auto vw = VW::initialize(vwtest::make_args("--ect", "2", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  EXPECT_GE(ex->pred.multiclass, 1u);
  EXPECT_LE(ex->pred.multiclass, 2u);
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, EctBasic3Classes)
{
  auto vw = VW::initialize(vwtest::make_args("--ect", "3", "--quiet"));
  auto* ex = VW::read_example(*vw, "2 | a b c");
  vw->learn(*ex);
  EXPECT_GE(ex->pred.multiclass, 1u);
  EXPECT_LE(ex->pred.multiclass, 3u);
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, EctBasic5Classes)
{
  auto vw = VW::initialize(vwtest::make_args("--ect", "5", "--quiet"));
  auto* ex = VW::read_example(*vw, "3 | a b c d e");
  vw->learn(*ex);
  EXPECT_GE(ex->pred.multiclass, 1u);
  EXPECT_LE(ex->pred.multiclass, 5u);
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, EctBasic10Classes)
{
  auto vw = VW::initialize(vwtest::make_args("--ect", "10", "--quiet"));
  auto* ex = VW::read_example(*vw, "7 | a b c d e");
  vw->learn(*ex);
  EXPECT_GE(ex->pred.multiclass, 1u);
  EXPECT_LE(ex->pred.multiclass, 10u);
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, EctWithErrors1)
{
  auto vw = VW::initialize(vwtest::make_args("--ect", "4", "--error", "1", "--quiet"));
  auto* ex = VW::read_example(*vw, "2 | a b c");
  vw->learn(*ex);
  EXPECT_GE(ex->pred.multiclass, 1u);
  EXPECT_LE(ex->pred.multiclass, 4u);
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, EctWithErrors2)
{
  auto vw = VW::initialize(vwtest::make_args("--ect", "5", "--error", "2", "--quiet"));
  auto* ex = VW::read_example(*vw, "3 | a b c d e");
  vw->learn(*ex);
  EXPECT_GE(ex->pred.multiclass, 1u);
  EXPECT_LE(ex->pred.multiclass, 5u);
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, EctMultipleExamples)
{
  auto vw = VW::initialize(vwtest::make_args("--ect", "4", "--quiet"));
  for (int i = 0; i < 20; i++)
  {
    uint32_t label = (i % 4) + 1;
    auto* ex = VW::read_example(*vw, std::to_string(label) + " | a:" + std::to_string(i));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageMiscReductions, EctPredictAfterTrain)
{
  auto vw = VW::initialize(vwtest::make_args("--ect", "3", "--quiet"));
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

TEST(CoverageMiscReductions, EctSingleClass)
{
  auto vw = VW::initialize(vwtest::make_args("--ect", "1", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  EXPECT_EQ(ex->pred.multiclass, 1u);
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, EctWithLogisticLink)
{
  auto vw = VW::initialize(vwtest::make_args("--ect", "3", "--link", "logistic", "--quiet"));
  auto* ex = VW::read_example(*vw, "2 | a b c");
  vw->learn(*ex);
  EXPECT_GE(ex->pred.multiclass, 1u);
  EXPECT_LE(ex->pred.multiclass, 3u);
  vw->finish_example(*ex);
}

// ============================================================
// Bootstrap (~10 tests)
// ============================================================

TEST(CoverageMiscReductions, BsBasicMean2)
{
  auto vw = VW::initialize(vwtest::make_args("--bootstrap", "2", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, BsBasicMean5)
{
  auto vw = VW::initialize(vwtest::make_args("--bootstrap", "5", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, BsBasicMean10)
{
  auto vw = VW::initialize(vwtest::make_args("--bootstrap", "10", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, BsVote2)
{
  auto vw = VW::initialize(vwtest::make_args("--bootstrap", "2", "--bs_type", "vote", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, BsVote5)
{
  auto vw = VW::initialize(vwtest::make_args("--bootstrap", "5", "--bs_type", "vote", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, BsVote10)
{
  auto vw = VW::initialize(vwtest::make_args("--bootstrap", "10", "--bs_type", "vote", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, BsMeanMultipleExamples)
{
  auto vw = VW::initialize(vwtest::make_args("--bootstrap", "3", "--quiet"));
  for (int i = 0; i < 20; i++)
  {
    float label = (i % 2 == 0) ? 1.0f : -1.0f;
    auto* ex = VW::read_example(*vw, std::to_string(label) + " | a:" + std::to_string(i * 0.1f));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageMiscReductions, BsVoteMultipleExamples)
{
  auto vw = VW::initialize(vwtest::make_args("--bootstrap", "5", "--bs_type", "vote", "--quiet"));
  for (int i = 0; i < 20; i++)
  {
    float label = (i % 2 == 0) ? 1.0f : -1.0f;
    auto* ex = VW::read_example(*vw, std::to_string(label) + " | a:" + std::to_string(i * 0.1f));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageMiscReductions, BsPredictOnly)
{
  auto vw = VW::initialize(vwtest::make_args("--bootstrap", "3", "--quiet"));
  auto* ex = VW::read_example(*vw, "| a b c");
  vw->predict(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, BsMeanWithLabel)
{
  auto vw = VW::initialize(vwtest::make_args("--bootstrap", "4", "--quiet"));
  auto* ex = VW::read_example(*vw, "0.5 | a:1 b:2 c:3");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  vw->finish_example(*ex);
}

// ============================================================
// Neural Net (~10 tests)
// ============================================================

TEST(CoverageMiscReductions, NnBasic2Units)
{
  auto vw = VW::initialize(vwtest::make_args("--nn", "2", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, NnBasic5Units)
{
  auto vw = VW::initialize(vwtest::make_args("--nn", "5", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, NnBasic10Units)
{
  auto vw = VW::initialize(vwtest::make_args("--nn", "10", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, NnWithDropout)
{
  auto vw = VW::initialize(vwtest::make_args("--nn", "3", "--dropout", "--quiet"));
  for (int i = 0; i < 10; i++)
  {
    auto* ex = VW::read_example(*vw, std::to_string(i * 0.1f) + " | a:" + std::to_string(i * 0.1f));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageMiscReductions, NnWithInpass)
{
  auto vw = VW::initialize(vwtest::make_args("--nn", "3", "--inpass", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a:1 b:2 c:3");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, NnWithMultitask)
{
  auto vw = VW::initialize(vwtest::make_args("--nn", "3", "--multitask", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a:1 b:2");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, NnWithMeanfield)
{
  auto vw = VW::initialize(vwtest::make_args("--nn", "3", "--meanfield", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a:1 b:2");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, NnMultipleExamples)
{
  auto vw = VW::initialize(vwtest::make_args("--nn", "4", "--quiet"));
  for (int i = 0; i < 20; i++)
  {
    float label = (i % 2 == 0) ? 1.0f : -1.0f;
    auto* ex = VW::read_example(*vw, std::to_string(label) + " | a:" + std::to_string(i * 0.1f));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageMiscReductions, NnPredictOnly)
{
  auto vw = VW::initialize(vwtest::make_args("--nn", "3", "--quiet"));
  auto* ex = VW::read_example(*vw, "| a b c");
  vw->predict(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, NnInpassMultipleExamples)
{
  auto vw = VW::initialize(vwtest::make_args("--nn", "3", "--inpass", "--quiet"));
  for (int i = 0; i < 15; i++)
  {
    float label = static_cast<float>(i) / 10.0f;
    auto* ex = VW::read_example(
        *vw, std::to_string(label) + " | a:" + std::to_string(i * 0.1f) + " b:" + std::to_string(1.0f - i * 0.1f));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

// ============================================================
// FTRL (~15 tests)
// ============================================================

TEST(CoverageMiscReductions, FtrlBasic)
{
  auto vw = VW::initialize(vwtest::make_args("--ftrl", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, FtrlAlphaBeta)
{
  auto vw = VW::initialize(vwtest::make_args("--ftrl", "--ftrl_alpha", "0.1", "--ftrl_beta", "0.5", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, FtrlHighAlpha)
{
  auto vw = VW::initialize(vwtest::make_args("--ftrl", "--ftrl_alpha", "5.0", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, FtrlLowAlpha)
{
  auto vw = VW::initialize(vwtest::make_args("--ftrl", "--ftrl_alpha", "0.01", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, FtrlMultipleExamples)
{
  auto vw = VW::initialize(vwtest::make_args("--ftrl", "--quiet"));
  for (int i = 0; i < 20; i++)
  {
    float label = (i % 2 == 0) ? 1.0f : -1.0f;
    auto* ex = VW::read_example(*vw, std::to_string(label) + " | a:" + std::to_string(i * 0.1f));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageMiscReductions, FtrlPredictOnly)
{
  auto vw = VW::initialize(vwtest::make_args("--ftrl", "--quiet"));
  auto* ex = VW::read_example(*vw, "| a b c");
  vw->predict(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, PistolBasic)
{
  auto vw = VW::initialize(vwtest::make_args("--pistol", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, PistolAlphaBeta)
{
  auto vw = VW::initialize(vwtest::make_args("--pistol", "--ftrl_alpha", "2.0", "--ftrl_beta", "1.0", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, PistolMultipleExamples)
{
  auto vw = VW::initialize(vwtest::make_args("--pistol", "--quiet"));
  for (int i = 0; i < 20; i++)
  {
    float label = (i % 2 == 0) ? 1.0f : -1.0f;
    auto* ex = VW::read_example(*vw, std::to_string(label) + " | a:" + std::to_string(i * 0.1f));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageMiscReductions, CoinBasic)
{
  auto vw = VW::initialize(vwtest::make_args("--coin", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, CoinAlphaBeta)
{
  auto vw = VW::initialize(vwtest::make_args("--coin", "--ftrl_alpha", "8.0", "--ftrl_beta", "2.0", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, CoinMultipleExamples)
{
  auto vw = VW::initialize(vwtest::make_args("--coin", "--quiet"));
  for (int i = 0; i < 20; i++)
  {
    float label = (i % 2 == 0) ? 1.0f : -1.0f;
    auto* ex = VW::read_example(*vw, std::to_string(label) + " | a:" + std::to_string(i * 0.1f));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageMiscReductions, FtrlWithL1)
{
  auto vw = VW::initialize(vwtest::make_args("--ftrl", "--l1", "0.01", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, FtrlWithL2)
{
  auto vw = VW::initialize(vwtest::make_args("--ftrl", "--l2", "0.01", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, FtrlWithMetrics)
{
  auto vw = VW::initialize(vwtest::make_args("--ftrl", "--extra_metrics", "/dev/null", "--quiet"));
  for (int i = 0; i < 5; i++)
  {
    auto* ex = VW::read_example(*vw, "1 | a b c");
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

// ============================================================
// Active Learning (~10 tests)
// ============================================================

TEST(CoverageMiscReductions, ActiveSimulationBasic)
{
  auto vw = VW::initialize(vwtest::make_args("--active", "--simulation", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, ActiveSimulationMultipleExamples)
{
  auto vw = VW::initialize(vwtest::make_args("--active", "--simulation", "--quiet"));
  for (int i = 0; i < 20; i++)
  {
    float label = (i % 2 == 0) ? 1.0f : -1.0f;
    auto* ex = VW::read_example(*vw, std::to_string(label) + " | a:" + std::to_string(i * 0.1f));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageMiscReductions, ActiveSimulationMellowness)
{
  auto vw = VW::initialize(vwtest::make_args("--active", "--simulation", "--mellowness", "0.5", "--quiet"));
  for (int i = 0; i < 20; i++)
  {
    float label = (i % 2 == 0) ? 1.0f : -1.0f;
    auto* ex = VW::read_example(*vw, std::to_string(label) + " | a:" + std::to_string(i * 0.1f));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageMiscReductions, ActiveSimulationHighMellowness)
{
  auto vw = VW::initialize(vwtest::make_args("--active", "--simulation", "--mellowness", "10.0", "--quiet"));
  for (int i = 0; i < 10; i++)
  {
    float label = (i % 2 == 0) ? 1.0f : -1.0f;
    auto* ex = VW::read_example(*vw, std::to_string(label) + " | a:" + std::to_string(i * 0.1f));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageMiscReductions, ActiveSimulationPredictOnly)
{
  auto vw = VW::initialize(vwtest::make_args("--active", "--simulation", "--quiet"));
  auto* ex = VW::read_example(*vw, "| a b c");
  vw->predict(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  vw->finish_example(*ex);
}

// Note: --active without --simulation or --direct triggers daemon-like network setup,
// which causes hangs in unit tests. Only simulation and direct modes are tested here.

TEST(CoverageMiscReductions, ActiveDirectModeBasic)
{
  auto vw = VW::initialize(vwtest::make_args("--active", "--direct", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, ActiveDirectModeMultipleExamples)
{
  auto vw = VW::initialize(vwtest::make_args("--active", "--direct", "--quiet"));
  for (int i = 0; i < 20; i++)
  {
    float label = (i % 2 == 0) ? 1.0f : -1.0f;
    auto* ex = VW::read_example(*vw, std::to_string(label) + " | a:" + std::to_string(i * 0.1f));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageMiscReductions, ActiveDirectModeVaryingLabels)
{
  // In direct mode, unlabeled examples with short/empty tags trigger UB
  // in active.cc (tag.begin()+6 on empty tag). Only test labeled examples here.
  auto vw = VW::initialize(vwtest::make_args("--active", "--direct", "--quiet"));
  for (int i = 0; i < 10; i++)
  {
    float label = (i % 2 == 0) ? 1.0f : -1.0f;
    auto* ex = VW::read_example(*vw, std::to_string(label) + " | a:" + std::to_string(i * 0.1f));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageMiscReductions, ActiveDirectModeQueryTag)
{
  auto vw = VW::initialize(vwtest::make_args("--active", "--direct", "--quiet"));
  // Train with a few examples first
  for (int i = 0; i < 5; i++)
  {
    auto* ex = VW::read_example(*vw, std::to_string((i % 2 == 0) ? 1.0f : -1.0f) + " | a:" + std::to_string(i));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageMiscReductions, ActiveSimulationLowMellowness)
{
  auto vw = VW::initialize(vwtest::make_args("--active", "--simulation", "--mellowness", "0.01", "--quiet"));
  for (int i = 0; i < 10; i++)
  {
    float label = (i % 2 == 0) ? 1.0f : -1.0f;
    auto* ex = VW::read_example(*vw, std::to_string(label) + " | a:" + std::to_string(i * 0.1f));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

// ============================================================
// LRQ (~10 tests)
// ============================================================

TEST(CoverageMiscReductions, LrqBasicRank1)
{
  auto vw = VW::initialize(vwtest::make_args("--lrq", "ab1", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 |a x:1 |b y:2");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, LrqBasicRank3)
{
  auto vw = VW::initialize(vwtest::make_args("--lrq", "ab3", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 |a x:1 |b y:2");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, LrqBasicRank5)
{
  auto vw = VW::initialize(vwtest::make_args("--lrq", "ab5", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 |a x:1 y:0.5 |b z:2 w:1");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, LrqWithDropout)
{
  auto vw = VW::initialize(vwtest::make_args("--lrq", "ab3", "--lrqdropout", "--quiet"));
  for (int i = 0; i < 10; i++)
  {
    auto* ex = VW::read_example(*vw,
        std::to_string((i % 2 == 0) ? 1.0f : -1.0f) + " |a x:" + std::to_string(i * 0.1f) +
            " |b y:" + std::to_string(1.0f - i * 0.1f));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageMiscReductions, LrqMultipleExamples)
{
  auto vw = VW::initialize(vwtest::make_args("--lrq", "ab2", "--quiet"));
  for (int i = 0; i < 20; i++)
  {
    auto* ex = VW::read_example(*vw,
        std::to_string(i * 0.1f) + " |a x:" + std::to_string(i * 0.1f) + " |b y:" + std::to_string(1.0f - i * 0.05f));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageMiscReductions, LrqPredictOnly)
{
  auto vw = VW::initialize(vwtest::make_args("--lrq", "ab2", "--quiet"));
  auto* ex = VW::read_example(*vw, "|a x:1 |b y:2");
  vw->predict(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, LrqMultiplePairs)
{
  auto vw = VW::initialize(vwtest::make_args("--lrq", "ab2", "--lrq", "ac2", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 |a x:1 |b y:2 |c z:3");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, LrqHighRank)
{
  auto vw = VW::initialize(vwtest::make_args("--lrq", "ab10", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 |a x:1 y:0.5 |b z:2 w:1");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, LrqNegativeLabel)
{
  auto vw = VW::initialize(vwtest::make_args("--lrq", "ab2", "--quiet"));
  auto* ex = VW::read_example(*vw, "-1 |a x:1 |b y:2");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, LrqMultipleFeaturesPerNs)
{
  auto vw = VW::initialize(vwtest::make_args("--lrq", "ab3", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 |a x:1 y:2 z:3 |b w:4 v:5 u:6");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

// ============================================================
// Interact (~10 tests)
// ============================================================

TEST(CoverageMiscReductions, InteractBasic)
{
  auto vw = VW::initialize(vwtest::make_args("--interact", "ab", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 |a 1:1 x:2 |b 1:1 y:3");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, InteractMultipleExamples)
{
  auto vw = VW::initialize(vwtest::make_args("--interact", "ab", "--quiet"));
  for (int i = 0; i < 15; i++)
  {
    float label = (i % 2 == 0) ? 1.0f : -1.0f;
    auto* ex = VW::read_example(*vw,
        std::to_string(label) + " |a 1:1 x:" + std::to_string(i * 0.1f) +
            " |b 1:1 y:" + std::to_string(1.0f - i * 0.1f));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageMiscReductions, InteractPredictOnly)
{
  auto vw = VW::initialize(vwtest::make_args("--interact", "ab", "--quiet"));
  auto* ex = VW::read_example(*vw, "|a 1:1 x:2 |b 1:1 y:3");
  vw->predict(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, InteractMissingNamespace)
{
  // When one namespace is missing, interact should still work (just passes through)
  auto vw = VW::initialize(vwtest::make_args("--interact", "ab", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 |a x:1");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, InteractThreeNamespaces)
{
  // --interact requires two different namespaces; same namespace triggers v_array assertion
  auto vw = VW::initialize(vwtest::make_args("--interact", "ab", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 |a 1:1 x:2 |b 1:1 y:3 |c z:4");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, InteractSingleFeatureEach)
{
  auto vw = VW::initialize(vwtest::make_args("--interact", "ab", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 |a 1:1 |b 1:1");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, InteractManyFeatures)
{
  auto vw = VW::initialize(vwtest::make_args("--interact", "ab", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 |a 1:1 x:1 y:2 z:3 w:4 |b 1:1 x:5 y:6 z:7 w:8");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, InteractDifferentWeights)
{
  auto vw = VW::initialize(vwtest::make_args("--interact", "ab", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 |a 1:1 x:0.5 y:0.3 |b 1:1 x:0.8 y:0.2");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, InteractNegativeLabel)
{
  auto vw = VW::initialize(vwtest::make_args("--interact", "ab", "--quiet"));
  auto* ex = VW::read_example(*vw, "-1 |a 1:1 x:1 |b 1:1 y:2");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, InteractLearnThenPredict)
{
  auto vw = VW::initialize(vwtest::make_args("--interact", "ab", "--quiet"));
  for (int i = 0; i < 10; i++)
  {
    auto* ex =
        VW::read_example(*vw, "1 |a 1:1 x:" + std::to_string(i * 0.1f) + " |b 1:1 y:" + std::to_string(i * 0.2f));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
  auto* ex = VW::read_example(*vw, "|a 1:1 x:0.5 |b 1:1 y:1.0");
  vw->predict(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  vw->finish_example(*ex);
}

// ============================================================
// Explore Eval (~10 tests)
// ============================================================

TEST(CoverageMiscReductions, ExploreEvalBasic)
{
  auto vw = VW::initialize(vwtest::make_args("--explore_eval", "--quiet"));
  learn_cb_adf_b17(*vw, "shared | s_1", {"| a_1", "| b_1"}, 0, 1.0f, 0.5f);
}

TEST(CoverageMiscReductions, ExploreEvalMultipleExamples)
{
  auto vw = VW::initialize(vwtest::make_args("--explore_eval", "--quiet"));
  for (int i = 0; i < 10; i++)
  {
    learn_cb_adf_b17(*vw, "shared | s_" + std::to_string(i), {"| a_1", "| b_1", "| c_1"}, i % 3, 1.0f, 0.33f);
  }
}

TEST(CoverageMiscReductions, ExploreEvalThreeActions)
{
  auto vw = VW::initialize(vwtest::make_args("--explore_eval", "--quiet"));
  learn_cb_adf_b17(*vw, "shared | s_1", {"| a_1", "| b_1", "| c_1"}, 0, 1.0f, 0.33f);
}

TEST(CoverageMiscReductions, ExploreEvalFixedMultiplier)
{
  auto vw = VW::initialize(vwtest::make_args("--explore_eval", "--multiplier", "0.5", "--quiet"));
  learn_cb_adf_b17(*vw, "shared | s_1", {"| a_1", "| b_1"}, 0, 1.0f, 0.5f);
}

TEST(CoverageMiscReductions, ExploreEvalFixedMultiplierMultiple)
{
  auto vw = VW::initialize(vwtest::make_args("--explore_eval", "--multiplier", "0.8", "--quiet"));
  for (int i = 0; i < 15; i++)
  {
    learn_cb_adf_b17(*vw, "shared | s_" + std::to_string(i), {"| a_1", "| b_1"}, i % 2, 1.0f, 0.5f);
  }
}

TEST(CoverageMiscReductions, ExploreEvalTargetRate)
{
  auto vw = VW::initialize(vwtest::make_args("--explore_eval", "--target_rate", "0.5", "--quiet"));
  for (int i = 0; i < 10; i++)
  {
    learn_cb_adf_b17(*vw, "shared | s_" + std::to_string(i), {"| a_1", "| b_1"}, 0, 1.0f, 0.5f);
  }
}

TEST(CoverageMiscReductions, ExploreEvalHighCost)
{
  auto vw = VW::initialize(vwtest::make_args("--explore_eval", "--quiet"));
  learn_cb_adf_b17(*vw, "shared | s_1", {"| a_1", "| b_1"}, 0, 100.0f, 0.5f);
}

TEST(CoverageMiscReductions, ExploreEvalZeroCost)
{
  auto vw = VW::initialize(vwtest::make_args("--explore_eval", "--quiet"));
  learn_cb_adf_b17(*vw, "shared | s_1", {"| a_1", "| b_1"}, 0, 0.0f, 0.5f);
}

TEST(CoverageMiscReductions, ExploreEvalManyActions)
{
  auto vw = VW::initialize(vwtest::make_args("--explore_eval", "--quiet"));
  learn_cb_adf_b17(*vw, "shared | s_1", {"| a_1", "| a_2", "| a_3", "| a_4", "| a_5"}, 0, 1.0f, 0.2f);
}

TEST(CoverageMiscReductions, ExploreEvalWithMetrics)
{
  auto vw = VW::initialize(vwtest::make_args("--explore_eval", "--extra_metrics", "/dev/null", "--quiet"));
  for (int i = 0; i < 5; i++)
  {
    learn_cb_adf_b17(*vw, "shared | s_" + std::to_string(i), {"| a_1", "| b_1"}, 0, 1.0f, 0.5f);
  }
}

// ============================================================
// Freegrad (~10 tests)
// ============================================================

TEST(CoverageMiscReductions, FreegradBasic)
{
  auto vw = VW::initialize(vwtest::make_args("--freegrad", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, FreegradMultipleExamples)
{
  auto vw = VW::initialize(vwtest::make_args("--freegrad", "--quiet"));
  for (int i = 0; i < 20; i++)
  {
    float label = (i % 2 == 0) ? 1.0f : -1.0f;
    auto* ex = VW::read_example(*vw, std::to_string(label) + " | a:" + std::to_string(i * 0.1f));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageMiscReductions, FreegradWithRestart)
{
  auto vw = VW::initialize(vwtest::make_args("--freegrad", "--restart", "--quiet"));
  for (int i = 0; i < 15; i++)
  {
    float label = (i % 2 == 0) ? 1.0f : -1.0f;
    auto* ex = VW::read_example(*vw, std::to_string(label) + " | a:" + std::to_string(i * 0.1f));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageMiscReductions, FreegradWithProject)
{
  auto vw = VW::initialize(vwtest::make_args("--freegrad", "--project", "--quiet"));
  for (int i = 0; i < 15; i++)
  {
    float label = (i % 2 == 0) ? 1.0f : -1.0f;
    auto* ex = VW::read_example(*vw, std::to_string(label) + " | a:" + std::to_string(i * 0.1f));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageMiscReductions, FreegradWithProjectAndRadius)
{
  auto vw = VW::initialize(vwtest::make_args("--freegrad", "--project", "--radius", "5.0", "--quiet"));
  for (int i = 0; i < 10; i++)
  {
    float label = static_cast<float>(i) / 5.0f;
    auto* ex = VW::read_example(*vw, std::to_string(label) + " | a:" + std::to_string(i * 0.1f));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageMiscReductions, FreegradHighEpsilon)
{
  auto vw = VW::initialize(vwtest::make_args("--freegrad", "--fepsilon", "10.0", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, FreegradLowEpsilon)
{
  auto vw = VW::initialize(vwtest::make_args("--freegrad", "--fepsilon", "0.01", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, FreegradWithLipschitz)
{
  auto vw = VW::initialize(vwtest::make_args("--freegrad", "--flipschitz_const", "1.0", "--quiet"));
  for (int i = 0; i < 10; i++)
  {
    float label = (i % 2 == 0) ? 1.0f : -1.0f;
    auto* ex = VW::read_example(*vw, std::to_string(label) + " | a:" + std::to_string(i * 0.1f));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageMiscReductions, FreegradPredictOnly)
{
  auto vw = VW::initialize(vwtest::make_args("--freegrad", "--quiet"));
  auto* ex = VW::read_example(*vw, "| a b c");
  vw->predict(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, FreegradProjectRestartCombined)
{
  auto vw = VW::initialize(vwtest::make_args("--freegrad", "--project", "--restart", "--quiet"));
  for (int i = 0; i < 20; i++)
  {
    float label = (i % 2 == 0) ? 1.0f : -1.0f;
    auto* ex = VW::read_example(
        *vw, std::to_string(label) + " | a:" + std::to_string(i * 0.1f) + " b:" + std::to_string(1.0f - i * 0.05f));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

// ============================================================
// CBZO (~10 tests)
// ============================================================

TEST(CoverageMiscReductions, CbzoConstantBasic)
{
  auto vw = VW::initialize(vwtest::make_args("--cbzo", "--policy", "constant", "--quiet"));
  auto* ex = VW::read_example(*vw, "ca 0.5:1.0:0.5 | a b c");
  vw->learn(*ex);
  EXPECT_FALSE(ex->pred.pdf.empty());
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, CbzoLinearBasic)
{
  auto vw = VW::initialize(vwtest::make_args("--cbzo", "--policy", "linear", "--quiet"));
  auto* ex = VW::read_example(*vw, "ca 0.5:1.0:0.5 | a b c");
  vw->learn(*ex);
  EXPECT_FALSE(ex->pred.pdf.empty());
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, CbzoConstantMultipleExamples)
{
  auto vw = VW::initialize(vwtest::make_args("--cbzo", "--policy", "constant", "--quiet"));
  for (int i = 0; i < 10; i++)
  {
    float action = static_cast<float>(i) / 10.0f;
    auto* ex = VW::read_example(*vw, "ca " + std::to_string(action) + ":1.0:0.5 | a:" + std::to_string(i * 0.1f));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageMiscReductions, CbzoLinearMultipleExamples)
{
  auto vw = VW::initialize(vwtest::make_args("--cbzo", "--policy", "linear", "--quiet"));
  for (int i = 0; i < 10; i++)
  {
    float action = static_cast<float>(i) / 10.0f;
    auto* ex = VW::read_example(*vw, "ca " + std::to_string(action) + ":1.0:0.5 | a:" + std::to_string(i * 0.1f));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageMiscReductions, CbzoConstantRadiusSmall)
{
  auto vw = VW::initialize(vwtest::make_args("--cbzo", "--policy", "constant", "--radius", "0.01", "--quiet"));
  auto* ex = VW::read_example(*vw, "ca 0.5:1.0:0.5 | a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, CbzoConstantRadiusLarge)
{
  auto vw = VW::initialize(vwtest::make_args("--cbzo", "--policy", "constant", "--radius", "1.0", "--quiet"));
  auto* ex = VW::read_example(*vw, "ca 0.5:1.0:0.5 | a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, CbzoLinearRadiusSmall)
{
  auto vw = VW::initialize(vwtest::make_args("--cbzo", "--policy", "linear", "--radius", "0.01", "--quiet"));
  auto* ex = VW::read_example(*vw, "ca 0.5:1.0:0.5 | a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, CbzoConstantPredictOnly)
{
  auto vw = VW::initialize(vwtest::make_args("--cbzo", "--policy", "constant", "--quiet"));
  auto* ex = VW::read_example(*vw, "| a b c");
  vw->predict(*ex);
  EXPECT_FALSE(ex->pred.pdf.empty());
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, CbzoLinearPredictOnly)
{
  auto vw = VW::initialize(vwtest::make_args("--cbzo", "--policy", "linear", "--quiet"));
  auto* ex = VW::read_example(*vw, "| a b c");
  vw->predict(*ex);
  EXPECT_FALSE(ex->pred.pdf.empty());
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, CbzoLinearWithL2)
{
  auto vw = VW::initialize(vwtest::make_args("--cbzo", "--policy", "linear", "--l2", "0.01", "--quiet"));
  for (int i = 0; i < 5; i++)
  {
    float action = static_cast<float>(i) / 5.0f;
    auto* ex = VW::read_example(*vw, "ca " + std::to_string(action) + ":1.0:0.5 | a:" + std::to_string(i * 0.1f));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

// ============================================================
// Confidence (~5 tests)
// ============================================================

TEST(CoverageMiscReductions, ConfidenceBasic)
{
  auto vw = VW::initialize(vwtest::make_args("--confidence", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  EXPECT_TRUE(std::isfinite(ex->confidence));
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, ConfidenceMultipleExamples)
{
  auto vw = VW::initialize(vwtest::make_args("--confidence", "--quiet"));
  for (int i = 0; i < 20; i++)
  {
    float label = (i % 2 == 0) ? 1.0f : -1.0f;
    auto* ex = VW::read_example(*vw, std::to_string(label) + " | a:" + std::to_string(i * 0.1f));
    vw->learn(*ex);
    EXPECT_TRUE(std::isfinite(ex->confidence));
    vw->finish_example(*ex);
  }
}

TEST(CoverageMiscReductions, ConfidenceAfterTraining)
{
  auto vw = VW::initialize(vwtest::make_args("--confidence", "--confidence_after_training", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  EXPECT_TRUE(std::isfinite(ex->confidence));
  vw->finish_example(*ex);
}

TEST(CoverageMiscReductions, ConfidenceAfterTrainingMultiple)
{
  auto vw = VW::initialize(vwtest::make_args("--confidence", "--confidence_after_training", "--quiet"));
  for (int i = 0; i < 15; i++)
  {
    float label = (i % 2 == 0) ? 1.0f : -1.0f;
    auto* ex = VW::read_example(*vw, std::to_string(label) + " | a:" + std::to_string(i * 0.1f));
    vw->learn(*ex);
    EXPECT_TRUE(std::isfinite(ex->confidence));
    vw->finish_example(*ex);
  }
}

TEST(CoverageMiscReductions, ConfidenceUnlabeled)
{
  auto vw = VW::initialize(vwtest::make_args("--confidence", "--quiet"));
  // First train with some labeled examples
  for (int i = 0; i < 5; i++)
  {
    auto* ex = VW::read_example(*vw, "1 | a:" + std::to_string(i * 0.1f));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
  // Then test with unlabeled
  auto* ex = VW::read_example(*vw, "| a:0.5");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->confidence));
  vw->finish_example(*ex);
}
