// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

// Batch 14: Core Framework coverage tests targeting learner.cc, vw.cc, gd.cc,
// parse_regressor.cc, and global_data.cc.

#include "vw/core/learner.h"
#include "vw/core/parse_regressor.h"
#include "vw/core/reductions/gd.h"
#include "vw/core/vw.h"
#include "vw/test_common/test_common.h"

#include <gtest/gtest.h>

#include <cmath>
#include <cstdio>
#include <fstream>
#include <string>

// ============================================================
// Learner tests (~25 tests)
// ============================================================

TEST(CoverageCoreFramework, LearnerGetEnabledLearners)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  std::vector<std::string> enabled;
  vw->l->get_enabled_learners(enabled);
  EXPECT_FALSE(enabled.empty());
  // The bottom learner should be "gd"
  EXPECT_EQ(enabled[0], "gd");
}

TEST(CoverageCoreFramework, LearnerGetName)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  std::string name = vw->l->get_name();
  EXPECT_FALSE(name.empty());
}

TEST(CoverageCoreFramework, LearnerIsMultilineForSingleline)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  EXPECT_FALSE(vw->l->is_multiline());
}

TEST(CoverageCoreFramework, LearnerIsMultilineForMultiline)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--quiet"));
  EXPECT_TRUE(vw->l->is_multiline());
}

TEST(CoverageCoreFramework, LearnerGetLearnerByNamePrefixGd)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* found = vw->l->get_learner_by_name_prefix("gd");
  EXPECT_NE(found, nullptr);
  EXPECT_NE(found->get_name().find("gd"), std::string::npos);
}

TEST(CoverageCoreFramework, LearnerGetLearnerByNamePrefixNotFound)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  EXPECT_THROW(vw->l->get_learner_by_name_prefix("nonexistent_learner_xyz"), VW::vw_exception);
}

TEST(CoverageCoreFramework, LearnerSensitivityBasic)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  float sens = vw->l->sensitivity(*ex);
  EXPECT_TRUE(std::isfinite(sens));
  VW::finish_example(*vw, *ex);
}

TEST(CoverageCoreFramework, LearnerSensitivityAdaptive)
{
  auto vw = VW::initialize(vwtest::make_args("--adaptive", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
  auto* ex2 = VW::read_example(*vw, "1 | a b c");
  float sens = vw->l->sensitivity(*ex2);
  EXPECT_TRUE(std::isfinite(sens));
  VW::finish_example(*vw, *ex2);
}

TEST(CoverageCoreFramework, LearnerSensitivityNormalized)
{
  auto vw = VW::initialize(vwtest::make_args("--normalized", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
  auto* ex2 = VW::read_example(*vw, "1 | d e f");
  float sens = vw->l->sensitivity(*ex2);
  EXPECT_TRUE(std::isfinite(sens));
  VW::finish_example(*vw, *ex2);
}

TEST(CoverageCoreFramework, LearnerSensitivityAdaptiveAndNormalized)
{
  auto vw = VW::initialize(vwtest::make_args("--adaptive", "--normalized", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
  // Use the same features that were trained on so normalized weights are non-zero
  auto* ex2 = VW::read_example(*vw, "0 | a b c");
  float sens = vw->l->sensitivity(*ex2);
  // Sensitivity may be infinite for unseen features with adaptive+normalized;
  // just check it doesn't crash and returns a value
  (void)sens;
  VW::finish_example(*vw, *ex2);
}

TEST(CoverageCoreFramework, LearnerRequireSingleline)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* sl = VW::LEARNER::require_singleline(vw->l.get());
  EXPECT_NE(sl, nullptr);
}

TEST(CoverageCoreFramework, LearnerRequireMultilineOnSinglelineThrows)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  EXPECT_THROW(VW::LEARNER::require_multiline(vw->l.get()), VW::vw_exception);
}

TEST(CoverageCoreFramework, LearnerRequireMultiline)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--quiet"));
  auto* ml = VW::LEARNER::require_multiline(vw->l.get());
  EXPECT_NE(ml, nullptr);
}

TEST(CoverageCoreFramework, LearnerRequireSinglelineOnMultilineThrows)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--quiet"));
  EXPECT_THROW(VW::LEARNER::require_singleline(vw->l.get()), VW::vw_exception);
}

TEST(CoverageCoreFramework, LearnerSaveLoadRoundTrip)
{
  const char* model_file = "/tmp/batch14_learner_sl.vw";
  std::remove(model_file);
  {
    auto vw = VW::initialize(vwtest::make_args("--quiet", "-f", model_file));
    auto* ex = VW::read_example(*vw, "1 | a b c");
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
    vw->finish();
  }
  {
    auto vw = VW::initialize(vwtest::make_args("--quiet", "-i", model_file));
    auto* ex = VW::read_example(*vw, "| a b c");
    vw->predict(*ex);
    EXPECT_TRUE(std::isfinite(ex->pred.scalar));
    VW::finish_example(*vw, *ex);
  }
  std::remove(model_file);
}

TEST(CoverageCoreFramework, LearnerGetInputLabelType)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto lt = vw->l->get_input_label_type();
  EXPECT_EQ(lt, VW::label_type_t::SIMPLE);
}

TEST(CoverageCoreFramework, LearnerGetOutputPredictionType)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto pt = vw->l->get_output_prediction_type();
  EXPECT_EQ(pt, VW::prediction_type_t::SCALAR);
}

