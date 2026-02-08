// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

// Batch 11: Targeted tests for Text & Core Parsers coverage.

#include "vw/core/example.h"
#include "vw/core/parse_primitives.h"
#include "vw/core/vw.h"
#include "vw/test_common/test_common.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cfloat>
#include <cmath>
#include <cstdio>
#include <sstream>
#include <string>
#include <vector>

// ============================================================
// 1. Text Parser Edge Cases (~40 tests)
// ============================================================

TEST(CoverageTextParser, TextParserEmptyLine)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin"));
  auto* ex = VW::read_example(*vw, "");
  EXPECT_TRUE(ex->is_newline);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, TextParserPipeOnly)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin"));
  auto* ex = VW::read_example(*vw, "|");
  // A pipe with nothing after it creates default namespace with no features
  EXPECT_FALSE(ex->is_newline);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, TextParserMultiplePipes)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin"));
  auto* ex = VW::read_example(*vw, "| a b |x c d |y e f");
  // Default namespace and x and y namespaces
  EXPECT_GE(ex->indices.size(), 2);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, TextParserDoublePipes)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin"));
  // Two consecutive pipes - second pipe starts a new (empty) namespace
  auto* ex = VW::read_example(*vw, "|| a b c");
  EXPECT_FALSE(ex->is_newline);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, TextParserNamespaceWithColon)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin"));
  // Namespace with a channel value
  auto* ex = VW::read_example(*vw, "|ns:2.0 a b c");
  // The features should be multiplied by 2.0
  EXPECT_FALSE(ex->is_newline);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, TextParserNamespaceColonZero)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin"));
  // Namespace channel value of 0 should zero out features
  auto* ex = VW::read_example(*vw, "|ns:0.0 a b c");
  EXPECT_FALSE(ex->is_newline);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, TextParserNamespaceColonNegative)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin"));
  auto* ex = VW::read_example(*vw, "|ns:-1.5 a b c");
  EXPECT_FALSE(ex->is_newline);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, TextParserFeatureWithColonValue)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin"));
  auto* ex = VW::read_example(*vw, "| feature1:3.5 feature2:-1.0 feature3:0.0");
  EXPECT_EQ(ex->feature_space[' '].size(), 2);  // feature3:0.0 should be excluded
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, TextParserFeatureColonLargeValue)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin"));
  auto* ex = VW::read_example(*vw, "| big:1e10 small:1e-10");
  EXPECT_EQ(ex->feature_space[' '].size(), 2);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, TextParserFeatureWithoutValue)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin"));
  auto* ex = VW::read_example(*vw, "| cat dog bird");
  // Features without values default to 1.0
  EXPECT_EQ(ex->feature_space[' '].size(), 3);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, TextParserAnonymousFeatures)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin"));
  // Features with just colon and value (no name)
  auto* ex = VW::read_example(*vw, "| :1.0 :2.0 :3.0");
  EXPECT_EQ(ex->feature_space[' '].size(), 3);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, TextParserTagParsing)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin"));
  auto* ex = VW::read_example(*vw, "1 'my_tag | a b c");
  EXPECT_EQ(std::string(ex->tag.begin(), ex->tag.end()), "my_tag");
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, TextParserTagParsingNoSpace)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin"));
  auto* ex = VW::read_example(*vw, "1 'tag_no_space| a b c");
  // The tag field is separated from features by the pipe
  EXPECT_FALSE(ex->tag.empty());
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, TextParserEmptyTag)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin"));
  auto* ex = VW::read_example(*vw, "1 '| a b c");
  // Empty tag (just apostrophe before pipe)
  EXPECT_TRUE(ex->tag.empty());
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, TextParserLabelOnly)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin"));
  auto* ex = VW::read_example(*vw, "1");
  // Label without features is valid
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, TextParserLabelWithWeight)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin"));
  auto* ex = VW::read_example(*vw, "1 2.0 | a b");
  // weight = 2.0
  EXPECT_FLOAT_EQ(ex->weight, 2.0f);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, TextParserNegativeLabel)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin"));
  auto* ex = VW::read_example(*vw, "-1 | a b");
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, TextParserLabelWithInitialPrediction)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin"));
  auto* ex = VW::read_example(*vw, "1 1.0 0.5 | a b");
  // label weight initial
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, TextParserTabSeparatedLabel)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin"));
  // Tab before features is treated as label/features separator
  auto* ex = VW::read_example(*vw, "1\t| a b c");
  EXPECT_FALSE(ex->is_newline);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, TextParserMultipleSpacesBetweenFeatures)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin"));
  auto* ex = VW::read_example(*vw, "|   a   b   c   ");
  EXPECT_EQ(ex->feature_space[' '].size(), 3);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, TextParserTrailingSpaces)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin"));
  auto* ex = VW::read_example(*vw, "| a b c   ");
  EXPECT_EQ(ex->feature_space[' '].size(), 3);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, TextParserTabsAsSeparators)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin"));
  auto* ex = VW::read_example(*vw, "|\ta\tb\tc");
  EXPECT_EQ(ex->feature_space[' '].size(), 3);
  VW::finish_example(*vw, *ex);
}

