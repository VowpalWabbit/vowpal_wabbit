// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

// Batch 18: Coverage tests for memory_tree, eigen_memory_tree, plt, automl,
// epsilon_decay, ccb, cats_tree, feature_group, gen_cs_example, interactions, marginal, new_mf.

#include "vw/core/feature_group.h"
#include "vw/core/vw.h"
#include "vw/test_common/test_common.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

// ============================================================
// Helper: learn+finish a multi_ex for cb_explore_adf tests
// ============================================================
static void learn_cb_adf_b18(VW::workspace& vw, const std::string& shared, const std::vector<std::string>& actions,
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

static void predict_cb_adf_b18(VW::workspace& vw, const std::string& shared, const std::vector<std::string>& actions)
{
  VW::multi_ex multi_ex;
  multi_ex.push_back(VW::read_example(vw, shared));
  for (const auto& a : actions) { multi_ex.push_back(VW::read_example(vw, a)); }
  multi_ex.push_back(VW::read_example(vw, ""));
  vw.predict(multi_ex);
  vw.finish_example(multi_ex);
}

// Helper for CCB multi_ex learn
static void learn_ccb(VW::workspace& vw, const std::string& shared, const std::vector<std::string>& action_lines,
    const std::vector<std::string>& slot_lines)
{
  VW::multi_ex multi_ex;
  multi_ex.push_back(VW::read_example(vw, shared));
  for (const auto& a : action_lines) { multi_ex.push_back(VW::read_example(vw, a)); }
  for (const auto& s : slot_lines) { multi_ex.push_back(VW::read_example(vw, s)); }
  vw.learn(multi_ex);
  vw.finish_example(multi_ex);
}

static void predict_ccb(VW::workspace& vw, const std::string& shared, const std::vector<std::string>& action_lines,
    const std::vector<std::string>& slot_lines)
{
  VW::multi_ex multi_ex;
  multi_ex.push_back(VW::read_example(vw, shared));
  for (const auto& a : action_lines) { multi_ex.push_back(VW::read_example(vw, a)); }
  for (const auto& s : slot_lines) { multi_ex.push_back(VW::read_example(vw, s)); }
  vw.predict(multi_ex);
  vw.finish_example(multi_ex);
}

// ============================================================
// Memory Tree (~10 tests)
// ============================================================

TEST(CoverageAutomlTrees, MemoryTreeBasic)
{
  auto vw = VW::initialize(vwtest::make_args("--memory_tree", "4", "--quiet"));
  // First example in an empty tree legitimately predicts 0
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);
  // After one example, prediction should be valid
  auto* ex2 = VW::read_example(*vw, "2 | d e f");
  vw->learn(*ex2);
  vw->finish_example(*ex2);
}