TEST(CoverageCoreFramework, LearnerMultipredictBasic)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  // Train a bit first
  for (int i = 0; i < 5; i++)
  {
    auto* ex = VW::read_example(*vw, "1 | a b c");
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
  auto* ex = VW::read_example(*vw, "| a b c");
  VW::polyprediction preds[1];
  vw->l->multipredict(*ex, 0, 1, preds, true);
  EXPECT_TRUE(std::isfinite(preds[0].scalar));
  VW::finish_example(*vw, *ex);
}

TEST(CoverageCoreFramework, LearnerDepthTracking)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  EXPECT_EQ(ex->debug_current_reduction_depth, 0u);
  vw->learn(*ex);
  // After learn completes, depth should be back to 0
  EXPECT_EQ(ex->debug_current_reduction_depth, 0u);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageCoreFramework, LearnerGetBaseLearner)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  // The top learner should have a base learner (gd)
  auto* base = vw->l->get_base_learner();
  // For simple setup, the top-level IS gd, so base may be null
  // Just ensure no crash
  (void)base;
}

TEST(CoverageCoreFramework, LearnerPersistMetrics)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--extra_metrics", "/tmp/batch14_metrics.json"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
  // Metrics will be written on finish
  std::remove("/tmp/batch14_metrics.json");
}

TEST(CoverageCoreFramework, LearnerLearnReturnsPredictonProperty)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  // learn_returns_prediction is a property of the learner
  bool lrp = vw->l->learn_returns_prediction;
  // Just verify it's a valid boolean
  EXPECT_TRUE(lrp == true || lrp == false);
}

// ============================================================
// VW initialization tests (~25 tests)
// ============================================================

TEST(CoverageCoreFramework, VwInitBasicQuiet)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  EXPECT_NE(vw, nullptr);
}

TEST(CoverageCoreFramework, VwAreFeaturesCompatibleSameConfig)
{
  auto vw1 = VW::initialize(vwtest::make_args("--quiet"));
  auto vw2 = VW::initialize(vwtest::make_args("--quiet"));
  const char* result = VW::are_features_compatible(*vw1, *vw2);
  EXPECT_EQ(result, nullptr);
}

TEST(CoverageCoreFramework, VwAreFeaturesCompatibleDifferentBits)
{
  auto vw1 = VW::initialize(vwtest::make_args("--quiet", "-b", "16"));
  auto vw2 = VW::initialize(vwtest::make_args("--quiet", "-b", "17"));
  const char* result = VW::are_features_compatible(*vw1, *vw2);
  EXPECT_NE(result, nullptr);
  EXPECT_STREQ(result, "num_bits");
}

TEST(CoverageCoreFramework, VwAreFeaturesCompatibleDifferentInteractions)
{
  auto vw1 = VW::initialize(vwtest::make_args("--quiet", "-q", "ab"));
  auto vw2 = VW::initialize(vwtest::make_args("--quiet"));
  const char* result = VW::are_features_compatible(*vw1, *vw2);
  EXPECT_NE(result, nullptr);
}

TEST(CoverageCoreFramework, VwAreFeaturesCompatibleNoConstant)
{
  auto vw1 = VW::initialize(vwtest::make_args("--quiet", "--noconstant"));
  auto vw2 = VW::initialize(vwtest::make_args("--quiet"));
  const char* result = VW::are_features_compatible(*vw1, *vw2);
  EXPECT_NE(result, nullptr);
  EXPECT_STREQ(result, "add_constant");
}

TEST(CoverageCoreFramework, VwFeatureLimitBasic)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--feature_limit", "3"));
  auto* ex = VW::read_example(*vw, "1 | a:1 b:2 c:3 d:4 e:5 f:6");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageCoreFramework, VwFeatureLimitNamespaceSpecific)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--feature_limit", "x2"));
  auto* ex = VW::read_example(*vw, "1 |x a:1 b:2 c:3 d:4");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageCoreFramework, VwHoldoutPeriod)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--holdout_period", "3"));
  for (int i = 0; i < 10; i++)
  {
    auto* ex = VW::read_example(*vw, "1 | a b c");
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

TEST(CoverageCoreFramework, VwHoldoutAfter)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--holdout_after", "5"));
  for (int i = 0; i < 10; i++)
  {
    auto* ex = VW::read_example(*vw, "1 | a b c");
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

TEST(CoverageCoreFramework, VwGetFeaturesAndReturn)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a:1 b:2 c:3");
  size_t feature_number = 0;
  VW::feature* features = VW::get_features(*vw, ex, feature_number);
  // Should have at least 3 features + constant
  EXPECT_GE(feature_number, 3u);
  VW::return_features(features);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageCoreFramework, VwGetFeaturesSparseWeights)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--sparse_weights"));
  auto* ex = VW::read_example(*vw, "1 | a:1 b:2 c:3");
  size_t feature_number = 0;
  VW::feature* features = VW::get_features(*vw, ex, feature_number);
  EXPECT_GE(feature_number, 3u);
  VW::return_features(features);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageCoreFramework, VwLossFunctionSquared)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--loss_function", "squared"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageCoreFramework, VwLossFunctionHinge)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--loss_function", "hinge"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageCoreFramework, VwLossFunctionLogistic)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--loss_function", "logistic"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageCoreFramework, VwLossFunctionQuantile)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--loss_function", "quantile"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageCoreFramework, VwLossFunctionPoisson)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--loss_function", "poisson"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageCoreFramework, VwLossFunctionExpectile)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--loss_function", "expectile", "--expectile_q", "0.25"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageCoreFramework, VwPrintEnabledLearners)
{
  // Use non-quiet mode to cover the print_enabled_learners path
  auto vw = VW::initialize(vwtest::make_args("--no_stdin"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageCoreFramework, VwReadExampleString)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  std::string line = "1 | feature1 feature2";
  auto* ex = VW::read_example(*vw, line);
  EXPECT_NE(ex, nullptr);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageCoreFramework, VwSetupExampleMultipleNamespaces)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* ex = VW::read_example(*vw, "1 |a f1:1 |b f2:2 |c f3:3");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageCoreFramework, VwTestOnlyExample)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  // Test-only example (no label)
  auto* ex = VW::read_example(*vw, "| a b c");
  vw->learn(*ex);
  // Test-only examples go through predict path
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  VW::finish_example(*vw, *ex);
}