// Audit mode triggers the audit template specialization in tc_parser
TEST(CoverageTextParser, AuditModeBasic)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "-a"));
  auto* ex = VW::read_example(*vw, "1 |ns feature1:1.0 feature2:2.0");
  vw->learn(*ex);
  // In audit mode, space_names should be populated
  EXPECT_FALSE(ex->feature_space['n'].space_names.empty());
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, AuditModeDefaultNamespace)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "-a"));
  auto* ex = VW::read_example(*vw, "1 | feature1:1.0");
  vw->learn(*ex);
  EXPECT_FALSE(ex->feature_space[' '].space_names.empty());
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, AuditModeMultipleNamespaces)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "-a"));
  auto* ex = VW::read_example(*vw, "1 |a f1:1.0 |b f2:2.0 |c f3:3.0");
  vw->learn(*ex);
  EXPECT_FALSE(ex->feature_space['a'].space_names.empty());
  EXPECT_FALSE(ex->feature_space['b'].space_names.empty());
  EXPECT_FALSE(ex->feature_space['c'].space_names.empty());
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, AuditModeAnonymousFeatures)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "-a"));
  auto* ex = VW::read_example(*vw, "1 | :1.0 :2.0");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, HashInvMode)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "--invert_hash", "/dev/null"));
  auto* ex = VW::read_example(*vw, "1 |ns feature1:1.0 feature2:2.0");
  vw->learn(*ex);
  // hash_inv triggers the audit code path too
  EXPECT_FALSE(ex->feature_space['n'].space_names.empty());
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, TextParserScientificNotation)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin"));
  auto* ex = VW::read_example(*vw, "| f1:1.5e2 f2:3.0E-4 f3:-2.5e+1");
  EXPECT_EQ(ex->feature_space[' '].size(), 3);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, TextParserNegativeFeatureValue)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin"));
  auto* ex = VW::read_example(*vw, "| f1:-1.0 f2:-0.5 f3:-100");
  EXPECT_EQ(ex->feature_space[' '].size(), 3);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, TextParserSingleCharNamespace)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin"));
  auto* ex = VW::read_example(*vw, "|A f1 f2 |B f3 f4");
  EXPECT_EQ(ex->feature_space['A'].size(), 2);
  EXPECT_EQ(ex->feature_space['B'].size(), 2);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, TextParserLongNamespace)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin"));
  // Only the first char of namespace name determines the index
  auto* ex = VW::read_example(*vw, "|longnamespace f1 f2");
  EXPECT_EQ(ex->feature_space['l'].size(), 2);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, TextParserSameNamespaceTwice)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin"));
  auto* ex = VW::read_example(*vw, "|a f1 |a f2");
  // Same namespace twice should accumulate features
  EXPECT_EQ(ex->feature_space['a'].size(), 2);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, TextParserFeatureIntValues)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin"));
  auto* ex = VW::read_example(*vw, "| f1:1 f2:2 f3:3 f4:100");
  EXPECT_EQ(ex->feature_space[' '].size(), 4);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, TextParserManyFeatures)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin"));
  std::string features = "|";
  for (int i = 0; i < 100; i++) { features += " f" + std::to_string(i) + ":" + std::to_string(i * 0.01); }
  auto* ex = VW::read_example(*vw, features);
  // f0:0.0 will be excluded (zero value)
  EXPECT_EQ(ex->feature_space[' '].size(), 99);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, TextParserLearnAndPredict)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin"));
  // Learn some examples
  for (int i = 0; i < 10; i++)
  {
    auto* ex = VW::read_example(*vw, std::to_string(i % 2 == 0 ? 1 : -1) + " | f1:" + std::to_string(i * 0.1));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
  // Predict
  auto* ex = VW::read_example(*vw, "| f1:0.5");
  vw->predict(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, TextParserNamespaceChannelValueLarge)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin"));
  auto* ex = VW::read_example(*vw, "|ns:100.0 a:1.0 b:1.0");
  EXPECT_EQ(ex->feature_space['n'].size(), 2);
  VW::finish_example(*vw, *ex);
}

// ============================================================
// 2. Parser Dispatch (~30 tests)
// ============================================================