TEST(CoverageAutomlTrees, MemoryTreeLargeNodes)
{
  auto vw = VW::initialize(vwtest::make_args("--memory_tree", "16", "--quiet"));
  for (int i = 0; i < 20; i++)
  {
    uint32_t label = (i % 5) + 1;
    auto* ex = VW::read_example(*vw, std::to_string(label) + " | f:" + std::to_string(i));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageAutomlTrees, MemoryTreeMaxLabels)
{
  auto vw = VW::initialize(vwtest::make_args("--memory_tree", "8", "--max_number_of_labels", "5", "--quiet"));
  for (int i = 0; i < 15; i++)
  {
    uint32_t label = (i % 5) + 1;
    auto* ex = VW::read_example(*vw, std::to_string(label) + " | a:" + std::to_string(i));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageAutomlTrees, MemoryTreeLeafMultiplier)
{
  auto vw = VW::initialize(vwtest::make_args("--memory_tree", "8", "--leaf_example_multiplier", "3", "--quiet"));
  for (int i = 0; i < 10; i++)
  {
    auto* ex = VW::read_example(*vw, std::to_string((i % 3) + 1) + " | x:" + std::to_string(i));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageAutomlTrees, MemoryTreeDreamRepeats)
{
  auto vw = VW::initialize(vwtest::make_args("--memory_tree", "8", "--dream_repeats", "3", "--quiet"));
  for (int i = 0; i < 10; i++)
  {
    auto* ex = VW::read_example(*vw, std::to_string((i % 4) + 1) + " | a b c");
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageAutomlTrees, MemoryTreeAlpha)
{
  auto vw = VW::initialize(vwtest::make_args("--memory_tree", "8", "--alpha", "0.5", "--quiet"));
  auto* ex = VW::read_example(*vw, "2 | x:1.0 y:2.0");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageAutomlTrees, MemoryTreeLearnAtLeaf)
{
  auto vw = VW::initialize(vwtest::make_args("--memory_tree", "8", "--learn_at_leaf", "--quiet"));
  for (int i = 0; i < 15; i++)
  {
    auto* ex = VW::read_example(*vw, std::to_string((i % 3) + 1) + " | f:" + std::to_string(i * 0.1));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageAutomlTrees, MemoryTreeOnline)
{
  auto vw = VW::initialize(vwtest::make_args("--memory_tree", "8", "--online", "--quiet"));
  for (int i = 0; i < 15; i++)
  {
    auto* ex = VW::read_example(*vw, std::to_string((i % 3) + 1) + " | f:" + std::to_string(i));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageAutomlTrees, MemoryTreeDreamAtUpdate)
{
  auto vw = VW::initialize(
      vwtest::make_args("--memory_tree", "8", "--dream_at_update", "1", "--dream_repeats", "2", "--quiet"));
  for (int i = 0; i < 15; i++)
  {
    auto* ex = VW::read_example(*vw, std::to_string((i % 3) + 1) + " | a b c");
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageAutomlTrees, MemoryTreeTopK)
{
  auto vw = VW::initialize(vwtest::make_args("--memory_tree", "8", "--top_K", "3", "--quiet"));
  for (int i = 0; i < 20; i++)
  {
    auto* ex = VW::read_example(*vw, std::to_string((i % 4) + 1) + " | f:" + std::to_string(i));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

// ============================================================
// Eigen Memory Tree (~10 tests)
// ============================================================

TEST(CoverageAutomlTrees, EmtBasic)
{
  auto vw = VW::initialize(vwtest::make_args("--emt", "--quiet"));
  // First example in an empty tree legitimately predicts 0
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);
  auto* ex2 = VW::read_example(*vw, "2 | d e f");
  vw->learn(*ex2);
  vw->finish_example(*ex2);
}

TEST(CoverageAutomlTrees, EmtTreeBound)
{
  auto vw = VW::initialize(vwtest::make_args("--emt", "--emt_tree", "50", "--quiet"));
  for (int i = 0; i < 20; i++)
  {
    auto* ex = VW::read_example(*vw, std::to_string((i % 5) + 1) + " | f:" + std::to_string(i));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageAutomlTrees, EmtLeafSplit)
{
  auto vw = VW::initialize(vwtest::make_args("--emt", "--emt_leaf", "10", "--quiet"));
  for (int i = 0; i < 30; i++)
  {
    auto* ex = VW::read_example(*vw, std::to_string((i % 3) + 1) + " | f:" + std::to_string(i));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageAutomlTrees, EmtScorerRandom)
{
  auto vw = VW::initialize(vwtest::make_args("--emt", "--emt_scorer", "random", "--quiet"));
  for (int i = 0; i < 15; i++)
  {
    auto* ex = VW::read_example(*vw, std::to_string((i % 3) + 1) + " | a b c");
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageAutomlTrees, EmtScorerDistance)
{
  auto vw = VW::initialize(vwtest::make_args("--emt", "--emt_scorer", "distance", "--quiet"));
  for (int i = 0; i < 15; i++)
  {
    auto* ex = VW::read_example(*vw, std::to_string((i % 3) + 1) + " | x:" + std::to_string(i * 0.1));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageAutomlTrees, EmtScorerNotSelfConsistent)
{
  auto vw = VW::initialize(vwtest::make_args("--emt", "--emt_scorer", "not_self_consistent_rank", "--quiet"));
  for (int i = 0; i < 15; i++)
  {
    auto* ex = VW::read_example(*vw, std::to_string((i % 4) + 1) + " | a:" + std::to_string(i));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageAutomlTrees, EmtRouterRandom)
{
  auto vw = VW::initialize(vwtest::make_args("--emt", "--emt_router", "random", "--quiet"));
  for (int i = 0; i < 15; i++)
  {
    auto* ex = VW::read_example(*vw, std::to_string((i % 3) + 1) + " | a b c");
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageAutomlTrees, EmtInitialGaussian)
{
  auto vw = VW::initialize(vwtest::make_args("--emt", "--emt_initial", "gaussian", "--quiet"));
  for (int i = 0; i < 15; i++)
  {
    auto* ex = VW::read_example(*vw, std::to_string((i % 3) + 1) + " | x:" + std::to_string(i));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageAutomlTrees, EmtInitialEuclidean)
{
  auto vw = VW::initialize(vwtest::make_args("--emt", "--emt_initial", "euclidean", "--quiet"));
  for (int i = 0; i < 15; i++)
  {
    auto* ex = VW::read_example(*vw, std::to_string((i % 3) + 1) + " | x:" + std::to_string(i));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageAutomlTrees, EmtInitialNone)
{
  auto vw = VW::initialize(vwtest::make_args("--emt", "--emt_initial", "none", "--quiet"));
  for (int i = 0; i < 15; i++)
  {
    auto* ex = VW::read_example(*vw, std::to_string((i % 3) + 1) + " | a b c");
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

// ============================================================
// PLT (~5 tests)
// ============================================================

TEST(CoverageAutomlTrees, PltBasic)
{
  auto vw = VW::initialize(vwtest::make_args("--plt", "5", "--loss_function", "logistic", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageAutomlTrees, PltLargeK)
{
  auto vw = VW::initialize(vwtest::make_args("--plt", "20", "--loss_function", "logistic", "--quiet"));
  for (int i = 0; i < 10; i++)
  {
    uint32_t label = (i % 20) + 1;
    auto* ex = VW::read_example(*vw, std::to_string(label) + " | f:" + std::to_string(i));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageAutomlTrees, PltKaryTree)
{
  auto vw =
      VW::initialize(vwtest::make_args("--plt", "10", "--kary_tree", "4", "--loss_function", "logistic", "--quiet"));
  for (int i = 0; i < 10; i++)
  {
    auto* ex = VW::read_example(*vw, std::to_string((i % 10) + 1) + " | a b c");
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageAutomlTrees, PltThreshold)
{
  auto vw =
      VW::initialize(vwtest::make_args("--plt", "5", "--threshold", "0.3", "--loss_function", "logistic", "--quiet"));
  for (int i = 0; i < 10; i++)
  {
    auto* ex = VW::read_example(*vw, std::to_string((i % 5) + 1) + " | f:" + std::to_string(i));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageAutomlTrees, PltTopK)
{
  auto vw = VW::initialize(vwtest::make_args("--plt", "10", "--top_k", "3", "--loss_function", "logistic", "--quiet"));
  for (int i = 0; i < 10; i++)
  {
    auto* ex = VW::read_example(*vw, std::to_string((i % 10) + 1) + " | a b c d");
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

// ============================================================
// AutoML (~10 tests)
// ============================================================

TEST(CoverageAutomlTrees, AutomlBasic)
{
  auto vw = VW::initialize(vwtest::make_args(
      "--automl", "3", "--priority_type", "none", "--cb_explore_adf", "--quiet", "--random_seed", "5"));
  // Use prob=1.0 so IPS-weighted reward stays in [0,1] for confidence_sequence_robust
  learn_cb_adf_b18(*vw, "shared | s_1", {"| a_1", "| b_1"}, 0, -0.5f, 1.0f);
}

TEST(CoverageAutomlTrees, AutomlFourConfigs)
{
  auto vw = VW::initialize(vwtest::make_args("--automl", "4", "--cb_explore_adf", "--quiet", "--random_seed", "5"));
  for (int i = 0; i < 10; i++)
  {
    learn_cb_adf_b18(*vw, "shared | s_" + std::to_string(i), {"| a_1", "| b_1", "| c_1"}, i % 3, -0.5f, 1.0f);
  }
}

TEST(CoverageAutomlTrees, AutomlOracleOneDiff)
{
  auto vw = VW::initialize(vwtest::make_args(
      "--automl", "3", "--oracle_type", "one_diff", "--cb_explore_adf", "--quiet", "--random_seed", "5"));
  for (int i = 0; i < 10; i++)
  {
    learn_cb_adf_b18(*vw, "shared | s_" + std::to_string(i), {"| a_1", "| b_1"}, i % 2, -0.5f, 1.0f);
  }
}

TEST(CoverageAutomlTrees, AutomlOracleRand)
{
  auto vw = VW::initialize(
      vwtest::make_args("--automl", "3", "--oracle_type", "rand", "--cb_explore_adf", "--quiet", "--random_seed", "5"));
  for (int i = 0; i < 10; i++)
  {
    learn_cb_adf_b18(*vw, "shared | s_" + std::to_string(i), {"| a_1", "| b_1"}, i % 2, -0.5f, 1.0f);
  }
}

TEST(CoverageAutomlTrees, AutomlOracleChampdupe)
{
  auto vw = VW::initialize(vwtest::make_args(
      "--automl", "3", "--oracle_type", "champdupe", "--cb_explore_adf", "--quiet", "--random_seed", "5"));
  for (int i = 0; i < 10; i++)
  {
    learn_cb_adf_b18(*vw, "shared | s_" + std::to_string(i), {"| a_1", "| b_1"}, i % 2, -0.5f, 1.0f);
  }
}

TEST(CoverageAutomlTrees, AutomlOracleOneDiffInclusion)
{
  auto vw = VW::initialize(vwtest::make_args(
      "--automl", "3", "--oracle_type", "one_diff_inclusion", "--cb_explore_adf", "--quiet", "--random_seed", "5"));
  for (int i = 0; i < 10; i++)
  {
    learn_cb_adf_b18(*vw, "shared | s_" + std::to_string(i), {"| a_1", "| b_1"}, i % 2, -0.5f, 1.0f);
  }
}

TEST(CoverageAutomlTrees, AutomlInteractionTypeCubic)
{
  auto vw = VW::initialize(vwtest::make_args(
      "--automl", "3", "--interaction_type", "cubic", "--cb_explore_adf", "--quiet", "--random_seed", "5"));
  for (int i = 0; i < 10; i++)
  {
    learn_cb_adf_b18(*vw, "shared | s_" + std::to_string(i), {"| a_1", "| b_1"}, i % 2, -0.5f, 1.0f);
  }
}

TEST(CoverageAutomlTrees, AutomlDefaultLease)
{
  auto vw = VW::initialize(vwtest::make_args(
      "--automl", "3", "--default_lease", "100", "--cb_explore_adf", "--quiet", "--random_seed", "5"));
  for (int i = 0; i < 10; i++)
  {
    learn_cb_adf_b18(*vw, "shared | s_" + std::to_string(i), {"| a_1", "| b_1"}, i % 2, -0.5f, 1.0f);
  }
}

TEST(CoverageAutomlTrees, AutomlFixedSignificanceLevel)
{
  auto vw = VW::initialize(vwtest::make_args(
      "--automl", "3", "--fixed_significance_level", "--cb_explore_adf", "--quiet", "--random_seed", "5"));
  for (int i = 0; i < 10; i++)
  {
    learn_cb_adf_b18(*vw, "shared | s_" + std::to_string(i), {"| a_1", "| b_1"}, i % 2, -0.5f, 1.0f);
  }
}

TEST(CoverageAutomlTrees, AutomlPredictAfterLearn)
{
  auto vw = VW::initialize(vwtest::make_args("--automl", "3", "--cb_explore_adf", "--quiet", "--random_seed", "5"));
  for (int i = 0; i < 10; i++)
  {
    learn_cb_adf_b18(*vw, "shared | s_" + std::to_string(i), {"| a_1", "| b_1"}, i % 2, -0.5f, 1.0f);
  }
  predict_cb_adf_b18(*vw, "shared | s_test", {"| a_1", "| b_1"});
}

// ============================================================
// Epsilon Decay (~5 tests)
// ============================================================

TEST(CoverageAutomlTrees, EpsilonDecayBasic)
{
  auto vw = VW::initialize(
      vwtest::make_args("--epsilon_decay", "--model_count", "3", "--cb_explore_adf", "--quiet", "--random_seed", "5"));
  learn_cb_adf_b18(*vw, "shared | s_1", {"| a_1", "| b_1"}, 0, -0.5f, 1.0f);
}

TEST(CoverageAutomlTrees, EpsilonDecayModelCount5)
{
  auto vw = VW::initialize(
      vwtest::make_args("--epsilon_decay", "--model_count", "5", "--cb_explore_adf", "--quiet", "--random_seed", "5"));
  for (int i = 0; i < 15; i++)
  {
    learn_cb_adf_b18(*vw, "shared | s_" + std::to_string(i), {"| a_1", "| b_1", "| c_1"}, i % 3, -0.5f, 1.0f);
  }
}

TEST(CoverageAutomlTrees, EpsilonDecayMinScope)
{
  auto vw = VW::initialize(vwtest::make_args("--epsilon_decay", "--model_count", "3", "--min_scope", "10",
      "--cb_explore_adf", "--quiet", "--random_seed", "5"));
  for (int i = 0; i < 20; i++)
  {
    learn_cb_adf_b18(*vw, "shared | s_" + std::to_string(i), {"| a_1", "| b_1"}, i % 2, -0.5f, 1.0f);
  }
}

TEST(CoverageAutomlTrees, EpsilonDecayConstantEpsilon)
{
  auto vw = VW::initialize(vwtest::make_args("--epsilon_decay", "--model_count", "3", "--constant_epsilon",
      "--cb_explore_adf", "--quiet", "--random_seed", "5"));
  for (int i = 0; i < 10; i++)
  {
    learn_cb_adf_b18(*vw, "shared | s_" + std::to_string(i), {"| a_1", "| b_1"}, i % 2, -0.5f, 1.0f);
  }
}

TEST(CoverageAutomlTrees, EpsilonDecayPredict)
{
  auto vw = VW::initialize(
      vwtest::make_args("--epsilon_decay", "--model_count", "3", "--cb_explore_adf", "--quiet", "--random_seed", "5"));
  for (int i = 0; i < 10; i++)
  {
    learn_cb_adf_b18(*vw, "shared | s_" + std::to_string(i), {"| a_1", "| b_1"}, i % 2, -0.5f, 1.0f);
  }
  predict_cb_adf_b18(*vw, "shared | s_test", {"| a_1", "| b_1"});
}

// ============================================================
// CCB (~10 tests)
// ============================================================

TEST(CoverageAutomlTrees, CcbBasicPredict)
{
  auto vw = VW::initialize(vwtest::make_args("--ccb_explore_adf", "--quiet"));
  predict_ccb(*vw, "ccb shared | s", {"ccb action | a1", "ccb action | a2"}, {"ccb slot | slot1"});
}

TEST(CoverageAutomlTrees, CcbBasicLearn)
{
  auto vw = VW::initialize(vwtest::make_args("--ccb_explore_adf", "--quiet"));
  learn_ccb(*vw, "ccb shared | s", {"ccb action | a1", "ccb action | a2"}, {"ccb slot 0:0.5:0.5 | slot1"});
}

TEST(CoverageAutomlTrees, CcbTwoSlots)
{
  auto vw = VW::initialize(vwtest::make_args("--ccb_explore_adf", "--quiet"));
  learn_ccb(*vw, "ccb shared | s", {"ccb action | a1", "ccb action | a2", "ccb action | a3"},
      {"ccb slot 0:0.5:0.5 | slot1", "ccb slot 1:0.3:0.5 | slot2"});
}

TEST(CoverageAutomlTrees, CcbThreeSlots)
{
  auto vw = VW::initialize(vwtest::make_args("--ccb_explore_adf", "--quiet"));
  learn_ccb(*vw, "ccb shared | s", {"ccb action | a1", "ccb action | a2", "ccb action | a3", "ccb action | a4"},
      {"ccb slot 0:1.0:0.25 | s1", "ccb slot 1:0.5:0.25 | s2", "ccb slot 2:0.0:0.25 | s3"});
}

TEST(CoverageAutomlTrees, CcbMultipleExamples)
{
  auto vw = VW::initialize(vwtest::make_args("--ccb_explore_adf", "--quiet"));
  for (int i = 0; i < 10; i++)
  {
    learn_ccb(*vw, "ccb shared | s_" + std::to_string(i), {"ccb action | a1", "ccb action | a2"},
        {"ccb slot 0:0.5:0.5 | slot1"});
  }
}

TEST(CoverageAutomlTrees, CcbExplicitIncludedActions)
{
  auto vw = VW::initialize(vwtest::make_args("--ccb_explore_adf", "--quiet"));
  VW::multi_ex multi_ex;
  multi_ex.push_back(VW::read_example(*vw, "ccb shared | s"));
  multi_ex.push_back(VW::read_example(*vw, "ccb action | a1"));
  multi_ex.push_back(VW::read_example(*vw, "ccb action | a2"));
  multi_ex.push_back(VW::read_example(*vw, "ccb action | a3"));
  multi_ex.push_back(VW::read_example(*vw, "ccb slot 0,1 | slot1"));
  vw->predict(multi_ex);
  auto& decision_scores = multi_ex[0]->pred.decision_scores;
  EXPECT_EQ(decision_scores.size(), 1u);
  vw->finish_example(multi_ex);
}

TEST(CoverageAutomlTrees, CcbWithEpsilon)
{
  auto vw = VW::initialize(vwtest::make_args("--ccb_explore_adf", "--epsilon", "0.2", "--quiet"));
  for (int i = 0; i < 10; i++)
  {
    learn_ccb(*vw, "ccb shared | s_" + std::to_string(i), {"ccb action | a1", "ccb action | a2", "ccb action | a3"},
        {"ccb slot 0:0.5:0.33 | slot1", "ccb slot 1:1.0:0.33 | slot2"});
  }
}

TEST(CoverageAutomlTrees, CcbNoSlotIndex)
{
  auto vw = VW::initialize(vwtest::make_args("--ccb_explore_adf", "--ccb_no_slot_index", "--quiet"));
  learn_ccb(*vw, "ccb shared | s", {"ccb action | a1", "ccb action | a2"}, {"ccb slot 0:0.5:0.5 | slot1"});
}

TEST(CoverageAutomlTrees, CcbAllSlotsLoss)
{
  auto vw = VW::initialize(vwtest::make_args("--ccb_explore_adf", "--all_slots_loss", "--quiet"));
  for (int i = 0; i < 5; i++)
  {
    learn_ccb(*vw, "ccb shared | s_" + std::to_string(i), {"ccb action | a1", "ccb action | a2"},
        {"ccb slot 0:0.5:0.5 | slot1"});
  }
}

TEST(CoverageAutomlTrees, CcbManyActions)
{
  auto vw = VW::initialize(vwtest::make_args("--ccb_explore_adf", "--quiet"));
  learn_ccb(*vw, "ccb shared | s",
      {"ccb action | a1", "ccb action | a2", "ccb action | a3", "ccb action | a4", "ccb action | a5"},
      {"ccb slot 0:0.5:0.2 | slot1"});
}

// ============================================================
// CATS Tree (~5 tests)
// ============================================================

TEST(CoverageAutomlTrees, CatsTreeBasic)
{
  auto vw = VW::initialize(vwtest::make_args("--cats_tree", "4", "--quiet"));
  auto* ex = VW::read_example(*vw, "1:1.0:0.5 | a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageAutomlTrees, CatsTreeBandwidth)
{
  auto vw = VW::initialize(vwtest::make_args("--cats_tree", "8", "--tree_bandwidth", "2", "--quiet"));
  auto* ex = VW::read_example(*vw, "3:1.0:0.5 | a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageAutomlTrees, CatsTreeLargeK)
{
  auto vw = VW::initialize(vwtest::make_args("--cats_tree", "16", "--quiet"));
  for (int i = 0; i < 10; i++)
  {
    uint32_t action = (i % 16) + 1;
    auto* ex = VW::read_example(*vw, std::to_string(action) + ":1.0:0.5 | f:" + std::to_string(i));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageAutomlTrees, CatsTreeBandwidth4)
{
  auto vw = VW::initialize(vwtest::make_args("--cats_tree", "8", "--tree_bandwidth", "4", "--quiet"));
  for (int i = 0; i < 10; i++)
  {
    uint32_t action = (i % 8) + 1;
    auto* ex = VW::read_example(*vw, std::to_string(action) + ":0.5:0.25 | a b c");
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageAutomlTrees, CatsTreePredict)
{
  auto vw = VW::initialize(vwtest::make_args("--cats_tree", "4", "--quiet"));
  // learn first
  for (int i = 0; i < 10; i++)
  {
    auto* ex = VW::read_example(*vw, std::to_string((i % 4) + 1) + ":1.0:0.5 | a b c");
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
  // predict
  auto* ex = VW::read_example(*vw, "| a b c");
  vw->predict(*ex);
  EXPECT_GE(ex->pred.multiclass, 1u);
  EXPECT_LE(ex->pred.multiclass, 4u);
  vw->finish_example(*ex);
}

// ============================================================
// Feature Group (~10 tests)
// ============================================================

TEST(CoverageAutomlTrees, FeatureGroupPushBack)
{
  VW::features f;
  f.push_back(1.0f, 10);
  f.push_back(2.0f, 20);
  f.push_back(3.0f, 30);
  EXPECT_EQ(f.size(), 3u);
  EXPECT_FLOAT_EQ(f.sum_feat_sq, 1.0f + 4.0f + 9.0f);
}

TEST(CoverageAutomlTrees, FeatureGroupPushBackWithHash)
{
  VW::features f;
  f.push_back(1.0f, 10, 100);
  f.push_back(2.0f, 20, 100);
  f.push_back(3.0f, 30, 200);
  EXPECT_EQ(f.size(), 3u);
  EXPECT_EQ(f.namespace_extents.size(), 2u);
}

TEST(CoverageAutomlTrees, FeatureGroupClear)
{
  VW::features f;
  f.push_back(1.0f, 10);
  f.push_back(2.0f, 20);
  EXPECT_EQ(f.size(), 2u);
  f.clear();
  EXPECT_EQ(f.size(), 0u);
  EXPECT_FLOAT_EQ(f.sum_feat_sq, 0.0f);
}

TEST(CoverageAutomlTrees, FeatureGroupTruncateToSize)
{
  VW::features f;
  f.push_back(1.0f, 10);
  f.push_back(2.0f, 20);
  f.push_back(3.0f, 30);
  f.truncate_to(static_cast<size_t>(1));
  EXPECT_EQ(f.size(), 1u);
}

TEST(CoverageAutomlTrees, FeatureGroupTruncateWithSumFeatSq)
{
  VW::features f;
  f.push_back(1.0f, 10);
  f.push_back(2.0f, 20);
  f.push_back(3.0f, 30);
  // Remove elements at index 2 and beyond: sum_feat_sq of removed = 9.0
  f.truncate_to(static_cast<size_t>(2), 9.0f);
  EXPECT_EQ(f.size(), 2u);
  EXPECT_FLOAT_EQ(f.sum_feat_sq, 1.0f + 4.0f);
}

TEST(CoverageAutomlTrees, FeatureGroupTruncateToIterator)
{
  VW::features f;
  f.push_back(1.0f, 10);
  f.push_back(2.0f, 20);
  f.push_back(3.0f, 30);
  auto it = f.begin();
  ++it;  // points to element at index 1
  f.truncate_to(it);
  EXPECT_EQ(f.size(), 1u);
}

TEST(CoverageAutomlTrees, FeatureGroupConcat)
{
  VW::features f1;
  f1.push_back(1.0f, 10);
  f1.push_back(2.0f, 20);

  VW::features f2;
  f2.push_back(3.0f, 30);
  f2.push_back(4.0f, 40);

  f1.concat(f2);
  EXPECT_EQ(f1.size(), 4u);
  EXPECT_FLOAT_EQ(f1.sum_feat_sq, 1.0f + 4.0f + 9.0f + 16.0f);
}

TEST(CoverageAutomlTrees, FeatureGroupSort)
{
  VW::features f;
  f.push_back(3.0f, 30);
  f.push_back(1.0f, 10);
  f.push_back(2.0f, 20);
  uint64_t mask = ~static_cast<uint64_t>(0);  // no masking
  bool sorted = f.sort(mask);
  EXPECT_TRUE(sorted);
  EXPECT_EQ(f.indices[0], 10u);
  EXPECT_EQ(f.indices[1], 20u);
  EXPECT_EQ(f.indices[2], 30u);
}

TEST(CoverageAutomlTrees, FeatureGroupSortEmpty)
{
  VW::features f;
  uint64_t mask = ~static_cast<uint64_t>(0);
  bool sorted = f.sort(mask);
  EXPECT_FALSE(sorted);
}

TEST(CoverageAutomlTrees, FeatureGroupStartEndExtent)
{
  VW::features f;
  f.start_ns_extent(100);
  f.push_back(1.0f, 10);
  f.push_back(2.0f, 20);
  f.end_ns_extent();
  EXPECT_EQ(f.namespace_extents.size(), 1u);
  EXPECT_EQ(f.namespace_extents[0].begin_index, 0u);
  EXPECT_EQ(f.namespace_extents[0].end_index, 2u);
  EXPECT_EQ(f.namespace_extents[0].hash, 100u);
}

TEST(CoverageAutomlTrees, FeatureGroupDotProduct)
{
  VW::features f1;
  f1.push_back(1.0f, 10);
  f1.push_back(2.0f, 20);
  f1.push_back(3.0f, 30);

  VW::features f2;
  f2.push_back(4.0f, 10);
  f2.push_back(5.0f, 20);
  f2.push_back(6.0f, 30);

  float dp = VW::features_dot_product(f1, f2);
  EXPECT_FLOAT_EQ(dp, 1.0f * 4.0f + 2.0f * 5.0f + 3.0f * 6.0f);
}

TEST(CoverageAutomlTrees, FeatureGroupDotProductEmpty)
{
  VW::features f1;
  VW::features f2;
  float dp = VW::features_dot_product(f1, f2);
  EXPECT_FLOAT_EQ(dp, 0.0f);
}

// ============================================================
// Gen CS Example / CB Type paths (~10 tests)
// ============================================================

TEST(CoverageAutomlTrees, CbExploreAdfDrCbType)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--cb_type", "dr", "--quiet"));
  for (int i = 0; i < 10; i++)
  {
    learn_cb_adf_b18(*vw, "shared | s_" + std::to_string(i), {"| a_1", "| b_1", "| c_1"}, i % 3, 1.0f, 0.33f);
  }
}

TEST(CoverageAutomlTrees, CbExploreAdfDmCbType)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--cb_type", "dm", "--quiet"));
  for (int i = 0; i < 10; i++)
  {
    learn_cb_adf_b18(*vw, "shared | s_" + std::to_string(i), {"| a_1", "| b_1", "| c_1"}, i % 3, 1.0f, 0.33f);
  }
}

TEST(CoverageAutomlTrees, CbExploreAdfIpsCbType)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--cb_type", "ips", "--quiet"));
  for (int i = 0; i < 10; i++)
  {
    learn_cb_adf_b18(*vw, "shared | s_" + std::to_string(i), {"| a_1", "| b_1"}, i % 2, 1.0f, 0.5f);
  }
}

TEST(CoverageAutomlTrees, CbExploreAdfSmCbType)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--cb_type", "sm", "--quiet"));
  for (int i = 0; i < 10; i++)
  {
    learn_cb_adf_b18(*vw, "shared | s_" + std::to_string(i), {"| a_1", "| b_1"}, i % 2, 1.0f, 0.5f);
  }
}

TEST(CoverageAutomlTrees, CbExploreAdfMtrCbType)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--cb_type", "mtr", "--quiet"));
  for (int i = 0; i < 10; i++)
  {
    learn_cb_adf_b18(*vw, "shared | s_" + std::to_string(i), {"| a_1", "| b_1"}, i % 2, 1.0f, 0.5f);
  }
}

TEST(CoverageAutomlTrees, CbExploreAdfDrZeroCost)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--cb_type", "dr", "--quiet"));
  learn_cb_adf_b18(*vw, "shared | s_1", {"| a_1", "| b_1", "| c_1"}, 0, 0.0f, 0.33f);
}

TEST(CoverageAutomlTrees, CbExploreAdfDrHighCost)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--cb_type", "dr", "--quiet"));
  learn_cb_adf_b18(*vw, "shared | s_1", {"| a_1", "| b_1"}, 0, 100.0f, 0.5f);
}

TEST(CoverageAutomlTrees, CbExploreAdfDrManyActions)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--cb_type", "dr", "--quiet"));
  learn_cb_adf_b18(*vw, "shared | s_1", {"| a_1", "| a_2", "| a_3", "| a_4", "| a_5"}, 0, 1.0f, 0.2f);
}

TEST(CoverageAutomlTrees, CbExploreAdfDmManyExamples)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--cb_type", "dm", "--quiet"));
  for (int i = 0; i < 20; i++)
  {
    learn_cb_adf_b18(*vw, "shared | s_" + std::to_string(i), {"| a_1", "| b_1"}, i % 2, 1.0f, 0.5f);
  }
}

TEST(CoverageAutomlTrees, CbExploreAdfDrPredictAfterTrain)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--cb_type", "dr", "--quiet"));
  for (int i = 0; i < 10; i++)
  {
    learn_cb_adf_b18(*vw, "shared | s_" + std::to_string(i), {"| a_1", "| b_1"}, i % 2, 1.0f, 0.5f);
  }
  predict_cb_adf_b18(*vw, "shared | s_test", {"| a_1", "| b_1"});
}

// ============================================================
// Additional Misc (~15 tests)
// ============================================================

// Marginal
TEST(CoverageAutomlTrees, MarginalBasic)
{
  auto vw = VW::initialize(vwtest::make_args("--marginal", "f", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 |f a b c");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  vw->finish_example(*ex);
}

TEST(CoverageAutomlTrees, MarginalMultipleExamples)
{
  auto vw = VW::initialize(vwtest::make_args("--marginal", "f", "--quiet"));
  for (int i = 0; i < 20; i++)
  {
    float label = (i % 2 == 0) ? 1.0f : -1.0f;
    auto* ex = VW::read_example(*vw, std::to_string(label) + " |f a b c");
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageAutomlTrees, MarginalCompete)
{
  auto vw = VW::initialize(vwtest::make_args("--marginal", "f", "--compete", "--quiet"));
  for (int i = 0; i < 10; i++)
  {
    auto* ex = VW::read_example(*vw, "1 |f a b c");
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageAutomlTrees, MarginalInitialDenominator)
{
  auto vw = VW::initialize(
      vwtest::make_args("--marginal", "f", "--initial_denominator", "2.0", "--initial_numerator", "1.0", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 |f a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

// New MF
TEST(CoverageAutomlTrees, NewMfBasic)
{
  auto vw = VW::initialize(vwtest::make_args("--new_mf", "2", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 |u user1 |i item1");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageAutomlTrees, NewMfRank5)
{
  auto vw = VW::initialize(vwtest::make_args("--new_mf", "5", "--quiet"));
  for (int i = 0; i < 10; i++)
  {
    auto* ex = VW::read_example(*vw,
        std::to_string(static_cast<float>(i % 5)) + " |u user" + std::to_string(i % 3) + " |i item" +
            std::to_string(i % 4));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

// Interactions: quadratic and cubic
TEST(CoverageAutomlTrees, QuadraticInteraction)
{
  auto vw = VW::initialize(vwtest::make_args("-q", "ab", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 |a x:1.0 |b y:2.0");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  vw->finish_example(*ex);
}

TEST(CoverageAutomlTrees, CubicInteraction)
{
  auto vw = VW::initialize(vwtest::make_args("--cubic", "abc", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 |a x:1.0 |b y:2.0 |c z:3.0");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  vw->finish_example(*ex);
}

TEST(CoverageAutomlTrees, QuadraticWildcard)
{
  auto vw = VW::initialize(vwtest::make_args("-q", "::", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 |a x:1.0 |b y:2.0");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  vw->finish_example(*ex);
}

TEST(CoverageAutomlTrees, MultipleQuadraticInteractions)
{
  auto vw = VW::initialize(vwtest::make_args("-q", "ab", "-q", "bc", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 |a x:1.0 |b y:2.0 |c z:3.0");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

// Memory tree with OAS (multi-label)
TEST(CoverageAutomlTrees, MemoryTreeOas)
{
  auto vw = VW::initialize(vwtest::make_args("--memory_tree", "8", "--oas", "--quiet"));
  // multilabel format: each label on its own
  auto* ex = VW::read_example(*vw, "1,2,3 | a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

// CCB with cb_type IPS
TEST(CoverageAutomlTrees, CcbCbTypeIps)
{
  auto vw = VW::initialize(vwtest::make_args("--ccb_explore_adf", "--cb_type", "ips", "--quiet"));
  learn_ccb(*vw, "ccb shared | s", {"ccb action | a1", "ccb action | a2"}, {"ccb slot 0:0.5:0.5 | slot1"});
}

// CCB with cb_type DR
TEST(CoverageAutomlTrees, CcbCbTypeDr)
{
  auto vw = VW::initialize(vwtest::make_args("--ccb_explore_adf", "--cb_type", "dr", "--quiet"));
  for (int i = 0; i < 5; i++)
  {
    learn_ccb(*vw, "ccb shared | s_" + std::to_string(i), {"ccb action | a1", "ccb action | a2"},
        {"ccb slot 0:0.5:0.5 | slot1"});
  }
}

// Feature group: concat with extents
TEST(CoverageAutomlTrees, FeatureGroupConcatWithExtents)
{
  VW::features f1;
  f1.start_ns_extent(100);
  f1.push_back(1.0f, 10);
  f1.push_back(2.0f, 20);
  f1.end_ns_extent();

  VW::features f2;
  f2.start_ns_extent(200);
  f2.push_back(3.0f, 30);
  f2.end_ns_extent();

  f1.concat(f2);
  EXPECT_EQ(f1.size(), 3u);
  EXPECT_EQ(f1.namespace_extents.size(), 2u);
}

// Feature group: concat with same hash merges extents
TEST(CoverageAutomlTrees, FeatureGroupConcatSameHash)
{
  VW::features f1;
  f1.start_ns_extent(100);
  f1.push_back(1.0f, 10);
  f1.end_ns_extent();

  VW::features f2;
  f2.start_ns_extent(100);
  f2.push_back(2.0f, 20);
  f2.end_ns_extent();

  f1.concat(f2);
  EXPECT_EQ(f1.size(), 2u);
  // Same hash extents should be merged
  EXPECT_EQ(f1.namespace_extents.size(), 1u);
  EXPECT_EQ(f1.namespace_extents[0].end_index, 2u);
}

// Feature group: truncate with namespace extents
TEST(CoverageAutomlTrees, FeatureGroupTruncateWithExtents)
{
  VW::features f;
  f.start_ns_extent(100);
  f.push_back(1.0f, 10);
  f.push_back(2.0f, 20);
  f.end_ns_extent();
  f.start_ns_extent(200);
  f.push_back(3.0f, 30);
  f.end_ns_extent();

  f.truncate_to(static_cast<size_t>(2), 9.0f);
  EXPECT_EQ(f.size(), 2u);
  EXPECT_EQ(f.namespace_extents.size(), 1u);
}

// Feature group: sort with namespace extents
TEST(CoverageAutomlTrees, FeatureGroupSortWithExtents)
{
  VW::features f;
  f.start_ns_extent(100);
  f.push_back(3.0f, 30);
  f.push_back(1.0f, 10);
  f.end_ns_extent();
  uint64_t mask = ~static_cast<uint64_t>(0);
  bool sorted = f.sort(mask);
  EXPECT_TRUE(sorted);
  EXPECT_EQ(f.indices[0], 10u);
  EXPECT_EQ(f.indices[1], 30u);
}

// Validate extents
TEST(CoverageAutomlTrees, FeatureGroupValidateExtents)
{
  VW::features f;
  f.start_ns_extent(100);
  f.push_back(1.0f, 10);
  f.push_back(2.0f, 20);
  f.end_ns_extent();
  EXPECT_TRUE(f.validate_extents());
}

// Feature group: end_ns_extent merges consecutive same-hash extents
TEST(CoverageAutomlTrees, FeatureGroupEndExtentMerge)
{
  VW::features f;
  f.start_ns_extent(100);
  f.push_back(1.0f, 10);
  f.end_ns_extent();
  f.start_ns_extent(100);
  f.push_back(2.0f, 20);
  f.end_ns_extent();
  // Should merge into one extent
  EXPECT_EQ(f.namespace_extents.size(), 1u);
  EXPECT_EQ(f.namespace_extents[0].end_index, 2u);
}

// Push_back with hash - different hash closes previous open extent
TEST(CoverageAutomlTrees, FeatureGroupPushBackHashClosesExtent)
{
  VW::features f;
  f.start_ns_extent(100);
  f.push_back(1.0f, 10);
  f.end_ns_extent();
  // push_back with a different hash
  f.push_back(2.0f, 20, 200);
  EXPECT_EQ(f.namespace_extents.size(), 2u);
  EXPECT_EQ(f.namespace_extents[1].hash, 200u);
}

// Automl with priority_challengers
TEST(CoverageAutomlTrees, AutomlPriorityChallengers)
{
  auto vw = VW::initialize(vwtest::make_args(
      "--automl", "4", "--priority_challengers", "1", "--cb_explore_adf", "--quiet", "--random_seed", "5"));
  for (int i = 0; i < 10; i++)
  {
    learn_cb_adf_b18(*vw, "shared | s_" + std::to_string(i), {"| a_1", "| b_1"}, i % 2, -0.5f, 1.0f);
  }
}

// Epsilon decay with challenger_epsilon
TEST(CoverageAutomlTrees, EpsilonDecayChallengerEpsilon)
{
  auto vw = VW::initialize(vwtest::make_args("--epsilon_decay", "--model_count", "3", "--challenger_epsilon",
      "--cb_explore_adf", "--quiet", "--random_seed", "5"));
  for (int i = 0; i < 10; i++)
  {
    learn_cb_adf_b18(*vw, "shared | s_" + std::to_string(i), {"| a_1", "| b_1"}, i % 2, -0.5f, 1.0f);
  }
}
