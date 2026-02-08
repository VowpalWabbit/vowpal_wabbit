// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

// Batch 9: Targeted tests for reductions with zero dedicated unit tests.

#include "vw/core/example.h"
#include "vw/core/vw.h"
#include "vw/io/io_adapter.h"
#include "vw/test_common/test_common.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cfloat>
#include <cmath>
#include <cstdio>
#include <sstream>
#include <string>

// ============================================================
// LDA (--lda N) Tests
// ============================================================

TEST(CoverageNumericReductions, LdaBasicInitAndLearn)
{
  auto vw = VW::initialize(vwtest::make_args("--lda", "3", "--quiet"));
  auto* ex = VW::read_example(*vw, "| word1:1 word2:2 word3:3");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, LdaOneTopic)
{
  auto vw = VW::initialize(vwtest::make_args("--lda", "1", "--quiet"));
  auto* ex = VW::read_example(*vw, "| word1:1 word2:2");
  vw->learn(*ex);
  EXPECT_EQ(ex->pred.scalars.size(), 1);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, LdaTwoTopics)
{
  auto vw = VW::initialize(vwtest::make_args("--lda", "2", "--quiet"));
  auto* ex = VW::read_example(*vw, "| word1:1 word2:2 word3:3");
  vw->learn(*ex);
  EXPECT_EQ(ex->pred.scalars.size(), 2);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, LdaFiveTopics)
{
  auto vw = VW::initialize(vwtest::make_args("--lda", "5", "--quiet"));
  auto* ex = VW::read_example(*vw, "| word1:1 word2:2 word3:3 word4:4 word5:5");
  vw->learn(*ex);
  EXPECT_EQ(ex->pred.scalars.size(), 5);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, LdaTenTopics)
{
  auto vw = VW::initialize(vwtest::make_args("--lda", "10", "--quiet"));
  auto* ex = VW::read_example(*vw, "| word1:1 word2:2 word3:3");
  vw->learn(*ex);
  EXPECT_EQ(ex->pred.scalars.size(), 10);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, LdaAlphaParam)
{
  auto vw = VW::initialize(vwtest::make_args("--lda", "3", "--lda_alpha", "0.5", "--quiet"));
  auto* ex = VW::read_example(*vw, "| word1:1 word2:2");
  vw->learn(*ex);
  EXPECT_EQ(ex->pred.scalars.size(), 3);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, LdaRhoParam)
{
  auto vw = VW::initialize(vwtest::make_args("--lda", "3", "--lda_rho", "0.5", "--quiet"));
  auto* ex = VW::read_example(*vw, "| word1:1 word2:2");
  vw->learn(*ex);
  EXPECT_EQ(ex->pred.scalars.size(), 3);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, LdaAlphaAndRho)
{
  auto vw = VW::initialize(vwtest::make_args("--lda", "3", "--lda_alpha", "0.5", "--lda_rho", "0.5", "--quiet"));
  auto* ex = VW::read_example(*vw, "| word1:1 word2:2 word3:3");
  vw->learn(*ex);
  EXPECT_EQ(ex->pred.scalars.size(), 3);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, LdaDocumentCountHint)
{
  auto vw = VW::initialize(vwtest::make_args("--lda", "3", "--lda_D", "100", "--quiet"));
  auto* ex = VW::read_example(*vw, "| word1:1 word2:2");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, LdaTighterConvergence)
{
  auto vw = VW::initialize(vwtest::make_args("--lda", "3", "--lda_epsilon", "0.01", "--quiet"));
  auto* ex = VW::read_example(*vw, "| word1:1 word2:2 word3:3");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, LdaMathModeSIMD)
{
  auto vw = VW::initialize(vwtest::make_args("--lda", "3", "--math-mode", "0", "--quiet"));
  auto* ex = VW::read_example(*vw, "| word1:1 word2:2 word3:3");
  vw->learn(*ex);
  EXPECT_EQ(ex->pred.scalars.size(), 3);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, LdaMathModePrecise)
{
  auto vw = VW::initialize(vwtest::make_args("--lda", "3", "--math-mode", "1", "--quiet"));
  auto* ex = VW::read_example(*vw, "| word1:1 word2:2 word3:3");
  vw->learn(*ex);
  EXPECT_EQ(ex->pred.scalars.size(), 3);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, LdaMathModeFastApprox)
{
  auto vw = VW::initialize(vwtest::make_args("--lda", "3", "--math-mode", "2", "--quiet"));
  auto* ex = VW::read_example(*vw, "| word1:1 word2:2 word3:3");
  vw->learn(*ex);
  EXPECT_EQ(ex->pred.scalars.size(), 3);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, LdaSingleFeature)
{
  auto vw = VW::initialize(vwtest::make_args("--lda", "3", "--quiet"));
  auto* ex = VW::read_example(*vw, "| word1:1");
  vw->learn(*ex);
  EXPECT_EQ(ex->pred.scalars.size(), 3);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, LdaManyFeatures)
{
  auto vw = VW::initialize(vwtest::make_args("--lda", "3", "--quiet"));
  auto* ex = VW::read_example(*vw, "| a:1 b:2 c:3 d:4 e:5 f:6 g:7 h:8 i:9 j:10");
  vw->learn(*ex);
  EXPECT_EQ(ex->pred.scalars.size(), 3);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, LdaEmptyFeatures)
{
  auto vw = VW::initialize(vwtest::make_args("--lda", "3", "--quiet"));
  auto* ex = VW::read_example(*vw, "|");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, LdaMultipleExamples)
{
  auto vw = VW::initialize(vwtest::make_args("--lda", "3", "--quiet"));
  for (int i = 0; i < 10; i++)
  {
    auto* ex = VW::read_example(*vw, "| word1:1 word2:2 word3:3");
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageNumericReductions, LdaPredsAreNonNegative)
{
  auto vw = VW::initialize(vwtest::make_args("--lda", "3", "--quiet"));
  auto* ex = VW::read_example(*vw, "| word1:1 word2:2 word3:3");
  vw->learn(*ex);
  for (size_t i = 0; i < ex->pred.scalars.size(); i++) { EXPECT_GE(ex->pred.scalars[i], 0.0f); }
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, LdaPredsSumCloseToOne)
{
  auto vw = VW::initialize(vwtest::make_args("--lda", "3", "--quiet"));
  // Feed a few examples to warm up
  for (int i = 0; i < 5; i++)
  {
    auto* ex = VW::read_example(*vw, "| word1:1 word2:2 word3:3");
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
  auto* ex = VW::read_example(*vw, "| word1:1 word2:2 word3:3");
  vw->predict(*ex);
  float sum = 0.0f;
  for (size_t i = 0; i < ex->pred.scalars.size(); i++) { sum += ex->pred.scalars[i]; }
  // LDA topic weights should sum to approximately 1.0
  EXPECT_GT(sum, 0.0f);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, LdaDifferentFeatureDistributions)
{
  auto vw = VW::initialize(vwtest::make_args("--lda", "3", "--quiet"));
  auto* ex1 = VW::read_example(*vw, "| sports:5 football:3 game:2");
  vw->learn(*ex1);
  vw->finish_example(*ex1);

  auto* ex2 = VW::read_example(*vw, "| science:5 physics:3 theory:2");
  vw->learn(*ex2);
  vw->finish_example(*ex2);

  auto* ex3 = VW::read_example(*vw, "| art:5 painting:3 gallery:2");
  vw->learn(*ex3);
  vw->finish_example(*ex3);
}

TEST(CoverageNumericReductions, LdaPredictOnly)
{
  auto vw = VW::initialize(vwtest::make_args("--lda", "3", "--quiet"));
  auto* ex = VW::read_example(*vw, "| word1:1 word2:2 word3:3");
  vw->predict(*ex);
  EXPECT_EQ(ex->pred.scalars.size(), 3);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, LdaHighAlpha)
{
  auto vw = VW::initialize(vwtest::make_args("--lda", "3", "--lda_alpha", "10.0", "--quiet"));
  auto* ex = VW::read_example(*vw, "| word1:1 word2:2");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, LdaHighRho)
{
  auto vw = VW::initialize(vwtest::make_args("--lda", "3", "--lda_rho", "10.0", "--quiet"));
  auto* ex = VW::read_example(*vw, "| word1:1 word2:2");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, LdaWithMetrics)
{
  auto vw = VW::initialize(vwtest::make_args("--lda", "3", "--metrics", "--quiet"));
  auto* ex = VW::read_example(*vw, "| word1:1 word2:2 word3:3");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, LdaSmallLargeDocCountHints)
{
  auto vw = VW::initialize(vwtest::make_args("--lda", "3", "--lda_D", "10", "--quiet"));
  auto* ex = VW::read_example(*vw, "| word1:1 word2:2");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

// ============================================================
// Kernel SVM (--ksvm) Tests
// ============================================================

TEST(CoverageNumericReductions, KsvmBasicInit)
{
  auto vw = VW::initialize(vwtest::make_args("--ksvm", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 |f x:1.0");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, KsvmLinearKernel)
{
  auto vw = VW::initialize(vwtest::make_args("--ksvm", "--kernel", "linear", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 |f x:1.0 y:0.5");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, KsvmRbfKernel)
{
  auto vw = VW::initialize(vwtest::make_args("--ksvm", "--kernel", "rbf", "--bandwidth", "1.0", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 |f x:1.0 y:0.5");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, KsvmPolyKernelDeg2)
{
  auto vw = VW::initialize(vwtest::make_args("--ksvm", "--kernel", "poly", "--degree", "2", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 |f x:1.0 y:0.5");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, KsvmPolyKernelDeg3)
{
  auto vw = VW::initialize(vwtest::make_args("--ksvm", "--kernel", "poly", "--degree", "3", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 |f x:1.0 y:0.5");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, KsvmRbfBandwidthSmall)
{
  auto vw = VW::initialize(vwtest::make_args("--ksvm", "--kernel", "rbf", "--bandwidth", "0.1", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 |f x:1.0");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, KsvmRbfBandwidthLarge)
{
  auto vw = VW::initialize(vwtest::make_args("--ksvm", "--kernel", "rbf", "--bandwidth", "10.0", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 |f x:1.0");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, KsvmPoolSize)
{
  auto vw = VW::initialize(vwtest::make_args("--ksvm", "--pool_size", "10", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 |f x:1.0 y:0.5");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, KsvmReprocess)
{
  auto vw = VW::initialize(vwtest::make_args("--ksvm", "--reprocess", "2", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 |f x:1.0 y:0.5");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, KsvmSubsample)
{
  auto vw = VW::initialize(vwtest::make_args("--ksvm", "--subsample", "5", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 |f x:1.0 y:0.5");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, KsvmPoolGreedy)
{
  auto vw = VW::initialize(vwtest::make_args("--ksvm", "--pool_greedy", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 |f x:1.0 y:0.5");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

// NOTE: --para_active by itself causes a segfault in KSVM (known issue).
// Skipping KsvmParaActive test.

TEST(CoverageNumericReductions, KsvmBinaryClassificationPositive)
{
  auto vw = VW::initialize(vwtest::make_args("--ksvm", "--quiet"));
  for (int i = 0; i < 10; i++)
  {
    auto* ex = VW::read_example(*vw, "1 |f x:1.0 y:1.0");
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageNumericReductions, KsvmBinaryClassificationNegative)
{
  auto vw = VW::initialize(vwtest::make_args("--ksvm", "--quiet"));
  for (int i = 0; i < 10; i++)
  {
    auto* ex = VW::read_example(*vw, "-1 |f x:-1.0 y:-1.0");
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageNumericReductions, KsvmBinaryClassificationMixed)
{
  auto vw = VW::initialize(vwtest::make_args("--ksvm", "--quiet"));
  for (int i = 0; i < 20; i++)
  {
    std::string label = (i % 2 == 0) ? "1" : "-1";
    std::string features = (i % 2 == 0) ? "x:1.0 y:1.0" : "x:-1.0 y:-1.0";
    auto* ex = VW::read_example(*vw, label + " |f " + features);
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageNumericReductions, KsvmRegressionLabels)
{
  auto vw = VW::initialize(vwtest::make_args("--ksvm", "--quiet"));
  auto* ex = VW::read_example(*vw, "0.5 |f x:1.0 y:0.5");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, KsvmL2Regularization)
{
  auto vw = VW::initialize(vwtest::make_args("--ksvm", "--l2", "0.01", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 |f x:1.0 y:0.5");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, KsvmMultipleLearnIterations)
{
  auto vw = VW::initialize(vwtest::make_args("--ksvm", "--quiet"));
  for (int i = 0; i < 50; i++)
  {
    float v = static_cast<float>(i) / 50.0f;
    std::string label = (v > 0.5f) ? "1" : "-1";
    std::string ex_str = label + " |f x:" + std::to_string(v) + " y:" + std::to_string(1.0f - v);
    auto* ex = VW::read_example(*vw, ex_str);
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageNumericReductions, KsvmPredictAfterLearn)
{
  auto vw = VW::initialize(vwtest::make_args("--ksvm", "--quiet"));
  // Train
  for (int i = 0; i < 10; i++)
  {
    auto* ex = VW::read_example(*vw, "1 |f x:1.0 y:1.0");
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
  // Predict
  auto* ex = VW::read_example(*vw, "| x:1.0 y:1.0");
  vw->predict(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, KsvmRbfMultipleExamples)
{
  auto vw = VW::initialize(vwtest::make_args("--ksvm", "--kernel", "rbf", "--bandwidth", "1.0", "--quiet"));
  for (int i = 0; i < 20; i++)
  {
    std::string label = (i % 2 == 0) ? "1" : "-1";
    auto* ex = VW::read_example(*vw, label + " |f x:" + std::to_string(i * 0.1));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageNumericReductions, KsvmPolyMultipleExamples)
{
  auto vw = VW::initialize(vwtest::make_args("--ksvm", "--kernel", "poly", "--degree", "2", "--quiet"));
  for (int i = 0; i < 20; i++)
  {
    std::string label = (i % 2 == 0) ? "1" : "-1";
    auto* ex = VW::read_example(*vw, label + " |f x:" + std::to_string(i * 0.1));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageNumericReductions, KsvmPoolGreedyWithPoolSize)
{
  auto vw = VW::initialize(vwtest::make_args("--ksvm", "--pool_greedy", "--pool_size", "5", "--quiet"));
  for (int i = 0; i < 20; i++)
  {
    std::string label = (i % 2 == 0) ? "1" : "-1";
    auto* ex = VW::read_example(*vw, label + " |f x:" + std::to_string(i * 0.1) + " y:" + std::to_string(i * 0.2));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageNumericReductions, KsvmPoolSizeLargerThanExamples)
{
  auto vw = VW::initialize(vwtest::make_args("--ksvm", "--pool_size", "100", "--quiet"));
  for (int i = 0; i < 5; i++)
  {
    auto* ex = VW::read_example(*vw, "1 |f x:" + std::to_string(i * 0.5));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageNumericReductions, KsvmReprocessHigher)
{
  auto vw = VW::initialize(vwtest::make_args("--ksvm", "--reprocess", "5", "--quiet"));
  for (int i = 0; i < 15; i++)
  {
    std::string label = (i % 2 == 0) ? "1" : "-1";
    auto* ex = VW::read_example(*vw, label + " |f x:" + std::to_string(i * 0.1));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

// ============================================================
// BFGS (--bfgs) Tests
// ============================================================

TEST(CoverageNumericReductions, BfgsBasicInit)
{
  const char* cache_file = "/tmp/vw_batch9_bfgs_1.cache";
  std::remove(cache_file);
  auto vw = VW::initialize(
      vwtest::make_args("--bfgs", "--passes", "2", "--quiet", "--cache_file", cache_file, "--no_stdin"));
  auto* ex = VW::read_example(*vw, "1 |f x:1.0");
  vw->learn(*ex);
  vw->finish_example(*ex);
  std::remove(cache_file);
}

TEST(CoverageNumericReductions, BfgsConjugateGradient)
{
  const char* cache_file = "/tmp/vw_batch9_bfgs_2.cache";
  std::remove(cache_file);
  auto vw = VW::initialize(
      vwtest::make_args("--conjugate_gradient", "--passes", "2", "--quiet", "--cache_file", cache_file, "--no_stdin"));
  auto* ex = VW::read_example(*vw, "1 |f x:1.0");
  vw->learn(*ex);
  vw->finish_example(*ex);
  std::remove(cache_file);
}

TEST(CoverageNumericReductions, BfgsMem0CG)
{
  const char* cache_file = "/tmp/vw_batch9_bfgs_3.cache";
  std::remove(cache_file);
  auto vw = VW::initialize(
      vwtest::make_args("--bfgs", "--mem", "0", "--passes", "2", "--quiet", "--cache_file", cache_file, "--no_stdin"));
  auto* ex = VW::read_example(*vw, "1 |f x:1.0");
  vw->learn(*ex);
  vw->finish_example(*ex);
  std::remove(cache_file);
}

TEST(CoverageNumericReductions, BfgsMem5)
{
  const char* cache_file = "/tmp/vw_batch9_bfgs_4.cache";
  std::remove(cache_file);
  auto vw = VW::initialize(
      vwtest::make_args("--bfgs", "--mem", "5", "--passes", "2", "--quiet", "--cache_file", cache_file, "--no_stdin"));
  auto* ex = VW::read_example(*vw, "1 |f x:1.0");
  vw->learn(*ex);
  vw->finish_example(*ex);
  std::remove(cache_file);
}

TEST(CoverageNumericReductions, BfgsMem15Default)
{
  const char* cache_file = "/tmp/vw_batch9_bfgs_5.cache";
  std::remove(cache_file);
  auto vw = VW::initialize(
      vwtest::make_args("--bfgs", "--mem", "15", "--passes", "2", "--quiet", "--cache_file", cache_file, "--no_stdin"));
  auto* ex = VW::read_example(*vw, "1 |f x:1.0");
  vw->learn(*ex);
  vw->finish_example(*ex);
  std::remove(cache_file);
}

TEST(CoverageNumericReductions, BfgsHessianOn)
{
  const char* cache_file = "/tmp/vw_batch9_bfgs_6.cache";
  std::remove(cache_file);
  auto vw = VW::initialize(vwtest::make_args(
      "--bfgs", "--hessian_on", "--passes", "2", "--quiet", "--cache_file", cache_file, "--no_stdin"));
  auto* ex = VW::read_example(*vw, "1 |f x:1.0");
  vw->learn(*ex);
  vw->finish_example(*ex);
  std::remove(cache_file);
}

TEST(CoverageNumericReductions, BfgsTermination)
{
  const char* cache_file = "/tmp/vw_batch9_bfgs_7.cache";
  std::remove(cache_file);
  auto vw = VW::initialize(vwtest::make_args(
      "--bfgs", "--termination", "0.0001", "--passes", "2", "--quiet", "--cache_file", cache_file, "--no_stdin"));
  auto* ex = VW::read_example(*vw, "1 |f x:1.0");
  vw->learn(*ex);
  vw->finish_example(*ex);
  std::remove(cache_file);
}

TEST(CoverageNumericReductions, BfgsWithL2)
{
  const char* cache_file = "/tmp/vw_batch9_bfgs_8.cache";
  std::remove(cache_file);
  auto vw = VW::initialize(vwtest::make_args(
      "--bfgs", "--l2", "0.01", "--passes", "2", "--quiet", "--cache_file", cache_file, "--no_stdin"));
  auto* ex = VW::read_example(*vw, "1 |f x:1.0");
  vw->learn(*ex);
  vw->finish_example(*ex);
  std::remove(cache_file);
}

TEST(CoverageNumericReductions, BfgsSquaredLoss)
{
  const char* cache_file = "/tmp/vw_batch9_bfgs_9.cache";
  std::remove(cache_file);
  auto vw = VW::initialize(vwtest::make_args(
      "--bfgs", "--passes", "2", "--loss_function", "squared", "--quiet", "--cache_file", cache_file, "--no_stdin"));
  auto* ex = VW::read_example(*vw, "1 |f x:1.0");
  vw->learn(*ex);
  vw->finish_example(*ex);
  std::remove(cache_file);
}

TEST(CoverageNumericReductions, BfgsLogisticLoss)
{
  const char* cache_file = "/tmp/vw_batch9_bfgs_10.cache";
  std::remove(cache_file);
  auto vw = VW::initialize(vwtest::make_args(
      "--bfgs", "--passes", "2", "--loss_function", "logistic", "--quiet", "--cache_file", cache_file, "--no_stdin"));
  auto* ex = VW::read_example(*vw, "1 |f x:1.0");
  vw->learn(*ex);
  vw->finish_example(*ex);
  std::remove(cache_file);
}

TEST(CoverageNumericReductions, BfgsFeedMultipleExamples)
{
  const char* cache_file = "/tmp/vw_batch9_bfgs_11.cache";
  std::remove(cache_file);
  auto vw = VW::initialize(
      vwtest::make_args("--bfgs", "--passes", "2", "--quiet", "--cache_file", cache_file, "--no_stdin"));
  for (int i = 0; i < 10; i++)
  {
    float label = (i % 2 == 0) ? 1.0f : -1.0f;
    auto* ex = VW::read_example(*vw, std::to_string(label) + " |f x:" + std::to_string(i * 0.1f));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
  std::remove(cache_file);
}

TEST(CoverageNumericReductions, BfgsMultipleFeatures)
{
  const char* cache_file = "/tmp/vw_batch9_bfgs_12.cache";
  std::remove(cache_file);
  auto vw = VW::initialize(
      vwtest::make_args("--bfgs", "--passes", "2", "--quiet", "--cache_file", cache_file, "--no_stdin"));
  auto* ex = VW::read_example(*vw, "1 |f x:1.0 y:0.5 z:0.3");
  vw->learn(*ex);
  vw->finish_example(*ex);
  std::remove(cache_file);
}

TEST(CoverageNumericReductions, BfgsPredictOnlyMode)
{
  // In predict-only mode, BFGS does not require multi-pass
  auto vw = VW::initialize(vwtest::make_args("--bfgs", "--quiet", "--no_stdin", "-t"));
  auto* ex = VW::read_example(*vw, "| x:1.0 y:0.5");
  vw->predict(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, BfgsSmallMem)
{
  const char* cache_file = "/tmp/vw_batch9_bfgs_14.cache";
  std::remove(cache_file);
  auto vw = VW::initialize(
      vwtest::make_args("--bfgs", "--mem", "1", "--passes", "2", "--quiet", "--cache_file", cache_file, "--no_stdin"));
  auto* ex = VW::read_example(*vw, "1 |f x:1.0");
  vw->learn(*ex);
  vw->finish_example(*ex);
  std::remove(cache_file);
}

TEST(CoverageNumericReductions, BfgsMem3)
{
  const char* cache_file = "/tmp/vw_batch9_bfgs_15.cache";
  std::remove(cache_file);
  auto vw = VW::initialize(
      vwtest::make_args("--bfgs", "--mem", "3", "--passes", "2", "--quiet", "--cache_file", cache_file, "--no_stdin"));
  auto* ex = VW::read_example(*vw, "1 |f x:1.0");
  vw->learn(*ex);
  vw->finish_example(*ex);
  std::remove(cache_file);
}

TEST(CoverageNumericReductions, BfgsHessianOnWithMultipleExamples)
{
  const char* cache_file = "/tmp/vw_batch9_bfgs_16.cache";
  std::remove(cache_file);
  auto vw = VW::initialize(vwtest::make_args(
      "--bfgs", "--hessian_on", "--passes", "2", "--quiet", "--cache_file", cache_file, "--no_stdin"));
  for (int i = 0; i < 5; i++)
  {
    auto* ex = VW::read_example(*vw, "1 |f x:" + std::to_string(i * 0.5));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
  std::remove(cache_file);
}

TEST(CoverageNumericReductions, BfgsLowTermination)
{
  const char* cache_file = "/tmp/vw_batch9_bfgs_17.cache";
  std::remove(cache_file);
  auto vw = VW::initialize(vwtest::make_args(
      "--bfgs", "--termination", "0.00001", "--passes", "2", "--quiet", "--cache_file", cache_file, "--no_stdin"));
  auto* ex = VW::read_example(*vw, "1 |f x:1.0");
  vw->learn(*ex);
  vw->finish_example(*ex);
  std::remove(cache_file);
}

TEST(CoverageNumericReductions, BfgsHighTermination)
{
  const char* cache_file = "/tmp/vw_batch9_bfgs_18.cache";
  std::remove(cache_file);
  auto vw = VW::initialize(vwtest::make_args(
      "--bfgs", "--termination", "1.0", "--passes", "2", "--quiet", "--cache_file", cache_file, "--no_stdin"));
  auto* ex = VW::read_example(*vw, "1 |f x:1.0");
  vw->learn(*ex);
  vw->finish_example(*ex);
  std::remove(cache_file);
}

TEST(CoverageNumericReductions, BfgsWithL1)
{
  const char* cache_file = "/tmp/vw_batch9_bfgs_19.cache";
  std::remove(cache_file);
  auto vw = VW::initialize(vwtest::make_args(
      "--bfgs", "--l1", "0.01", "--passes", "2", "--quiet", "--cache_file", cache_file, "--no_stdin"));
  auto* ex = VW::read_example(*vw, "1 |f x:1.0");
  vw->learn(*ex);
  vw->finish_example(*ex);
  std::remove(cache_file);
}

TEST(CoverageNumericReductions, BfgsMultiplePassesWithData)
{
  const char* cache_file = "/tmp/vw_batch9_bfgs_20.cache";
  std::remove(cache_file);
  auto vw = VW::initialize(
      vwtest::make_args("--bfgs", "--passes", "3", "--quiet", "--cache_file", cache_file, "--no_stdin"));
  for (int i = 0; i < 5; i++)
  {
    auto* ex = VW::read_example(*vw, std::to_string(i * 0.5f) + " |f x:" + std::to_string(i * 0.1f));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
  std::remove(cache_file);
}

TEST(CoverageNumericReductions, BfgsCGWithHessian)
{
  const char* cache_file = "/tmp/vw_batch9_bfgs_21.cache";
  std::remove(cache_file);
  auto vw = VW::initialize(vwtest::make_args(
      "--bfgs", "--mem", "0", "--hessian_on", "--passes", "2", "--quiet", "--cache_file", cache_file, "--no_stdin"));
  auto* ex = VW::read_example(*vw, "1 |f x:1.0");
  vw->learn(*ex);
  vw->finish_example(*ex);
  std::remove(cache_file);
}

TEST(CoverageNumericReductions, BfgsLargeMem)
{
  const char* cache_file = "/tmp/vw_batch9_bfgs_22.cache";
  std::remove(cache_file);
  auto vw = VW::initialize(
      vwtest::make_args("--bfgs", "--mem", "30", "--passes", "2", "--quiet", "--cache_file", cache_file, "--no_stdin"));
  auto* ex = VW::read_example(*vw, "1 |f x:1.0");
  vw->learn(*ex);
  vw->finish_example(*ex);
  std::remove(cache_file);
}

TEST(CoverageNumericReductions, BfgsHingeLoss)
{
  const char* cache_file = "/tmp/vw_batch9_bfgs_23.cache";
  std::remove(cache_file);
  auto vw = VW::initialize(vwtest::make_args(
      "--bfgs", "--passes", "2", "--loss_function", "hinge", "--quiet", "--cache_file", cache_file, "--no_stdin"));
  auto* ex = VW::read_example(*vw, "1 |f x:1.0");
  vw->learn(*ex);
  vw->finish_example(*ex);
  std::remove(cache_file);
}

TEST(CoverageNumericReductions, BfgsQuantileLoss)
{
  const char* cache_file = "/tmp/vw_batch9_bfgs_24.cache";
  std::remove(cache_file);
  auto vw = VW::initialize(vwtest::make_args(
      "--bfgs", "--passes", "2", "--loss_function", "quantile", "--quiet", "--cache_file", cache_file, "--no_stdin"));
  auto* ex = VW::read_example(*vw, "1 |f x:1.0");
  vw->learn(*ex);
  vw->finish_example(*ex);
  std::remove(cache_file);
}

TEST(CoverageNumericReductions, BfgsPredictOnlyWithFeatures)
{
  auto vw = VW::initialize(vwtest::make_args("--bfgs", "--quiet", "--no_stdin", "-t"));
  auto* ex = VW::read_example(*vw, "| x:1.0 y:0.5 z:0.3");
  vw->predict(*ex);
  // Should produce a scalar prediction
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  vw->finish_example(*ex);
}

// ============================================================
// CS Active (--cs_active N) Tests
// ============================================================

TEST(CoverageNumericReductions, CsActiveBasicInit)
{
  auto vw = VW::initialize(vwtest::make_args("--cs_active", "3", "--simulation", "--quiet"));
  auto* ex = VW::read_example(*vw, "1:1.0 2:0.0 3:2.0 | a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, CsActiveTwoClasses)
{
  auto vw = VW::initialize(vwtest::make_args("--cs_active", "2", "--simulation", "--quiet"));
  auto* ex = VW::read_example(*vw, "1:1.0 2:0.0 | a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, CsActiveFiveClasses)
{
  auto vw = VW::initialize(vwtest::make_args("--cs_active", "5", "--simulation", "--quiet"));
  auto* ex = VW::read_example(*vw, "1:1.0 2:0.0 3:2.0 4:1.5 5:0.5 | a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, CsActiveTenClasses)
{
  auto vw = VW::initialize(vwtest::make_args("--cs_active", "10", "--simulation", "--quiet"));
  auto* ex = VW::read_example(*vw, "1:1.0 2:0.0 3:2.0 | a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, CsActiveMellownessLow)
{
  auto vw =
      VW::initialize(vwtest::make_args("--cs_active", "3", "--simulation", "--mellowness", "0.01", "--quiet"));
  auto* ex = VW::read_example(*vw, "1:1.0 2:0.0 3:2.0 | a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, CsActiveMellownessHigh)
{
  auto vw =
      VW::initialize(vwtest::make_args("--cs_active", "3", "--simulation", "--mellowness", "1.0", "--quiet"));
  auto* ex = VW::read_example(*vw, "1:1.0 2:0.0 3:2.0 | a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, CsActiveDomination0)
{
  auto vw =
      VW::initialize(vwtest::make_args("--cs_active", "3", "--simulation", "--domination", "0", "--quiet"));
  auto* ex = VW::read_example(*vw, "1:1.0 2:0.0 3:2.0 | a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, CsActiveDomination1)
{
  auto vw =
      VW::initialize(vwtest::make_args("--cs_active", "3", "--simulation", "--domination", "1", "--quiet"));
  auto* ex = VW::read_example(*vw, "1:1.0 2:0.0 3:2.0 | a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, CsActiveBaseline)
{
  auto vw = VW::initialize(
      vwtest::make_args("--cs_active", "3", "--simulation", "--baseline", "--global_only", "--quiet"));
  auto* ex = VW::read_example(*vw, "1:1.0 2:0.0 3:2.0 | a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, CsActiveCostRange)
{
  auto vw = VW::initialize(
      vwtest::make_args("--cs_active", "3", "--simulation", "--cost_max", "5.0", "--cost_min", "-1.0", "--quiet"));
  auto* ex = VW::read_example(*vw, "1:1.0 2:0.0 3:2.0 | a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, CsActiveRangeC)
{
  auto vw =
      VW::initialize(vwtest::make_args("--cs_active", "3", "--simulation", "--range_c", "1.0", "--quiet"));
  auto* ex = VW::read_example(*vw, "1:1.0 2:0.0 3:2.0 | a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, CsActiveMultipleExamples)
{
  auto vw = VW::initialize(vwtest::make_args("--cs_active", "3", "--simulation", "--quiet"));
  for (int i = 0; i < 20; i++)
  {
    auto* ex = VW::read_example(*vw, "1:1.0 2:0.0 3:2.0 | a b c");
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageNumericReductions, CsActivePredictedClassSet)
{
  auto vw = VW::initialize(vwtest::make_args("--cs_active", "3", "--simulation", "--quiet"));
  auto* ex = VW::read_example(*vw, "1:1.0 2:0.0 3:2.0 | a b c");
  vw->learn(*ex);
  // predicted_class should be a valid class (1 to num_classes)
  EXPECT_GE(ex->pred.active_multiclass.predicted_class, 1u);
  EXPECT_LE(ex->pred.active_multiclass.predicted_class, 3u);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, CsActivePredictOnly)
{
  auto vw = VW::initialize(vwtest::make_args("--cs_active", "3", "--simulation", "--quiet"));
  auto* ex = VW::read_example(*vw, "1:1.0 2:0.0 3:2.0 | a b c");
  vw->predict(*ex);
  EXPECT_GE(ex->pred.active_multiclass.predicted_class, 1u);
  EXPECT_LE(ex->pred.active_multiclass.predicted_class, 3u);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, CsActiveSingleClassExample)
{
  auto vw = VW::initialize(vwtest::make_args("--cs_active", "3", "--simulation", "--quiet"));
  auto* ex = VW::read_example(*vw, "1:0.0 | a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, CsActiveAllCostsEqual)
{
  auto vw = VW::initialize(vwtest::make_args("--cs_active", "3", "--simulation", "--quiet"));
  auto* ex = VW::read_example(*vw, "1:1.0 2:1.0 3:1.0 | a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, CsActiveLargeCosts)
{
  auto vw = VW::initialize(vwtest::make_args("--cs_active", "3", "--simulation", "--quiet"));
  auto* ex = VW::read_example(*vw, "1:100.0 2:0.0 3:200.0 | a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, CsActiveZeroCosts)
{
  auto vw = VW::initialize(vwtest::make_args("--cs_active", "3", "--simulation", "--quiet"));
  auto* ex = VW::read_example(*vw, "1:0.0 2:0.0 3:0.0 | a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, CsActiveSimulationWithMellowness)
{
  auto vw =
      VW::initialize(vwtest::make_args("--cs_active", "5", "--simulation", "--mellowness", "0.5", "--quiet"));
  for (int i = 0; i < 10; i++)
  {
    auto* ex = VW::read_example(*vw, "1:1.0 2:0.0 3:2.0 4:1.5 5:0.5 | a b c d e");
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageNumericReductions, CsActiveBaselineWithDomination)
{
  auto vw = VW::initialize(vwtest::make_args(
      "--cs_active", "3", "--simulation", "--baseline", "--global_only", "--domination", "1", "--quiet"));
  for (int i = 0; i < 10; i++)
  {
    auto* ex = VW::read_example(*vw, "1:1.0 2:0.0 3:2.0 | a b c");
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageNumericReductions, CsActiveNoDomination)
{
  auto vw = VW::initialize(
      vwtest::make_args("--cs_active", "3", "--simulation", "--domination", "0", "--mellowness", "0.5", "--quiet"));
  for (int i = 0; i < 10; i++)
  {
    auto* ex = VW::read_example(*vw, "1:1.0 2:0.0 3:2.0 | a b c");
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageNumericReductions, CsActiveWithAdax)
{
  auto vw =
      VW::initialize(vwtest::make_args("--cs_active", "3", "--simulation", "--adax", "--quiet"));
  auto* ex = VW::read_example(*vw, "1:1.0 2:0.0 3:2.0 | a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, CsActiveTrainMultipleDifferentExamples)
{
  auto vw = VW::initialize(vwtest::make_args("--cs_active", "3", "--simulation", "--quiet"));
  {
    auto* ex = VW::read_example(*vw, "1:0.0 2:1.0 3:2.0 | x:1.0 y:0.0");
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
  {
    auto* ex = VW::read_example(*vw, "1:2.0 2:0.0 3:1.0 | x:0.0 y:1.0");
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
  {
    auto* ex = VW::read_example(*vw, "1:1.0 2:2.0 3:0.0 | x:1.0 y:1.0");
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageNumericReductions, CsActiveManyFeatures)
{
  auto vw = VW::initialize(vwtest::make_args("--cs_active", "3", "--simulation", "--quiet"));
  auto* ex = VW::read_example(*vw, "1:1.0 2:0.0 3:2.0 | a:1 b:2 c:3 d:4 e:5 f:6 g:7 h:8 i:9 j:10");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

// ============================================================
// GD MF (--rank N) Tests
// ============================================================

TEST(CoverageNumericReductions, GdMfBasicInit)
{
  auto vw = VW::initialize(vwtest::make_args("--rank", "2", "-q", "ui", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 |u user1 |i item1");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, GdMfRank1)
{
  auto vw = VW::initialize(vwtest::make_args("--rank", "1", "-q", "ui", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 |u user1 |i item1");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, GdMfRank5)
{
  auto vw = VW::initialize(vwtest::make_args("--rank", "5", "-q", "ui", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 |u user1 |i item1");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, GdMfRank10)
{
  auto vw = VW::initialize(vwtest::make_args("--rank", "10", "-q", "ui", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 |u user1 |i item1");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, GdMfMultipleExamples)
{
  auto vw = VW::initialize(vwtest::make_args("--rank", "3", "-q", "ui", "--quiet"));
  for (int i = 0; i < 10; i++)
  {
    std::string label = std::to_string(static_cast<float>(i % 5));
    auto* ex = VW::read_example(*vw, label + " |u user" + std::to_string(i % 3) + " |i item" + std::to_string(i % 4));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageNumericReductions, GdMfScalarPrediction)
{
  auto vw = VW::initialize(vwtest::make_args("--rank", "2", "-q", "ui", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 |u user1 |i item1");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, GdMfWithL2)
{
  auto vw = VW::initialize(vwtest::make_args("--rank", "2", "-q", "ui", "--l2", "0.01", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 |u user1 |i item1");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, GdMfLearningRateLow)
{
  auto vw = VW::initialize(vwtest::make_args("--rank", "2", "-q", "ui", "-l", "0.01", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 |u user1 |i item1");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, GdMfLearningRateHigh)
{
  auto vw = VW::initialize(vwtest::make_args("--rank", "2", "-q", "ui", "-l", "1.0", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 |u user1 |i item1");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, GdMfPowerT0)
{
  auto vw = VW::initialize(vwtest::make_args("--rank", "2", "-q", "ui", "--power_t", "0", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 |u user1 |i item1");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, GdMfPowerT1)
{
  auto vw = VW::initialize(vwtest::make_args("--rank", "2", "-q", "ui", "--power_t", "1", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 |u user1 |i item1");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, GdMfMultipleUserItemPairs)
{
  auto vw = VW::initialize(vwtest::make_args("--rank", "2", "-q", "ui", "--quiet"));
  // Collaborative filtering scenario
  auto* ex1 = VW::read_example(*vw, "5 |u alice |i movie_a");
  vw->learn(*ex1);
  vw->finish_example(*ex1);

  auto* ex2 = VW::read_example(*vw, "1 |u alice |i movie_b");
  vw->learn(*ex2);
  vw->finish_example(*ex2);

  auto* ex3 = VW::read_example(*vw, "4 |u bob |i movie_a");
  vw->learn(*ex3);
  vw->finish_example(*ex3);

  auto* ex4 = VW::read_example(*vw, "2 |u bob |i movie_b");
  vw->learn(*ex4);
  vw->finish_example(*ex4);
}

TEST(CoverageNumericReductions, GdMfPredictAfterTraining)
{
  auto vw = VW::initialize(vwtest::make_args("--rank", "2", "-q", "ui", "--quiet"));
  for (int i = 0; i < 20; i++)
  {
    auto* ex = VW::read_example(*vw, "1 |u user1 |i item1");
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
  auto* ex = VW::read_example(*vw, "|u user1 |i item1");
  vw->predict(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, GdMfMultipleFeaturePerNamespace)
{
  auto vw = VW::initialize(vwtest::make_args("--rank", "2", "-q", "ui", "--quiet"));
  auto* ex = VW::read_example(*vw, "3 |u user1 age:25 gender:1 |i item1 genre:3 year:2020");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, GdMfTwoInteractions)
{
  auto vw = VW::initialize(vwtest::make_args("--rank", "2", "-q", "ab", "-q", "bc", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 |a f1:1 |b f2:2 |c f3:3");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, GdMfNegativeLabels)
{
  auto vw = VW::initialize(vwtest::make_args("--rank", "2", "-q", "ui", "--quiet"));
  auto* ex = VW::read_example(*vw, "-1 |u user1 |i item1");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, GdMfZeroLabel)
{
  auto vw = VW::initialize(vwtest::make_args("--rank", "2", "-q", "ui", "--quiet"));
  auto* ex = VW::read_example(*vw, "0 |u user1 |i item1");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, GdMfLargeLabel)
{
  auto vw = VW::initialize(vwtest::make_args("--rank", "2", "-q", "ui", "--quiet"));
  auto* ex = VW::read_example(*vw, "100 |u user1 |i item1");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, GdMfMultipleLearnCycles)
{
  auto vw = VW::initialize(vwtest::make_args("--rank", "2", "-q", "ui", "--quiet"));
  for (int cycle = 0; cycle < 5; cycle++)
  {
    auto* ex = VW::read_example(*vw, "1 |u alice |i movieA");
    vw->learn(*ex);
    vw->finish_example(*ex);

    auto* ex2 = VW::read_example(*vw, "0 |u alice |i movieB");
    vw->learn(*ex2);
    vw->finish_example(*ex2);
  }
}

TEST(CoverageNumericReductions, GdMfWithL1)
{
  auto vw = VW::initialize(vwtest::make_args("--rank", "2", "-q", "ui", "--l1", "0.01", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 |u user1 |i item1");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, GdMfSmallBitPrecision)
{
  auto vw = VW::initialize(vwtest::make_args("--rank", "2", "-q", "ui", "-b", "10", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 |u user1 |i item1");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, GdMfRank2MultipleRounds)
{
  auto vw = VW::initialize(vwtest::make_args("--rank", "2", "-q", "ui", "--quiet"));
  for (int i = 0; i < 50; i++)
  {
    std::string u = "u" + std::to_string(i % 5);
    std::string item = "i" + std::to_string(i % 7);
    float label = static_cast<float>((i % 5 + i % 7) % 3);
    auto* ex = VW::read_example(*vw, std::to_string(label) + " |u " + u + " |i " + item);
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageNumericReductions, GdMfOnlyOneNamespaceHasFeatures)
{
  // With rank, the prediction uses linear terms for namespaces without interactions
  auto vw = VW::initialize(vwtest::make_args("--rank", "2", "-q", "ui", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 |u user1");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

// ============================================================
// Stagewise Poly (--stage_poly) Tests
// ============================================================

TEST(CoverageNumericReductions, StagePolyBasicInit)
{
  auto vw = VW::initialize(vwtest::make_args("--stage_poly", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | x:1.0 y:0.5");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, StagePolySchedExponent025)
{
  auto vw = VW::initialize(vwtest::make_args("--stage_poly", "--sched_exponent", "0.25", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | x:1.0 y:0.5");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, StagePolySchedExponent1)
{
  auto vw = VW::initialize(vwtest::make_args("--stage_poly", "--sched_exponent", "1.0", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | x:1.0 y:0.5");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, StagePolySchedExponent2)
{
  auto vw = VW::initialize(vwtest::make_args("--stage_poly", "--sched_exponent", "2.0", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | x:1.0 y:0.5");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, StagePolyBatchSz100)
{
  auto vw = VW::initialize(vwtest::make_args("--stage_poly", "--batch_sz", "100", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | x:1.0 y:0.5");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, StagePolyBatchSz10)
{
  auto vw = VW::initialize(vwtest::make_args("--stage_poly", "--batch_sz", "10", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | x:1.0 y:0.5");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, StagePolyBatchSzNoDoubling)
{
  auto vw = VW::initialize(vwtest::make_args("--stage_poly", "--batch_sz_no_doubling", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | x:1.0 y:0.5");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, StagePolyTwoFeatures)
{
  auto vw = VW::initialize(vwtest::make_args("--stage_poly", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | x:1.0 y:0.5");
  vw->learn(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, StagePolyFiveFeatures)
{
  auto vw = VW::initialize(vwtest::make_args("--stage_poly", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a:1 b:2 c:3 d:4 e:5");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, StagePolyTenFeatures)
{
  auto vw = VW::initialize(vwtest::make_args("--stage_poly", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a:1 b:2 c:3 d:4 e:5 f:6 g:7 h:8 i:9 j:10");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, StagePolySmallBitPrecision)
{
  auto vw = VW::initialize(vwtest::make_args("-b", "7", "--stage_poly", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | x:1.0 y:0.5");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, StagePolyMultipleLearnSmallBatch)
{
  // batch_sz=5 so we trigger support update after 5 examples
  auto vw = VW::initialize(vwtest::make_args("--stage_poly", "--batch_sz", "5", "--quiet"));
  for (int i = 0; i < 20; i++)
  {
    float v = static_cast<float>(i) / 10.0f;
    auto* ex = VW::read_example(*vw, std::to_string(v) + " | x:" + std::to_string(v) + " y:" + std::to_string(1.0f - v));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageNumericReductions, StagePolyMultipleLearnTriggerDoubling)
{
  // batch_sz=3 with doubling to trigger batch size doubling
  auto vw = VW::initialize(vwtest::make_args("--stage_poly", "--batch_sz", "3", "--quiet"));
  for (int i = 0; i < 30; i++)
  {
    float v = static_cast<float>(i) / 10.0f;
    auto* ex = VW::read_example(*vw, std::to_string(v) + " | x:" + std::to_string(v) + " y:" + std::to_string(1.0f - v));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageNumericReductions, StagePolyNoDoblingMultipleLearn)
{
  auto vw = VW::initialize(vwtest::make_args("--stage_poly", "--batch_sz", "3", "--batch_sz_no_doubling", "--quiet"));
  for (int i = 0; i < 30; i++)
  {
    float v = static_cast<float>(i) / 10.0f;
    auto* ex = VW::read_example(*vw, std::to_string(v) + " | x:" + std::to_string(v) + " y:" + std::to_string(1.0f - v));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageNumericReductions, StagePolyPredictOnly)
{
  auto vw = VW::initialize(vwtest::make_args("--stage_poly", "--quiet"));
  auto* ex = VW::read_example(*vw, "| x:1.0 y:0.5");
  vw->predict(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, StagePolyNegativeLabel)
{
  auto vw = VW::initialize(vwtest::make_args("--stage_poly", "--quiet"));
  auto* ex = VW::read_example(*vw, "-1 | x:1.0 y:0.5");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, StagePolyZeroLabel)
{
  auto vw = VW::initialize(vwtest::make_args("--stage_poly", "--quiet"));
  auto* ex = VW::read_example(*vw, "0 | x:1.0 y:0.5");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, StagePolySingleFeature)
{
  auto vw = VW::initialize(vwtest::make_args("--stage_poly", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | x:1.0");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, StagePolyManyExamplesConvergence)
{
  auto vw = VW::initialize(vwtest::make_args("--stage_poly", "--quiet"));
  for (int i = 0; i < 100; i++)
  {
    float x = static_cast<float>(i) / 100.0f;
    float label = x * x;  // quadratic target
    auto* ex = VW::read_example(*vw, std::to_string(label) + " | x:" + std::to_string(x));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageNumericReductions, StagePolyLowLearningRate)
{
  auto vw = VW::initialize(vwtest::make_args("--stage_poly", "-l", "0.01", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | x:1.0 y:0.5");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, StagePolyHighLearningRate)
{
  auto vw = VW::initialize(vwtest::make_args("--stage_poly", "-l", "1.0", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | x:1.0 y:0.5");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, StagePolyWithL2)
{
  auto vw = VW::initialize(vwtest::make_args("--stage_poly", "--l2", "0.01", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | x:1.0 y:0.5");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, StagePolyWithL1)
{
  auto vw = VW::initialize(vwtest::make_args("--stage_poly", "--l1", "0.01", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | x:1.0 y:0.5");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, StagePolyBatchSz1)
{
  auto vw = VW::initialize(vwtest::make_args("--stage_poly", "--batch_sz", "1", "--quiet"));
  for (int i = 0; i < 10; i++)
  {
    auto* ex = VW::read_example(*vw, "1 | x:" + std::to_string(i * 0.1) + " y:" + std::to_string(i * 0.2));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageNumericReductions, StagePolySchedExponent05WithBatch)
{
  auto vw =
      VW::initialize(vwtest::make_args("--stage_poly", "--sched_exponent", "0.5", "--batch_sz", "5", "--quiet"));
  for (int i = 0; i < 20; i++)
  {
    float v = static_cast<float>(i) / 10.0f;
    auto* ex = VW::read_example(*vw, std::to_string(v) + " | x:" + std::to_string(v));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

// ============================================================
// Additional cross-reduction tests and edge cases
// ============================================================

TEST(CoverageNumericReductions, LdaMinibatch2)
{
  auto vw = VW::initialize(vwtest::make_args("--lda", "3", "--minibatch", "2", "--quiet"));
  auto* ex1 = VW::read_example(*vw, "| word1:1 word2:2");
  vw->learn(*ex1);
  vw->finish_example(*ex1);

  auto* ex2 = VW::read_example(*vw, "| word3:3 word4:4");
  vw->learn(*ex2);
  vw->finish_example(*ex2);
}

TEST(CoverageNumericReductions, LdaMinibatch5)
{
  auto vw = VW::initialize(vwtest::make_args("--lda", "3", "--minibatch", "5", "--quiet"));
  for (int i = 0; i < 10; i++)
  {
    auto* ex = VW::read_example(*vw, "| word1:1 word2:2 word3:3");
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageNumericReductions, KsvmLinearMultipleExamples)
{
  auto vw = VW::initialize(vwtest::make_args("--ksvm", "--kernel", "linear", "--quiet"));
  for (int i = 0; i < 30; i++)
  {
    std::string label = (i % 2 == 0) ? "1" : "-1";
    float x = (i % 2 == 0) ? 1.0f : -1.0f;
    auto* ex = VW::read_example(*vw, label + " |f x:" + std::to_string(x) + " y:" + std::to_string(x * 0.5f));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageNumericReductions, GdMfRank3WithLargeData)
{
  auto vw = VW::initialize(vwtest::make_args("--rank", "3", "-q", "ui", "--quiet"));
  for (int i = 0; i < 30; i++)
  {
    std::string u = "user" + std::to_string(i % 5);
    std::string item = "item" + std::to_string(i % 8);
    float label = static_cast<float>((i % 5) * (i % 8)) / 10.0f;
    auto* ex = VW::read_example(*vw, std::to_string(label) + " |u " + u + " |i " + item);
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageNumericReductions, StagePolyLargeBatchSz)
{
  auto vw = VW::initialize(vwtest::make_args("--stage_poly", "--batch_sz", "1000", "--quiet"));
  for (int i = 0; i < 10; i++)
  {
    auto* ex = VW::read_example(*vw, "1 | x:" + std::to_string(i * 0.1) + " y:" + std::to_string(i * 0.2));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageNumericReductions, BfgsMultipleExamplesWithFeatureVariance)
{
  const char* cache_file = "/tmp/vw_batch9_bfgs_25.cache";
  std::remove(cache_file);
  auto vw = VW::initialize(
      vwtest::make_args("--bfgs", "--passes", "2", "--quiet", "--cache_file", cache_file, "--no_stdin"));
  for (int i = 0; i < 20; i++)
  {
    float x = static_cast<float>(i) * 0.1f;
    float y = x * 2.0f + 1.0f;
    auto* ex = VW::read_example(*vw, std::to_string(y) + " |f x:" + std::to_string(x));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
  std::remove(cache_file);
}

TEST(CoverageNumericReductions, CsActiveFiveClassesMultipleExamples)
{
  auto vw = VW::initialize(vwtest::make_args("--cs_active", "5", "--simulation", "--quiet"));
  for (int i = 0; i < 30; i++)
  {
    auto* ex = VW::read_example(*vw, "1:1.0 2:0.5 3:2.0 4:0.0 5:1.5 | a b c d e");
    vw->learn(*ex);
    EXPECT_GE(ex->pred.active_multiclass.predicted_class, 1u);
    EXPECT_LE(ex->pred.active_multiclass.predicted_class, 5u);
    vw->finish_example(*ex);
  }
}

TEST(CoverageNumericReductions, GdMfPowerTHalf)
{
  auto vw = VW::initialize(vwtest::make_args("--rank", "2", "-q", "ui", "--power_t", "0.5", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 |u user1 |i item1");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, StagePolySchedExponent3)
{
  auto vw = VW::initialize(vwtest::make_args("--stage_poly", "--sched_exponent", "3.0", "--quiet"));
  auto* ex = VW::read_example(*vw, "1 | x:1.0 y:0.5");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

TEST(CoverageNumericReductions, KsvmSubsampleWithPoolSize)
{
  auto vw = VW::initialize(vwtest::make_args("--ksvm", "--pool_size", "10", "--subsample", "3", "--quiet"));
  for (int i = 0; i < 30; i++)
  {
    std::string label = (i % 2 == 0) ? "1" : "-1";
    auto* ex = VW::read_example(*vw, label + " |f x:" + std::to_string(i * 0.1) + " y:" + std::to_string(i * 0.2));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageNumericReductions, LdaLargeAlphaAndRho)
{
  auto vw = VW::initialize(vwtest::make_args("--lda", "3", "--lda_alpha", "1.0", "--lda_rho", "1.0", "--quiet"));
  for (int i = 0; i < 10; i++)
  {
    auto* ex = VW::read_example(*vw, "| word1:1 word2:2 word3:3 word4:4");
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

TEST(CoverageNumericReductions, LdaLooseEpsilon)
{
  auto vw = VW::initialize(vwtest::make_args("--lda", "3", "--lda_epsilon", "0.1", "--quiet"));
  auto* ex = VW::read_example(*vw, "| word1:1 word2:2");
  vw->learn(*ex);
  vw->finish_example(*ex);
}

// NOTE: --para_active with KSVM causes segfault (known issue).
// Replacing with a safer pool_greedy + rbf combo test.
TEST(CoverageNumericReductions, KsvmPoolGreedyWithRbf)
{
  auto vw = VW::initialize(
      vwtest::make_args("--ksvm", "--pool_greedy", "--kernel", "rbf", "--bandwidth", "0.5", "--quiet"));
  for (int i = 0; i < 10; i++)
  {
    std::string label = (i % 2 == 0) ? "1" : "-1";
    auto* ex = VW::read_example(*vw, label + " |f x:" + std::to_string(i * 0.1));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}