TEST(CoverageTextParser, ParserCacheFileCreation)
{
  const char* cache_file = "/tmp/vw_batch11_cache_1.cache";
  std::remove(cache_file);
  auto vw = VW::initialize(
      vwtest::make_args("--quiet", "--no_stdin", "--cache_file", cache_file, "--passes", "1"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
  // Workspace destructor will close the cache
  vw.reset();
  std::remove(cache_file);
}

TEST(CoverageTextParser, ParserKillCacheFlag)
{
  const char* cache_file = "/tmp/vw_batch11_cache_2.cache";
  std::remove(cache_file);
  // First, create a cache
  {
    auto vw = VW::initialize(
        vwtest::make_args("--quiet", "--no_stdin", "--cache_file", cache_file));
    auto* ex = VW::read_example(*vw, "1 | a b c");
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
  // Now, --kill_cache should force recreation
  {
    auto vw = VW::initialize(
        vwtest::make_args("--quiet", "--no_stdin", "--cache_file", cache_file, "-k"));
    auto* ex = VW::read_example(*vw, "1 | a b c");
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
  std::remove(cache_file);
}

TEST(CoverageTextParser, ParserNoStdinMode)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, ParserTextReaderSetup)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin"));
  // Default text reader should be set up
  auto* ex = VW::read_example(*vw, "1 |a f1:1 |b f2:2");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, ParserMultipleExamples)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin"));
  for (int i = 0; i < 50; i++)
  {
    auto* ex = VW::read_example(*vw, std::to_string(i % 2 == 0 ? 1.0f : -1.0f) + " | f:" + std::to_string(i));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

TEST(CoverageTextParser, ParserSortFeatures)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "--sort_features"));
  auto* ex = VW::read_example(*vw, "1 | c b a z y x");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, ParserSmallBitPrecision)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "-b", "10"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, ParserLargeBitPrecision)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "-b", "24"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, ParserHashAll)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "--hash", "all"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, ParserHashSeed)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "--hash_seed", "42"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, ParserHashSeedDifferent)
{
  // Different hash seeds should produce different hashes
  auto vw1 = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "--hash_seed", "1"));
  auto vw2 = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "--hash_seed", "2"));

  auto* ex1 = VW::read_example(*vw1, "| a b c");
  auto* ex2 = VW::read_example(*vw2, "| a b c");

  // Feature hashes should differ
  EXPECT_NE(ex1->feature_space[' '].indices[0], ex2->feature_space[' '].indices[0]);

  VW::finish_example(*vw1, *ex1);
  VW::finish_example(*vw2, *ex2);
}