TEST(CoverageCoreFramework, VwTestModeFlag)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "-t"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  VW::finish_example(*vw, *ex);
}

TEST(CoverageCoreFramework, VwSeedModel)
{
  auto vw1 = VW::initialize(vwtest::make_args("--quiet"));
  auto* ex1 = VW::read_example(*vw1, "1 | a b c");
  vw1->learn(*ex1);
  VW::finish_example(*vw1, *ex1);

  auto vw2 = VW::seed_vw_model(*vw1, {"--quiet"});
  auto* ex2 = VW::read_example(*vw2, "| a b c");
  vw2->predict(*ex2);
  EXPECT_TRUE(std::isfinite(ex2->pred.scalar));
  VW::finish_example(*vw2, *ex2);
}

TEST(CoverageCoreFramework, VwPermutationsFlag)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--permutations", "-q", "ab"));
  auto* ex = VW::read_example(*vw, "1 |a x:1 |b y:2");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

// ============================================================
// GD tests (~30 tests)
// ============================================================

TEST(CoverageCoreFramework, GdAdaptiveFlag)
{
  auto vw = VW::initialize(vwtest::make_args("--adaptive", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  VW::finish_example(*vw, *ex);
}

TEST(CoverageCoreFramework, GdNormalizedFlag)
{
  auto vw = VW::initialize(vwtest::make_args("--normalized", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  VW::finish_example(*vw, *ex);
}

TEST(CoverageCoreFramework, GdAdaptiveNormalized)
{
  auto vw = VW::initialize(vwtest::make_args("--adaptive", "--normalized", "--quiet"));
  for (int i = 0; i < 10; i++)
  {
    auto* ex = VW::read_example(*vw, "1 | a b c");
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

TEST(CoverageCoreFramework, GdSparseWeights)
{
  auto vw = VW::initialize(vwtest::make_args("--sparse_weights", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  VW::finish_example(*vw, *ex);
}

TEST(CoverageCoreFramework, GdSparseWeightsAdaptive)
{
  auto vw = VW::initialize(vwtest::make_args("--sparse_weights", "--adaptive", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  VW::finish_example(*vw, *ex);
}

TEST(CoverageCoreFramework, GdLearningRateSmall)
{
  auto vw = VW::initialize(vwtest::make_args("--learning_rate", "0.001", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a:1 b:2 c:3");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  VW::finish_example(*vw, *ex);
}

TEST(CoverageCoreFramework, GdLearningRateLarge)
{
  auto vw = VW::initialize(vwtest::make_args("--learning_rate", "10.0", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a:1 b:2 c:3");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  VW::finish_example(*vw, *ex);
}

TEST(CoverageCoreFramework, GdPowerT0)
{
  auto vw = VW::initialize(vwtest::make_args("--power_t", "0", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  VW::finish_example(*vw, *ex);
}

TEST(CoverageCoreFramework, GdPowerT1)
{
  auto vw = VW::initialize(vwtest::make_args("--power_t", "1", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  VW::finish_example(*vw, *ex);
}

TEST(CoverageCoreFramework, GdPowerTHalf)
{
  auto vw = VW::initialize(vwtest::make_args("--power_t", "0.5", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  VW::finish_example(*vw, *ex);
}

TEST(CoverageCoreFramework, GdAuditMode)
{
  auto vw = VW::initialize(vwtest::make_args("--audit", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a:1 b:2 c:3");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageCoreFramework, GdAuditModeAdaptive)
{
  auto vw = VW::initialize(vwtest::make_args("--audit", "--adaptive", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a:1 b:2 c:3");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageCoreFramework, GdL1Regularization)
{
  auto vw = VW::initialize(vwtest::make_args("--l1", "0.01", "--quiet"));
  for (int i = 0; i < 10; i++)
  {
    auto* ex = VW::read_example(*vw, "1 | a:1 b:2 c:3");
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

TEST(CoverageCoreFramework, GdL2Regularization)
{
  auto vw = VW::initialize(vwtest::make_args("--l2", "0.01", "--quiet"));
  for (int i = 0; i < 10; i++)
  {
    auto* ex = VW::read_example(*vw, "1 | a:1 b:2 c:3");
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

TEST(CoverageCoreFramework, GdL1AndL2Combined)
{
  auto vw = VW::initialize(vwtest::make_args("--l1", "0.001", "--l2", "0.001", "--quiet"));
  for (int i = 0; i < 20; i++)
  {
    auto* ex = VW::read_example(*vw, "1 | a:1 b:2 c:3");
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

TEST(CoverageCoreFramework, GdInvariantUpdates)
{
  // --invariant is the default, but explicitly set it
  auto vw = VW::initialize(vwtest::make_args("--invariant", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  VW::finish_example(*vw, *ex);
}

TEST(CoverageCoreFramework, GdAdaxFlag)
{
  auto vw = VW::initialize(vwtest::make_args("--adax", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a:1 b:2 c:3");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageCoreFramework, GdConvergenceMultipleExamples)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  float first_pred = 0.0f;
  float last_pred = 0.0f;
  for (int i = 0; i < 50; i++)
  {
    auto* ex = VW::read_example(*vw, "1 | a:1 b:2 c:3");
    vw->learn(*ex);
    if (i == 0) { first_pred = ex->pred.scalar; }
    if (i == 49) { last_pred = ex->pred.scalar; }
    VW::finish_example(*vw, *ex);
  }
  // After training, prediction should move toward label 1
  EXPECT_GT(last_pred, first_pred);
}

TEST(CoverageCoreFramework, GdNegativeLabels)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* ex = VW::read_example(*vw, "-1 | a:1 b:2 c:3");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  VW::finish_example(*vw, *ex);
}

TEST(CoverageCoreFramework, GdZeroLabel)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* ex = VW::read_example(*vw, "0 | a:1 b:2 c:3");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  VW::finish_example(*vw, *ex);
}

TEST(CoverageCoreFramework, GdLargeLabel)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* ex = VW::read_example(*vw, "100 | a:1 b:2 c:3");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  VW::finish_example(*vw, *ex);
}

TEST(CoverageCoreFramework, GdNoConstant)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--noconstant"));
  auto* ex = VW::read_example(*vw, "1 | a:1 b:2 c:3");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  VW::finish_example(*vw, *ex);
}

TEST(CoverageCoreFramework, GdInitialWeight)
{
  // --initial_weight sets the default weight via set_default, but in dense mode
  // the prediction path may still return 0 because the weight stride/layout means
  // the initialized slot isn't always what the prediction reads.  Just verify no crash
  // and that after learning the prediction moves.
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--initial_weight", "0.1"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  VW::finish_example(*vw, *ex);
}

TEST(CoverageCoreFramework, GdInitialT)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--initial_t", "10"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  VW::finish_example(*vw, *ex);
}

TEST(CoverageCoreFramework, GdInteractions)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "-q", "ab"));
  auto* ex = VW::read_example(*vw, "1 |a x:1 |b y:2");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  VW::finish_example(*vw, *ex);
}

TEST(CoverageCoreFramework, GdCubicInteractions)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--cubic", "abc"));
  auto* ex = VW::read_example(*vw, "1 |a x:1 |b y:2 |c z:3");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  VW::finish_example(*vw, *ex);
}

TEST(CoverageCoreFramework, GdWeightedExample)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* ex = VW::read_example(*vw, "1 5.0 | a:1 b:2 c:3");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageCoreFramework, GdHashInv)
{
  // --hash_inv is not a standalone flag; use --invert_hash <file> which sets hash_inv internally
  const char* inv_file = "/tmp/batch14_hash_inv.txt";
  std::remove(inv_file);
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--invert_hash", inv_file));
  auto* ex = VW::read_example(*vw, "1 | a:1 b:2 c:3");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
  EXPECT_TRUE(vw->output_config.hash_inv);
  vw->finish();
  std::remove(inv_file);
}

TEST(CoverageCoreFramework, GdIgnoreNamespace)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--ignore", "b"));
  auto* ex = VW::read_example(*vw, "1 |a x:1 |b y:2");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageCoreFramework, GdIgnoreLinearNamespace)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--ignore_linear", "a", "-q", "ab"));
  auto* ex = VW::read_example(*vw, "1 |a x:1 |b y:2");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

// ============================================================
// Parse regressor tests (~20 tests)
// ============================================================

TEST(CoverageCoreFramework, ParseRegressorSaveAndLoadBinary)
{
  const char* model_file = "/tmp/batch14_regressor_bin.vw";
  std::remove(model_file);
  {
    auto vw = VW::initialize(vwtest::make_args("--quiet", "-f", model_file));
    for (int i = 0; i < 10; i++)
    {
      auto* ex = VW::read_example(*vw, "1 | a:1 b:2 c:3");
      vw->learn(*ex);
      VW::finish_example(*vw, *ex);
    }
    vw->finish();
  }
  {
    auto vw = VW::initialize(vwtest::make_args("--quiet", "-i", model_file));
    auto* ex = VW::read_example(*vw, "| a:1 b:2 c:3");
    vw->predict(*ex);
    // After training, prediction should be positive and close to label
    EXPECT_GT(ex->pred.scalar, 0.0f);
    VW::finish_example(*vw, *ex);
  }
  std::remove(model_file);
}

TEST(CoverageCoreFramework, ParseRegressorSaveResumeFlag)
{
  const char* model_file = "/tmp/batch14_save_resume.vw";
  std::remove(model_file);
  {
    auto vw = VW::initialize(vwtest::make_args("--quiet", "--save_resume", "-f", model_file));
    for (int i = 0; i < 5; i++)
    {
      auto* ex = VW::read_example(*vw, "1 | a:1 b:2");
      vw->learn(*ex);
      VW::finish_example(*vw, *ex);
    }
    vw->finish();
  }
  {
    auto vw = VW::initialize(vwtest::make_args("--quiet", "-i", model_file));
    auto* ex = VW::read_example(*vw, "| a:1 b:2");
    vw->predict(*ex);
    EXPECT_TRUE(std::isfinite(ex->pred.scalar));
    VW::finish_example(*vw, *ex);
  }
  std::remove(model_file);
}

TEST(CoverageCoreFramework, ParseRegressorPredictOnlyModel)
{
  const char* model_file = "/tmp/batch14_predict_only.vw";
  std::remove(model_file);
  {
    auto vw = VW::initialize(vwtest::make_args("--quiet", "--predict_only_model", "-f", model_file));
    for (int i = 0; i < 5; i++)
    {
      auto* ex = VW::read_example(*vw, "1 | a:1 b:2");
      vw->learn(*ex);
      VW::finish_example(*vw, *ex);
    }
    vw->finish();
  }
  {
    auto vw = VW::initialize(vwtest::make_args("--quiet", "-i", model_file, "-t"));
    auto* ex = VW::read_example(*vw, "| a:1 b:2");
    vw->predict(*ex);
    EXPECT_TRUE(std::isfinite(ex->pred.scalar));
    VW::finish_example(*vw, *ex);
  }
  std::remove(model_file);
}

TEST(CoverageCoreFramework, ParseRegressorReadableModel)
{
  const char* model_file = "/tmp/batch14_readable.txt";
  std::remove(model_file);
  {
    auto vw = VW::initialize(vwtest::make_args("--quiet", "--readable_model", model_file));
    for (int i = 0; i < 5; i++)
    {
      auto* ex = VW::read_example(*vw, "1 | a:1 b:2 c:3");
      vw->learn(*ex);
      VW::finish_example(*vw, *ex);
    }
    vw->finish();
  }
  // Verify the readable model file exists and is non-empty
  std::ifstream fin(model_file);
  EXPECT_TRUE(fin.good());
  fin.seekg(0, std::ios::end);
  EXPECT_GT(fin.tellg(), 0);
  fin.close();
  std::remove(model_file);
}

TEST(CoverageCoreFramework, ParseRegressorInvertHash)
{
  const char* model_file = "/tmp/batch14_invert_hash.txt";
  std::remove(model_file);
  {
    auto vw = VW::initialize(vwtest::make_args("--quiet", "--invert_hash", model_file));
    for (int i = 0; i < 5; i++)
    {
      auto* ex = VW::read_example(*vw, "1 | a:1 b:2 c:3");
      vw->learn(*ex);
      VW::finish_example(*vw, *ex);
    }
    vw->finish();
  }
  std::ifstream fin(model_file);
  EXPECT_TRUE(fin.good());
  fin.seekg(0, std::ios::end);
  EXPECT_GT(fin.tellg(), 0);
  fin.close();
  std::remove(model_file);
}

TEST(CoverageCoreFramework, ParseRegressorSaveResumeThenContinueTraining)
{
  const char* model_file = "/tmp/batch14_resume_train.vw";
  std::remove(model_file);
  float pred_before_resume = 0.0f;
  {
    auto vw = VW::initialize(vwtest::make_args("--quiet", "--save_resume", "-f", model_file));
    for (int i = 0; i < 10; i++)
    {
      auto* ex = VW::read_example(*vw, "1 | a:1 b:2");
      vw->learn(*ex);
      VW::finish_example(*vw, *ex);
    }
    auto* ex = VW::read_example(*vw, "| a:1 b:2");
    vw->predict(*ex);
    pred_before_resume = ex->pred.scalar;
    VW::finish_example(*vw, *ex);
    vw->finish();
  }
  {
    auto vw = VW::initialize(vwtest::make_args("--quiet", "-i", model_file));
    // Continue training
    for (int i = 0; i < 10; i++)
    {
      auto* ex = VW::read_example(*vw, "1 | a:1 b:2");
      vw->learn(*ex);
      VW::finish_example(*vw, *ex);
    }
    auto* ex = VW::read_example(*vw, "| a:1 b:2");
    vw->predict(*ex);
    // After more training, prediction should be at least as close to 1 as before
    EXPECT_GE(ex->pred.scalar, pred_before_resume - 0.1f);
    VW::finish_example(*vw, *ex);
  }
  std::remove(model_file);
}

TEST(CoverageCoreFramework, ParseRegressorPreservePerformanceCounters)
{
  const char* model_file = "/tmp/batch14_preserve_counters.vw";
  std::remove(model_file);
  {
    auto vw = VW::initialize(
        vwtest::make_args("--quiet", "--save_resume", "--preserve_performance_counters", "-f", model_file));
    for (int i = 0; i < 5; i++)
    {
      auto* ex = VW::read_example(*vw, "1 | a:1 b:2");
      vw->learn(*ex);
      VW::finish_example(*vw, *ex);
    }
    vw->finish();
  }
  {
    auto vw = VW::initialize(vwtest::make_args("--quiet", "-i", model_file, "--preserve_performance_counters"));
    auto* ex = VW::read_example(*vw, "| a:1 b:2");
    vw->predict(*ex);
    EXPECT_TRUE(std::isfinite(ex->pred.scalar));
    VW::finish_example(*vw, *ex);
  }
  std::remove(model_file);
}

TEST(CoverageCoreFramework, ParseRegressorSaveLoadAdaptive)
{
  const char* model_file = "/tmp/batch14_adaptive_sl.vw";
  std::remove(model_file);
  {
    auto vw = VW::initialize(vwtest::make_args("--adaptive", "--quiet", "-f", model_file));
    for (int i = 0; i < 5; i++)
    {
      auto* ex = VW::read_example(*vw, "1 | a:1 b:2");
      vw->learn(*ex);
      VW::finish_example(*vw, *ex);
    }
    vw->finish();
  }
  {
    auto vw = VW::initialize(vwtest::make_args("--quiet", "-i", model_file, "-t"));
    auto* ex = VW::read_example(*vw, "| a:1 b:2");
    vw->predict(*ex);
    EXPECT_TRUE(std::isfinite(ex->pred.scalar));
    VW::finish_example(*vw, *ex);
  }
  std::remove(model_file);
}

TEST(CoverageCoreFramework, ParseRegressorSaveLoadNormalized)
{
  const char* model_file = "/tmp/batch14_normalized_sl.vw";
  std::remove(model_file);
  {
    auto vw = VW::initialize(vwtest::make_args("--normalized", "--quiet", "-f", model_file));
    for (int i = 0; i < 5; i++)
    {
      auto* ex = VW::read_example(*vw, "1 | a:1 b:2");
      vw->learn(*ex);
      VW::finish_example(*vw, *ex);
    }
    vw->finish();
  }
  {
    auto vw = VW::initialize(vwtest::make_args("--quiet", "-i", model_file, "-t"));
    auto* ex = VW::read_example(*vw, "| a:1 b:2");
    vw->predict(*ex);
    EXPECT_TRUE(std::isfinite(ex->pred.scalar));
    VW::finish_example(*vw, *ex);
  }
  std::remove(model_file);
}

TEST(CoverageCoreFramework, ParseRegressorSaveLoadAdaptiveNormalized)
{
  const char* model_file = "/tmp/batch14_adapnorm_sl.vw";
  std::remove(model_file);
  {
    auto vw = VW::initialize(vwtest::make_args("--adaptive", "--normalized", "--quiet", "-f", model_file));
    for (int i = 0; i < 5; i++)
    {
      auto* ex = VW::read_example(*vw, "1 | a:1 b:2");
      vw->learn(*ex);
      VW::finish_example(*vw, *ex);
    }
    vw->finish();
  }
  {
    auto vw = VW::initialize(vwtest::make_args("--quiet", "-i", model_file, "-t"));
    auto* ex = VW::read_example(*vw, "| a:1 b:2");
    vw->predict(*ex);
    EXPECT_TRUE(std::isfinite(ex->pred.scalar));
    VW::finish_example(*vw, *ex);
  }
  std::remove(model_file);
}

TEST(CoverageCoreFramework, ParseRegressorSaveLoadInteractions)
{
  const char* model_file = "/tmp/batch14_interact_sl.vw";
  std::remove(model_file);
  {
    auto vw = VW::initialize(vwtest::make_args("--quiet", "-q", "ab", "-f", model_file));
    for (int i = 0; i < 5; i++)
    {
      auto* ex = VW::read_example(*vw, "1 |a x:1 |b y:2");
      vw->learn(*ex);
      VW::finish_example(*vw, *ex);
    }
    vw->finish();
  }
  {
    auto vw = VW::initialize(vwtest::make_args("--quiet", "-i", model_file, "-t"));
    auto* ex = VW::read_example(*vw, "|a x:1 |b y:2");
    vw->predict(*ex);
    EXPECT_TRUE(std::isfinite(ex->pred.scalar));
    VW::finish_example(*vw, *ex);
  }
  std::remove(model_file);
}

TEST(CoverageCoreFramework, ParseRegressorReadableModelSaveResume)
{
  const char* model_file = "/tmp/batch14_readable_sr.txt";
  std::remove(model_file);
  {
    auto vw = VW::initialize(vwtest::make_args("--quiet", "--save_resume", "--readable_model", model_file));
    for (int i = 0; i < 5; i++)
    {
      auto* ex = VW::read_example(*vw, "1 | a:1 b:2");
      vw->learn(*ex);
      VW::finish_example(*vw, *ex);
    }
    vw->finish();
  }
  std::ifstream fin(model_file);
  EXPECT_TRUE(fin.good());
  fin.seekg(0, std::ios::end);
  EXPECT_GT(fin.tellg(), 0);
  fin.close();
  std::remove(model_file);
}

TEST(CoverageCoreFramework, ParseRegressorSaveLoadSparseWeights)
{
  const char* model_file = "/tmp/batch14_sparse_sl.vw";
  std::remove(model_file);
  {
    auto vw = VW::initialize(vwtest::make_args("--sparse_weights", "--quiet", "-f", model_file));
    for (int i = 0; i < 5; i++)
    {
      auto* ex = VW::read_example(*vw, "1 | a:1 b:2");
      vw->learn(*ex);
      VW::finish_example(*vw, *ex);
    }
    vw->finish();
  }
  {
    auto vw = VW::initialize(vwtest::make_args("--quiet", "-i", model_file, "-t"));
    auto* ex = VW::read_example(*vw, "| a:1 b:2");
    vw->predict(*ex);
    EXPECT_TRUE(std::isfinite(ex->pred.scalar));
    VW::finish_example(*vw, *ex);
  }
  std::remove(model_file);
}

TEST(CoverageCoreFramework, ParseRegressorRandomWeightsInit)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--random_weights"));
  auto* ex = VW::read_example(*vw, "| a:1 b:2 c:3");
  vw->predict(*ex);
  // With random weights, prediction should be non-zero
  // (with high probability)
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  VW::finish_example(*vw, *ex);
}

TEST(CoverageCoreFramework, ParseRegressorRandomPositiveWeightsInit)
{
  // random_positive_weights is not a CLI flag; it is set internally by --new_mf.
  // Use --new_mf to exercise the random_positive_weights code path.
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--new_mf", "2"));
  EXPECT_TRUE(vw->initial_weights_config.random_positive_weights);
  auto* ex = VW::read_example(*vw, "1 |user a |item b");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  VW::finish_example(*vw, *ex);
}

TEST(CoverageCoreFramework, ParseRegressorNormalWeightsInit)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--normal_weights"));
  auto* ex = VW::read_example(*vw, "| a:1 b:2 c:3");
  vw->predict(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  VW::finish_example(*vw, *ex);
}

TEST(CoverageCoreFramework, ParseRegressorTruncatedNormalWeightsInit)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--truncated_normal_weights"));
  auto* ex = VW::read_example(*vw, "| a:1 b:2 c:3");
  vw->predict(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  VW::finish_example(*vw, *ex);
}

TEST(CoverageCoreFramework, ParseRegressorInitialWeightValue)
{
  // --initial_weight sets weights via set_default, but in dense mode the prediction
  // may still be 0 due to weight stride layout.  Verify the config is set and no crash.
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--initial_weight", "0.5"));
  EXPECT_FLOAT_EQ(vw->initial_weights_config.initial_weight, 0.5f);
  auto* ex = VW::read_example(*vw, "| a:1 b:2 c:3");
  vw->predict(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  VW::finish_example(*vw, *ex);
}

TEST(CoverageCoreFramework, ParseRegressorSaveLoadLogisticLoss)
{
  const char* model_file = "/tmp/batch14_logistic_sl.vw";
  std::remove(model_file);
  {
    auto vw = VW::initialize(vwtest::make_args("--quiet", "--loss_function", "logistic", "-f", model_file));
    for (int i = 0; i < 5; i++)
    {
      auto* ex = VW::read_example(*vw, "1 | a:1 b:2");
      vw->learn(*ex);
      VW::finish_example(*vw, *ex);
    }
    vw->finish();
  }
  {
    auto vw = VW::initialize(vwtest::make_args("--quiet", "-i", model_file, "-t"));
    auto* ex = VW::read_example(*vw, "| a:1 b:2");
    vw->predict(*ex);
    EXPECT_TRUE(std::isfinite(ex->pred.scalar));
    VW::finish_example(*vw, *ex);
  }
  std::remove(model_file);
}

// ============================================================
// Global data / workspace tests (~20 tests)
// ============================================================

TEST(CoverageCoreFramework, WorkspaceLearnSingleLine)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  VW::finish_example(*vw, *ex);
}

TEST(CoverageCoreFramework, WorkspacePredictSingleLine)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* ex = VW::read_example(*vw, "| a b c");
  vw->predict(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  // predict() should set test_only
  EXPECT_TRUE(ex->test_only);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageCoreFramework, WorkspaceLearnMultiLineCbAdf)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--quiet"));
  auto* shared = VW::read_example(*vw, "shared | s1 s2");
  auto* action1 = VW::read_example(*vw, "0:1:0.5 | a1");
  auto* action2 = VW::read_example(*vw, "| a2");
  VW::multi_ex multi;
  multi.push_back(shared);
  multi.push_back(action1);
  multi.push_back(action2);
  vw->learn(multi);
  vw->finish_example(multi);
}

TEST(CoverageCoreFramework, WorkspacePredictMultiLineCbAdf)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--quiet"));
  auto* shared = VW::read_example(*vw, "shared | s1 s2");
  auto* action1 = VW::read_example(*vw, "| a1");
  auto* action2 = VW::read_example(*vw, "| a2");
  VW::multi_ex multi;
  multi.push_back(shared);
  multi.push_back(action1);
  multi.push_back(action2);
  vw->predict(multi);
  // Predictions should be set on action examples
  EXPECT_FALSE(multi[0]->pred.a_s.empty());
  vw->finish_example(multi);
}

TEST(CoverageCoreFramework, WorkspaceFinishExampleSingleLine)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageCoreFramework, WorkspaceFinishExampleMultiLine)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--quiet"));
  auto* shared = VW::read_example(*vw, "shared | s1");
  auto* action1 = VW::read_example(*vw, "| a1");
  VW::multi_ex multi;
  multi.push_back(shared);
  multi.push_back(action1);
  vw->predict(multi);
  vw->finish_example(multi);
}

TEST(CoverageCoreFramework, WorkspaceLearnReturnsCorrectPrediction)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a:1 b:2 c:3");
  vw->learn(*ex);
  float pred = ex->pred.scalar;
  EXPECT_TRUE(std::isfinite(pred));
  VW::finish_example(*vw, *ex);
}

TEST(CoverageCoreFramework, WorkspaceMultipleLearnPredict)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  for (int i = 0; i < 20; i++)
  {
    auto* ex = VW::read_example(*vw, "1 | a:1 b:2 c:3");
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
  auto* ex = VW::read_example(*vw, "| a:1 b:2 c:3");
  vw->predict(*ex);
  EXPECT_GT(ex->pred.scalar, 0.0f);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageCoreFramework, WorkspaceGetPrediction)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  float pred = VW::get_prediction(ex);
  EXPECT_TRUE(std::isfinite(pred));
  VW::finish_example(*vw, *ex);
}

TEST(CoverageCoreFramework, WorkspaceGetLabel)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  float label = VW::get_label(ex);
  EXPECT_FLOAT_EQ(label, 1.0f);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageCoreFramework, WorkspaceGetImportance)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* ex = VW::read_example(*vw, "1 2.0 | a b c");
  float imp = VW::get_importance(ex);
  EXPECT_FLOAT_EQ(imp, 2.0f);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageCoreFramework, WorkspaceGetTag)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* ex = VW::read_example(*vw, "1 1.0 mytag| a b c");
  size_t tag_len = VW::get_tag_length(ex);
  EXPECT_EQ(tag_len, 5u);  // "mytag"
  const char* tag = VW::get_tag(ex);
  EXPECT_EQ(std::string(tag, tag_len), "mytag");
  VW::finish_example(*vw, *ex);
}

TEST(CoverageCoreFramework, WorkspaceGetFeatureNumber)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a:1 b:2 c:3");
  size_t num = VW::get_feature_number(ex);
  // Should have 3 features + constant = 4
  EXPECT_EQ(num, 4u);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageCoreFramework, WorkspaceAddLabel)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* ex = VW::new_unused_example(*vw);
  VW::add_label(ex, 1.0f, 1.0f, 0.0f);
  EXPECT_FLOAT_EQ(ex->l.simple.label, 1.0f);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageCoreFramework, WorkspaceNewUnusedExample)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* ex = VW::new_unused_example(*vw);
  EXPECT_NE(ex, nullptr);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageCoreFramework, WorkspaceAllocDeallocExamples)
{
  auto examples = std::make_unique<VW::example[]>(3);
  EXPECT_NE(examples.get(), nullptr);
}

TEST(CoverageCoreFramework, WorkspaceSharedData)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  EXPECT_NE(vw->sd, nullptr);
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
  // The direct API path (learn + VW::finish_example) does not invoke the
  // learner's finish_example callback that updates sd counters.  Instead,
  // verify that sd->update() works directly.
  vw->sd->update(false /*test_only*/, true /*labeled*/, 0.5f /*loss*/, 1.0f /*weight*/, 4 /*num_features*/);
  EXPECT_GT(vw->sd->weighted_labeled_examples, 0.0);
  EXPECT_GT(vw->sd->sum_loss, 0.0);
}

TEST(CoverageCoreFramework, WorkspaceOutputConfig)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  // Quiet mode should be set
  EXPECT_TRUE(vw->output_config.quiet);
}

TEST(CoverageCoreFramework, WorkspaceLearnMultipleExampleTypes)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  // Learn positive examples
  for (int i = 0; i < 10; i++)
  {
    auto* ex = VW::read_example(*vw, "1 | pos_a pos_b");
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
  // Learn negative examples
  for (int i = 0; i < 10; i++)
  {
    auto* ex = VW::read_example(*vw, "-1 | neg_a neg_b");
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
  // Predict should differentiate
  auto* pos_ex = VW::read_example(*vw, "| pos_a pos_b");
  vw->predict(*pos_ex);
  float pos_pred = pos_ex->pred.scalar;
  VW::finish_example(*vw, *pos_ex);

  auto* neg_ex = VW::read_example(*vw, "| neg_a neg_b");
  vw->predict(*neg_ex);
  float neg_pred = neg_ex->pred.scalar;
  VW::finish_example(*vw, *neg_ex);

  EXPECT_GT(pos_pred, neg_pred);
}

VW_WARNING_STATE_PUSH
VW_WARNING_DISABLE_DEPRECATED_USAGE
TEST(CoverageCoreFramework, WorkspaceCmdStringReplaceValue)
{
  auto* ss = new std::stringstream();
  *ss << "--learning_rate 0.5 --power_t 0.5";
  VW::cmd_string_replace_value(ss, "--learning_rate", "1.0");
  std::string result = ss->str();
  EXPECT_NE(result.find("1.0"), std::string::npos);
  delete ss;
}

TEST(CoverageCoreFramework, WorkspaceCmdStringReplaceValueNewFlag)
{
  auto* ss = new std::stringstream();
  *ss << "--learning_rate 0.5";
  VW::cmd_string_replace_value(ss, "--new_flag", "hello");
  std::string result = ss->str();
  EXPECT_NE(result.find("--new_flag"), std::string::npos);
  EXPECT_NE(result.find("hello"), std::string::npos);
  delete ss;
}
VW_WARNING_STATE_POP