TEST(CoverageTextParser, ParserNewlineExample)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin"));
  auto* ex = VW::read_example(*vw, "");
  EXPECT_TRUE(ex->is_newline);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, ParserPredictOnlyMode)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "-t"));
  auto* ex = VW::read_example(*vw, "| a b c");
  vw->predict(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, ParserNoConstant)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "--noconstant"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, ParserConstantValue)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "-C", "5.0"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, ParserMinMaxPrediction)
{
  auto vw = VW::initialize(
      vwtest::make_args("--quiet", "--no_stdin", "--min_prediction", "-5.0", "--max_prediction", "5.0"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  EXPECT_GE(ex->pred.scalar, -5.0f);
  EXPECT_LE(ex->pred.scalar, 5.0f);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, ParserLossFunctionSquared)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "--loss_function", "squared"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, ParserLossFunctionLogistic)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "--loss_function", "logistic"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, ParserLossFunctionHinge)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "--loss_function", "hinge"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, ParserLossFunctionQuantile)
{
  auto vw = VW::initialize(
      vwtest::make_args("--quiet", "--no_stdin", "--loss_function", "quantile", "--quantile_tau", "0.3"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, ParserLossFunctionPoisson)
{
  auto vw = VW::initialize(
      vwtest::make_args("--quiet", "--no_stdin", "--loss_function", "poisson", "--min_prediction", "0.01"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, ParserLogisticLossMinMax)
{
  auto vw = VW::initialize(vwtest::make_args(
      "--quiet", "--no_stdin", "--loss_function", "logistic", "--logistic_min", "-2.0", "--logistic_max", "2.0"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, ParserL1Lambda)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "--l1", "0.001"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, ParserL2Lambda)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "--l2", "0.001"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, ParserNoBiasRegularization)
{
  auto vw =
      VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "--l2", "0.01", "--no_bias_regularization"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, ParserNamedLabels)
{
  auto vw =
      VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "--named_labels", "cat,dog,bird", "--oaa", "3"));
  auto* ex = VW::read_example(*vw, "cat | a b c");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, ParserCacheMultipass)
{
  const char* cache_file = "/tmp/vw_batch11_cache_mp.cache";
  std::remove(cache_file);
  auto vw = VW::initialize(
      vwtest::make_args("--quiet", "--no_stdin", "--cache_file", cache_file, "--passes", "2"));
  for (int i = 0; i < 5; i++)
  {
    auto* ex = VW::read_example(*vw, std::to_string(i % 2 == 0 ? 1.0f : -1.0f) + " | f:" + std::to_string(i));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
  vw.reset();
  std::remove(cache_file);
}

TEST(CoverageTextParser, ParserEarlyTerminate)
{
  const char* cache_file = "/tmp/vw_batch11_cache_et.cache";
  std::remove(cache_file);
  auto vw = VW::initialize(vwtest::make_args(
      "--quiet", "--no_stdin", "--cache_file", cache_file, "--passes", "3", "--early_terminate", "1"));
  for (int i = 0; i < 10; i++)
  {
    auto* ex = VW::read_example(*vw, "1 | a b c");
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
  vw.reset();
  std::remove(cache_file);
}

// ============================================================
// 3. Parse Args - Interaction Setup (~30 tests)
// ============================================================

TEST(CoverageTextParser, QuadraticInteraction)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "-q", "ab"));
  auto* ex = VW::read_example(*vw, "1 |a f1:1 |b f2:2");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, QuadraticInteractionSameNamespace)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "-q", "aa"));
  auto* ex = VW::read_example(*vw, "1 |a f1:1 f2:2 f3:3");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, QuadraticInteractionWildcard)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "-q", "::"));
  auto* ex = VW::read_example(*vw, "1 |a f1:1 |b f2:2 |c f3:3");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, CubicInteraction)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "--cubic", "abc"));
  auto* ex = VW::read_example(*vw, "1 |a f1:1 |b f2:2 |c f3:3");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, InteractionsFlag)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "--interactions", "abcd"));
  auto* ex = VW::read_example(*vw, "1 |a f1:1 |b f2:2 |c f3:3 |d f4:4");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, InteractionsQuadAndCubic)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "-q", "ab", "--cubic", "abc"));
  auto* ex = VW::read_example(*vw, "1 |a f1:1 |b f2:2 |c f3:3");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, InteractionsMultipleQuadratic)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "-q", "ab", "-q", "ac", "-q", "bc"));
  auto* ex = VW::read_example(*vw, "1 |a f1:1 |b f2:2 |c f3:3");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, InteractionsPermutations)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "-q", "ab", "--permutations"));
  auto* ex = VW::read_example(*vw, "1 |a f1:1 |b f2:2");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, InteractionsLeaveDuplicates)
{
  auto vw = VW::initialize(
      vwtest::make_args("--quiet", "--no_stdin", "-q", "ab", "-q", "ba", "--leave_duplicate_interactions"));
  auto* ex = VW::read_example(*vw, "1 |a f1:1 |b f2:2");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, IgnoreNamespace)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "--ignore", "b"));
  auto* ex = VW::read_example(*vw, "1 |a f1:1 |b f2:2 |c f3:3");
  vw->learn(*ex);
  // Namespace b should be ignored
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, IgnoreMultipleNamespaces)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "--ignore", "b", "--ignore", "c"));
  auto* ex = VW::read_example(*vw, "1 |a f1:1 |b f2:2 |c f3:3");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, IgnoreLinearNamespace)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "--ignore_linear", "a", "-q", "ab"));
  auto* ex = VW::read_example(*vw, "1 |a f1:1 |b f2:2");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, KeepNamespace)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "--keep", "a"));
  auto* ex = VW::read_example(*vw, "1 |a f1:1 |b f2:2 |c f3:3");
  vw->learn(*ex);
  // Only namespace 'a' (and default?) should be kept
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, KeepMultipleNamespaces)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "--keep", "a", "--keep", "c"));
  auto* ex = VW::read_example(*vw, "1 |a f1:1 |b f2:2 |c f3:3");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, RedefineNamespace)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "--redefine", "a:=b"));
  auto* ex = VW::read_example(*vw, "1 |b f1:1 f2:2");
  vw->learn(*ex);
  // Namespace 'b' should be redefined to 'a'
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, RedefineNamespaceDefault)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "--redefine", ":=b"));
  auto* ex = VW::read_example(*vw, "1 |b f1:1 f2:2");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, RedefineNamespaceWildcard)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "--redefine", "a:=:"));
  auto* ex = VW::read_example(*vw, "1 |b f1:1 |c f2:2");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, FeatureLimitPerNamespace)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "--feature_limit", "a3"));
  auto* ex = VW::read_example(*vw, "1 |a f1:1 f2:2 f3:3 f4:4 f5:5");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, FeatureLimitGlobal)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "--feature_limit", "5"));
  auto* ex = VW::read_example(*vw, "1 | f1:1 f2:2 f3:3 f4:4 f5:5 f6:6 f7:7 f8:8 f9:9 f10:10");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, NgramFeatures)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "--ngram", "2"));
  auto* ex = VW::read_example(*vw, "1 | a b c d");
  vw->learn(*ex);
  // Should have more features than original due to ngrams
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, NgramWithSkip)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "--ngram", "3", "--skips", "1"));
  auto* ex = VW::read_example(*vw, "1 | a b c d e");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, NgramPerNamespace)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "--ngram", "a2"));
  auto* ex = VW::read_example(*vw, "1 |a f1 f2 f3 |b g1 g2");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, HoldoutPeriod)
{
  const char* cache_file = "/tmp/vw_batch11_cache_holdout.cache";
  std::remove(cache_file);
  auto vw = VW::initialize(vwtest::make_args(
      "--quiet", "--no_stdin", "--holdout_period", "5", "--cache_file", cache_file, "--passes", "2"));
  for (int i = 0; i < 20; i++)
  {
    auto* ex = VW::read_example(*vw, "1 | a b c");
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
  vw.reset();
  std::remove(cache_file);
}

TEST(CoverageTextParser, HoldoutAfter)
{
  const char* cache_file = "/tmp/vw_batch11_cache_holdout2.cache";
  std::remove(cache_file);
  auto vw = VW::initialize(vwtest::make_args(
      "--quiet", "--no_stdin", "--holdout_after", "10", "--cache_file", cache_file, "--passes", "2"));
  for (int i = 0; i < 20; i++)
  {
    auto* ex = VW::read_example(*vw, "1 | a b c");
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
  vw.reset();
  std::remove(cache_file);
}

TEST(CoverageTextParser, HoldoutOff)
{
  const char* cache_file = "/tmp/vw_batch11_cache_holdout3.cache";
  std::remove(cache_file);
  auto vw = VW::initialize(vwtest::make_args(
      "--quiet", "--no_stdin", "--holdout_off", "--cache_file", cache_file, "--passes", "2"));
  for (int i = 0; i < 10; i++)
  {
    auto* ex = VW::read_example(*vw, "1 | a b c");
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
  vw.reset();
  std::remove(cache_file);
}

TEST(CoverageTextParser, SpellingFeatures)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "--spelling", "_"));
  auto* ex = VW::read_example(*vw, "1 | Hello123 World456");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, SpellingFeaturesNamespace)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "--spelling", "a"));
  auto* ex = VW::read_example(*vw, "1 |a Hello123 World456");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, SpellingFeaturesAudit)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "--spelling", "_", "-a"));
  auto* ex = VW::read_example(*vw, "1 | Hello123 World456");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, SpellingFeaturesNamespaceAudit)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "--spelling", "a", "-a"));
  auto* ex = VW::read_example(*vw, "1 |a Hello World");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, AffixFeaturePrefix)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "--affix", "+2"));
  auto* ex = VW::read_example(*vw, "1 | hello world testing");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, AffixFeatureSuffix)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "--affix", "-3"));
  auto* ex = VW::read_example(*vw, "1 | hello world testing");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, AffixFeatureBothWithNamespace)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "--affix", "+2a,-3b"));
  auto* ex = VW::read_example(*vw, "1 |a hello world |b testing features");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, AffixFeatureAudit)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "--affix", "+2", "-a"));
  auto* ex = VW::read_example(*vw, "1 | hello world");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, AffixFeatureNamespaceAudit)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "--affix", "+2a", "-a"));
  auto* ex = VW::read_example(*vw, "1 |a hello world");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

// ============================================================
// 4. Parse Primitives (~20 tests)
// ============================================================

TEST(CoverageTextParser, ParseFloatBasicInteger)
{
  size_t end_idx = 0;
  float result = VW::details::parse_float("42", end_idx);
  EXPECT_FLOAT_EQ(result, 42.0f);
  EXPECT_GT(end_idx, 0u);
}

TEST(CoverageTextParser, ParseFloatNegativeInteger)
{
  size_t end_idx = 0;
  float result = VW::details::parse_float("-42", end_idx);
  EXPECT_FLOAT_EQ(result, -42.0f);
}

TEST(CoverageTextParser, ParseFloatDecimal)
{
  size_t end_idx = 0;
  float result = VW::details::parse_float("3.14", end_idx);
  EXPECT_NEAR(result, 3.14f, 0.001f);
}

TEST(CoverageTextParser, ParseFloatNegativeDecimal)
{
  size_t end_idx = 0;
  float result = VW::details::parse_float("-0.5", end_idx);
  EXPECT_FLOAT_EQ(result, -0.5f);
}

TEST(CoverageTextParser, ParseFloatScientific)
{
  size_t end_idx = 0;
  float result = VW::details::parse_float("1.5e2", end_idx);
  EXPECT_FLOAT_EQ(result, 150.0f);
}

TEST(CoverageTextParser, ParseFloatScientificNegExp)
{
  size_t end_idx = 0;
  float result = VW::details::parse_float("1.5e-2", end_idx);
  EXPECT_NEAR(result, 0.015f, 0.0001f);
}

TEST(CoverageTextParser, ParseFloatScientificUpperE)
{
  size_t end_idx = 0;
  float result = VW::details::parse_float("3.0E4", end_idx);
  EXPECT_FLOAT_EQ(result, 30000.0f);
}

TEST(CoverageTextParser, ParseFloatLeadingSpaces)
{
  size_t end_idx = 0;
  float result = VW::details::parse_float("   42", end_idx);
  EXPECT_FLOAT_EQ(result, 42.0f);
}

TEST(CoverageTextParser, ParseFloatZero)
{
  size_t end_idx = 0;
  float result = VW::details::parse_float("0", end_idx);
  EXPECT_FLOAT_EQ(result, 0.0f);
}

TEST(CoverageTextParser, ParseFloatZeroDecimal)
{
  size_t end_idx = 0;
  float result = VW::details::parse_float("0.0", end_idx);
  EXPECT_FLOAT_EQ(result, 0.0f);
}

TEST(CoverageTextParser, ParseFloatNullInput)
{
  size_t end_idx = 0;
  float result = VW::details::parse_float(nullptr, end_idx);
  EXPECT_FLOAT_EQ(result, 0.0f);
  EXPECT_EQ(end_idx, 0u);
}

TEST(CoverageTextParser, ParseFloatEmptyString)
{
  size_t end_idx = 0;
  float result = VW::details::parse_float("", end_idx);
  EXPECT_FLOAT_EQ(result, 0.0f);
  EXPECT_EQ(end_idx, 0u);
}

TEST(CoverageTextParser, ParseFloatWithEndLine)
{
  const char* str = "3.14 next";
  const char* end = str + 4;  // end after "3.14"
  size_t end_idx = 0;
  float result = VW::details::parse_float(str, end_idx, end);
  EXPECT_NEAR(result, 3.14f, 0.001f);
}

TEST(CoverageTextParser, ParseFloatInfFallback)
{
  size_t end_idx = 0;
  // "inf" is not parseable by our fast parser, falls back to strtof
  float result = VW::details::parse_float("inf", end_idx);
  EXPECT_TRUE(std::isinf(result));
}

TEST(CoverageTextParser, ParseFloatNanFallback)
{
  size_t end_idx = 0;
  // "nan" is not parseable by our fast parser, falls back to strtof
  float result = VW::details::parse_float("nan", end_idx);
  EXPECT_TRUE(std::isnan(result));
}

TEST(CoverageTextParser, ParseFloatLongDecimal)
{
  size_t end_idx = 0;
  // More than 35 decimal digits - exercises truncation path in parse_float
  float result = VW::details::parse_float("1.123456789012345678901234567890123456789", end_idx);
  EXPECT_NEAR(result, 1.1234567f, 0.001f);
}

TEST(CoverageTextParser, EscapedTokenizeBasic)
{
  auto result = VW::details::escaped_tokenize(',', "a,b,c");
  EXPECT_EQ(result.size(), 3u);
  EXPECT_EQ(result[0], "a");
  EXPECT_EQ(result[1], "b");
  EXPECT_EQ(result[2], "c");
}

TEST(CoverageTextParser, EscapedTokenizeWithEscape)
{
  auto result = VW::details::escaped_tokenize(',', "a\\,b,c");
  EXPECT_EQ(result.size(), 2u);
  EXPECT_EQ(result[0], "a,b");
  EXPECT_EQ(result[1], "c");
}

TEST(CoverageTextParser, EscapedTokenizeAllowEmpty)
{
  auto result = VW::details::escaped_tokenize(',', "a,,c", true);
  EXPECT_EQ(result.size(), 3u);
  EXPECT_EQ(result[0], "a");
  EXPECT_EQ(result[1], "");
  EXPECT_EQ(result[2], "c");
}

TEST(CoverageTextParser, EscapedTokenizeTrailingDelim)
{
  // Trailing delimiter after non-empty token does not produce an extra empty token
  // because last_space is only set when delimiter is at position 0 of remaining string
  auto result = VW::details::escaped_tokenize(',', "a,b,", true);
  EXPECT_EQ(result.size(), 2u);
  EXPECT_EQ(result[0], "a");
  EXPECT_EQ(result[1], "b");
}

TEST(CoverageTextParser, EscapedTokenizeConsecutiveDelims)
{
  // Consecutive delimiters produce empty tokens when allow_empty is true
  auto result = VW::details::escaped_tokenize(',', ",a,,b,", true);
  EXPECT_GE(result.size(), 3u);
}

TEST(CoverageTextParser, EscapedTokenizeEscapeAtEnd)
{
  auto result = VW::details::escaped_tokenize(',', "a,b\\");
  EXPECT_EQ(result.size(), 2u);
  EXPECT_EQ(result[0], "a");
  EXPECT_EQ(result[1], "b");
}

TEST(CoverageTextParser, SplitCommandLineBasic)
{
  auto result = VW::split_command_line(std::string("--quiet --no_stdin -b 18"));
  EXPECT_EQ(result.size(), 4u);
  EXPECT_EQ(result[0], "--quiet");
  EXPECT_EQ(result[1], "--no_stdin");
  EXPECT_EQ(result[2], "-b");
  EXPECT_EQ(result[3], "18");
}

TEST(CoverageTextParser, SplitCommandLineQuoted)
{
  auto result = VW::split_command_line(std::string("--data \"my file.txt\" --quiet"));
  EXPECT_EQ(result.size(), 3u);
  EXPECT_EQ(result[0], "--data");
  EXPECT_EQ(result[1], "my file.txt");
  EXPECT_EQ(result[2], "--quiet");
}

TEST(CoverageTextParser, SplitCommandLineSingleQuoted)
{
  auto result = VW::split_command_line(std::string("--data 'my file.txt' --quiet"));
  EXPECT_EQ(result.size(), 3u);
  EXPECT_EQ(result[0], "--data");
  EXPECT_EQ(result[1], "my file.txt");
  EXPECT_EQ(result[2], "--quiet");
}

TEST(CoverageTextParser, SplitCommandLineEscapedSpace)
{
  auto result = VW::split_command_line(std::string("--data my\\ file.txt --quiet"));
  EXPECT_EQ(result.size(), 3u);
  EXPECT_EQ(result[0], "--data");
  EXPECT_EQ(result[1], "my file.txt");
  EXPECT_EQ(result[2], "--quiet");
}

TEST(CoverageTextParser, SplitCommandLineEscapedTab)
{
  auto result = VW::split_command_line(std::string("hello\\tworld"));
  EXPECT_EQ(result.size(), 1u);
  EXPECT_EQ(result[0], "hello\tworld");
}

TEST(CoverageTextParser, SplitCommandLineEscapedNewline)
{
  auto result = VW::split_command_line(std::string("hello\\nworld"));
  EXPECT_EQ(result.size(), 1u);
  EXPECT_EQ(result[0], "hello\nworld");
}

TEST(CoverageTextParser, SplitCommandLineEmpty)
{
  auto result = VW::split_command_line(std::string(""));
  EXPECT_EQ(result.size(), 0u);
}

TEST(CoverageTextParser, SplitCommandLineNestedQuotes)
{
  auto result = VW::split_command_line(std::string("--data \"it's a file\" --quiet"));
  EXPECT_EQ(result.size(), 3u);
  EXPECT_EQ(result[0], "--data");
  EXPECT_EQ(result[1], "it's a file");
  EXPECT_EQ(result[2], "--quiet");
}

TEST(CoverageTextParser, SplitByLimitBasic)
{
  VW::string_view s("abcdefghij");
  auto result = VW::split_by_limit(s, 3);
  EXPECT_EQ(result.size(), 4u);  // "abc", "def", "ghi", "j"
  EXPECT_EQ(result[0], "abc");
  EXPECT_EQ(result[1], "def");
  EXPECT_EQ(result[2], "ghi");
  EXPECT_EQ(result[3], "j");
}

TEST(CoverageTextParser, SplitByLimitExact)
{
  VW::string_view s("abcdef");
  auto result = VW::split_by_limit(s, 3);
  EXPECT_EQ(result.size(), 2u);
  EXPECT_EQ(result[0], "abc");
  EXPECT_EQ(result[1], "def");
}

TEST(CoverageTextParser, SplitByLimitLargerThanString)
{
  VW::string_view s("abc");
  auto result = VW::split_by_limit(s, 100);
  EXPECT_EQ(result.size(), 1u);
  EXPECT_EQ(result[0], "abc");
}

TEST(CoverageTextParser, SplitByLimitSizeOne)
{
  VW::string_view s("abc");
  auto result = VW::split_by_limit(s, 1);
  EXPECT_EQ(result.size(), 3u);
  EXPECT_EQ(result[0], "a");
  EXPECT_EQ(result[1], "b");
  EXPECT_EQ(result[2], "c");
}

// ============================================================
// 5. More edge cases combining parsing and learning
// ============================================================

TEST(CoverageTextParser, TextParserExponentOnlyFeature)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin"));
  auto* ex = VW::read_example(*vw, "| f1:1E0 f2:0E0");
  // f2:0E0 = 0, should be excluded
  EXPECT_EQ(ex->feature_space[' '].size(), 1);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, TextParserFeatureValueOne)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin"));
  auto* ex = VW::read_example(*vw, "| f1:1.0 f2:1 f3");
  // All three features should have value 1.0
  EXPECT_EQ(ex->feature_space[' '].size(), 3);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, AuditWithQuadratic)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "-a", "-q", "ab"));
  auto* ex = VW::read_example(*vw, "1 |a f1:1.0 |b f2:2.0");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, AuditWithCubic)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "-a", "--cubic", "abc"));
  auto* ex = VW::read_example(*vw, "1 |a f1:1.0 |b f2:2.0 |c f3:3.0");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, AuditWithInteractions)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "-a", "--interactions", "abc"));
  auto* ex = VW::read_example(*vw, "1 |a f1:1.0 |b f2:2.0 |c f3:3.0");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, IgnoreAndInteractions)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "--ignore", "c", "-q", "ab"));
  auto* ex = VW::read_example(*vw, "1 |a f1:1 |b f2:2 |c f3:3");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, KeepAndInteractions)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "--keep", "a", "--keep", "b", "-q", "ab"));
  auto* ex = VW::read_example(*vw, "1 |a f1:1 |b f2:2 |c f3:3");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, PassLengthOption)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "--initial_pass_length", "10"));
  for (int i = 0; i < 5; i++)
  {
    auto* ex = VW::read_example(*vw, "1 | a b c");
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

TEST(CoverageTextParser, ExamplesOption)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "--examples", "10"));
  for (int i = 0; i < 5; i++)
  {
    auto* ex = VW::read_example(*vw, "1 | a b c");
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

TEST(CoverageTextParser, LearningRateAndPowerT)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "-l", "0.01", "--power_t", "0.5"));
  for (int i = 0; i < 10; i++)
  {
    auto* ex = VW::read_example(*vw, "1 | a b c");
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

TEST(CoverageTextParser, DecayLearningRate)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "--decay_learning_rate", "0.9"));
  for (int i = 0; i < 10; i++)
  {
    auto* ex = VW::read_example(*vw, "1 | a b c");
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

TEST(CoverageTextParser, AdaptiveAndInvariant)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "--adaptive", "--invariant"));
  for (int i = 0; i < 10; i++)
  {
    auto* ex = VW::read_example(*vw, "1 | a b c");
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

TEST(CoverageTextParser, NormalizedUpdates)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "--normalized"));
  for (int i = 0; i < 10; i++)
  {
    auto* ex = VW::read_example(*vw, "1 | a b c");
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

TEST(CoverageTextParser, TextParserCarriageReturn)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin"));
  auto* ex = VW::read_example(*vw, "1 | a b c\r");
  // Carriage return should be handled
  EXPECT_FALSE(ex->is_newline);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, TextParserMixedNamespacesAndFeatures)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin"));
  auto* ex = VW::read_example(*vw, "1 |a f1:1 f2:2 |b f3:0.5 |c |d f4:10");
  // c namespace has no features, d has one feature
  EXPECT_EQ(ex->feature_space['a'].size(), 2);
  EXPECT_EQ(ex->feature_space['b'].size(), 1);
  EXPECT_EQ(ex->feature_space['d'].size(), 1);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, TextParserIntegerLabel)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin"));
  auto* ex = VW::read_example(*vw, "42 | a b c");
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, TextParserZeroLabel)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin"));
  auto* ex = VW::read_example(*vw, "0 | a b c");
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, TextParserFloatLabel)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin"));
  auto* ex = VW::read_example(*vw, "0.123456 | a b c");
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, TextParserMultipleNamespacesLearnPredict)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "-q", "ab"));
  for (int i = 0; i < 20; i++)
  {
    auto* ex = VW::read_example(*vw,
        std::to_string(i % 2 == 0 ? 1.0f : -1.0f) + " |a f1:" + std::to_string(i * 0.1) +
            " |b f2:" + std::to_string(i * 0.2));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
  auto* ex = VW::read_example(*vw, "|a f1:0.5 |b f2:1.0");
  vw->predict(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, SpellingFeaturesPatterns)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "--spelling", "_"));
  // Test different character classes: digits, lowercase, uppercase, dots, special
  auto* ex = VW::read_example(*vw, "1 | ABC123 abc.def a@b");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, AffixLongWord)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "--affix", "+7"));
  auto* ex = VW::read_example(*vw, "1 | ab");
  // Word shorter than affix length, should take the whole word
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageTextParser, AffixSuffixLongWord)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--no_stdin", "--affix", "-7"));
  auto* ex = VW::read_example(*vw, "1 | ab");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}
