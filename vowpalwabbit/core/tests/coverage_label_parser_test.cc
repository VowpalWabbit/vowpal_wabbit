// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

// Batch 12: Targeted tests for label parsing coverage gaps.

#include "vw/core/cost_sensitive.h"

#include "vw/common/string_view.h"
#include "vw/common/text_utils.h"
#include "vw/core/cb.h"
#include "vw/core/cb_with_observations_label.h"
#include "vw/core/example.h"
#include "vw/core/io_buf.h"
#include "vw/core/memory.h"
#include "vw/core/model_utils.h"
#include "vw/core/multiclass.h"
#include "vw/core/multilabel.h"
#include "vw/core/named_labels.h"
#include "vw/core/no_label.h"
#include "vw/core/parse_primitives.h"
#include "vw/core/parser.h"
#include "vw/core/simple_label.h"
#include "vw/core/simple_label_parser.h"
#include "vw/core/slates_label.h"
#include "vw/core/vw.h"
#include "vw/io/io_adapter.h"
#include "vw/io/logger.h"
#include "vw/test_common/test_common.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cfloat>
#include <climits>
#include <cmath>
#include <limits>
#include <memory>
#include <vector>

// ==================== Helper functions ====================
namespace
{
void parse_cs_label_b12(VW::label_parser& lp, VW::string_view label, VW::polylabel& l)
{
  std::vector<VW::string_view> words;
  VW::tokenize(' ', label, words);
  lp.default_label(l);
  VW::label_parser_reuse_mem mem;
  VW::reduction_features red_features;
  auto null_logger = VW::io::create_null_logger();
  lp.parse_label(l, red_features, mem, nullptr, words, null_logger);
}

void parse_cs_label_with_ldict_b12(
    VW::label_parser& lp, VW::string_view label, VW::polylabel& l, const VW::named_labels* ldict)
{
  std::vector<VW::string_view> words;
  VW::tokenize(' ', label, words);
  lp.default_label(l);
  VW::label_parser_reuse_mem mem;
  VW::reduction_features red_features;
  auto null_logger = VW::io::create_null_logger();
  lp.parse_label(l, red_features, mem, ldict, words, null_logger);
}

void parse_cb_label_b12(VW::label_parser& lp, VW::string_view label, VW::polylabel& l)
{
  std::vector<VW::string_view> words;
  VW::tokenize(' ', label, words);
  lp.default_label(l);
  VW::reduction_features red_fts;
  VW::label_parser_reuse_mem mem;
  auto null_logger = VW::io::create_null_logger();
  lp.parse_label(l, red_fts, mem, nullptr, words, null_logger);
}

void parse_multiclass_label_b12(VW::label_parser& lp, VW::string_view label, VW::polylabel& l)
{
  std::vector<VW::string_view> words;
  VW::tokenize(' ', label, words);
  lp.default_label(l);
  VW::reduction_features red_fts;
  VW::label_parser_reuse_mem mem;
  auto null_logger = VW::io::create_null_logger();
  lp.parse_label(l, red_fts, mem, nullptr, words, null_logger);
}

void parse_multiclass_label_with_ldict_b12(
    VW::label_parser& lp, VW::string_view label, VW::polylabel& l, const VW::named_labels* ldict)
{
  std::vector<VW::string_view> words;
  VW::tokenize(' ', label, words);
  lp.default_label(l);
  VW::reduction_features red_fts;
  VW::label_parser_reuse_mem mem;
  auto null_logger = VW::io::create_null_logger();
  lp.parse_label(l, red_fts, mem, ldict, words, null_logger);
}

void parse_simple_label_b12(VW::label_parser& lp, VW::string_view label, VW::polylabel& l,
    VW::reduction_features& red_features)
{
  std::vector<VW::string_view> words;
  VW::tokenize(' ', label, words);
  lp.default_label(l);
  VW::label_parser_reuse_mem mem;
  auto null_logger = VW::io::create_null_logger();
  lp.parse_label(l, red_features, mem, nullptr, words, null_logger);
}

void parse_multilabel_b12(VW::label_parser& lp, VW::string_view label, VW::polylabel& l)
{
  std::vector<VW::string_view> words;
  VW::tokenize(' ', label, words);
  lp.default_label(l);
  VW::reduction_features red_fts;
  VW::label_parser_reuse_mem mem;
  auto null_logger = VW::io::create_null_logger();
  lp.parse_label(l, red_fts, mem, nullptr, words, null_logger);
}
}  // namespace

// ==================== 1. Cost-Sensitive Labels ====================

TEST(CoverageLabelParser, CSParseBasicThreeClasses)
{
  auto lp = VW::cs_label_parser_global;
  VW::polylabel label;
  parse_cs_label_b12(lp, "1:1.0 2:0.5 3:2.0", label);

  EXPECT_EQ(label.cs.costs.size(), 3);
  EXPECT_FLOAT_EQ(label.cs.costs[0].x, 1.0f);
  EXPECT_FLOAT_EQ(label.cs.costs[1].x, 0.5f);
  EXPECT_FLOAT_EQ(label.cs.costs[2].x, 2.0f);
}

TEST(CoverageLabelParser, CSParseSingleClass)
{
  auto lp = VW::cs_label_parser_global;
  VW::polylabel label;
  parse_cs_label_b12(lp, "1:0.5", label);

  EXPECT_EQ(label.cs.costs.size(), 1);
  EXPECT_FLOAT_EQ(label.cs.costs[0].x, 0.5f);
}

TEST(CoverageLabelParser, CSParseManyClasses)
{
  auto lp = VW::cs_label_parser_global;
  VW::polylabel label;
  parse_cs_label_b12(lp, "1:0.1 2:0.2 3:0.3 4:0.4 5:0.5 6:0.6 7:0.7 8:0.8 9:0.9 10:1.0", label);

  EXPECT_EQ(label.cs.costs.size(), 10);
  for (size_t i = 0; i < 10; i++) { EXPECT_FLOAT_EQ(label.cs.costs[i].x, 0.1f * (i + 1)); }
}

TEST(CoverageLabelParser, CSParseEmptyLabel)
{
  auto lp = VW::cs_label_parser_global;
  VW::polylabel label;
  parse_cs_label_b12(lp, "", label);

  EXPECT_EQ(label.cs.costs.size(), 0);
  EXPECT_TRUE(label.cs.is_test_label());
}

TEST(CoverageLabelParser, CSParseZeroCost)
{
  auto lp = VW::cs_label_parser_global;
  VW::polylabel label;
  parse_cs_label_b12(lp, "1:0.0 2:0.0", label);

  EXPECT_EQ(label.cs.costs.size(), 2);
  EXPECT_FLOAT_EQ(label.cs.costs[0].x, 0.0f);
  EXPECT_FLOAT_EQ(label.cs.costs[1].x, 0.0f);
  EXPECT_FALSE(label.cs.is_test_label());
}

TEST(CoverageLabelParser, CSParseNegativeCost)
{
  auto lp = VW::cs_label_parser_global;
  VW::polylabel label;
  parse_cs_label_b12(lp, "1:-5.0 2:-0.5", label);

  EXPECT_EQ(label.cs.costs.size(), 2);
  EXPECT_FLOAT_EQ(label.cs.costs[0].x, -5.0f);
  EXPECT_FLOAT_EQ(label.cs.costs[1].x, -0.5f);
  EXPECT_FALSE(label.cs.is_test_label());
}

TEST(CoverageLabelParser, CSParseLargeCost)
{
  auto lp = VW::cs_label_parser_global;
  VW::polylabel label;
  parse_cs_label_b12(lp, "1:1000000.0", label);

  EXPECT_EQ(label.cs.costs.size(), 1);
  EXPECT_FLOAT_EQ(label.cs.costs[0].x, 1000000.0f);
}

TEST(CoverageLabelParser, CSParseTestLabelNoValue)
{
  auto lp = VW::cs_label_parser_global;
  VW::polylabel label;
  // class index without cost => FLT_MAX => test label
  parse_cs_label_b12(lp, "1", label);

  EXPECT_EQ(label.cs.costs.size(), 1);
  EXPECT_FLOAT_EQ(label.cs.costs[0].x, FLT_MAX);
  EXPECT_TRUE(label.cs.is_test_label());
}

TEST(CoverageLabelParser, CSParseMultipleTestLabels)
{
  auto lp = VW::cs_label_parser_global;
  VW::polylabel label;
  parse_cs_label_b12(lp, "1 2 3", label);

  EXPECT_EQ(label.cs.costs.size(), 3);
  for (size_t i = 0; i < 3; i++) { EXPECT_FLOAT_EQ(label.cs.costs[i].x, FLT_MAX); }
  EXPECT_TRUE(label.cs.is_test_label());
}

TEST(CoverageLabelParser, CSParseSharedKeyword)
{
  auto lp = VW::cs_label_parser_global;
  VW::polylabel label;
  parse_cs_label_b12(lp, "shared", label);

  EXPECT_EQ(label.cs.costs.size(), 1);
  EXPECT_FLOAT_EQ(label.cs.costs[0].x, -FLT_MAX);
  EXPECT_EQ(label.cs.costs[0].class_index, 0u);
}

TEST(CoverageLabelParser, CSParseLabelKeywordWithCost)
{
  auto lp = VW::cs_label_parser_global;
  VW::polylabel label;
  parse_cs_label_b12(lp, "label:0.75", label);

  EXPECT_EQ(label.cs.costs.size(), 1);
  EXPECT_FLOAT_EQ(label.cs.costs[0].x, 0.75f);
  EXPECT_EQ(label.cs.costs[0].class_index, 0u);
}

TEST(CoverageLabelParser, CSParseLabelKeywordWithoutCost)
{
  // label without cost should trigger error path (size != 2) and produce empty costs
  auto lp = VW::cs_label_parser_global;
  VW::polylabel label;
  parse_cs_label_b12(lp, "label", label);

  // "label" alone: tokens will be ["label"], size==1, not ==2, so error path
  // No cost pushed, empty
  EXPECT_EQ(label.cs.costs.size(), 0);
}

TEST(CoverageLabelParser, CSParseSharedWithCostErrorPath)
{
  // "shared:1.0" should produce error path (shared with cost), empty costs
  auto lp = VW::cs_label_parser_global;
  VW::polylabel label;
  parse_cs_label_b12(lp, "shared:1.0", label);

  // shared with a cost: tokens=["shared","1.0"], size!=1 => error path, no cost pushed
  EXPECT_EQ(label.cs.costs.size(), 0);
}

TEST(CoverageLabelParser, CSParseWithNamedLabelsBasic)
{
  auto lp = VW::cs_label_parser_global;
  VW::polylabel label;
  VW::named_labels ldict("cat,dog,bird");

  parse_cs_label_with_ldict_b12(lp, "cat:0.5 dog:1.0 bird:0.25", label, &ldict);

  EXPECT_EQ(label.cs.costs.size(), 3);
  EXPECT_FLOAT_EQ(label.cs.costs[0].x, 0.5f);
  EXPECT_FLOAT_EQ(label.cs.costs[1].x, 1.0f);
  EXPECT_FLOAT_EQ(label.cs.costs[2].x, 0.25f);
}

TEST(CoverageLabelParser, CSParseTripleStarSharedWithLdict)
{
  auto lp = VW::cs_label_parser_global;
  VW::polylabel label;
  VW::named_labels ldict("cat,dog,bird");

  parse_cs_label_with_ldict_b12(lp, "***shared***", label, &ldict);

  EXPECT_EQ(label.cs.costs.size(), 1);
  EXPECT_FLOAT_EQ(label.cs.costs[0].x, -FLT_MAX);
}

TEST(CoverageLabelParser, CSParseTripleStarLabelWithLdict)
{
  auto lp = VW::cs_label_parser_global;
  VW::polylabel label;
  VW::named_labels ldict("cat,dog,bird");

  parse_cs_label_with_ldict_b12(lp, "***label***:0.5", label, &ldict);

  EXPECT_EQ(label.cs.costs.size(), 1);
  EXPECT_FLOAT_EQ(label.cs.costs[0].x, 0.5f);
  EXPECT_EQ(label.cs.costs[0].class_index, 0u);
}

TEST(CoverageLabelParser, CSPlainSharedNotRecognizedWithLdict)
{
  // With ldict, plain "shared" is NOT a keyword, it's treated as a class name
  auto lp = VW::cs_label_parser_global;
  VW::polylabel label;
  VW::named_labels ldict("cat,dog,shared");

  parse_cs_label_with_ldict_b12(lp, "shared", label, &ldict);

  // "shared" is treated as a regular label name, not as the shared keyword.
  // As a single token without a cost, it gets FLT_MAX (test label).
  EXPECT_EQ(label.cs.costs.size(), 1);
  EXPECT_FLOAT_EQ(label.cs.costs[0].x, FLT_MAX);
}

TEST(CoverageLabelParser, CSIsTestLabelEmpty)
{
  VW::cs_label label;
  label.costs.clear();
  EXPECT_TRUE(label.is_test_label());
}

TEST(CoverageLabelParser, CSIsTestLabelAllFLTMax)
{
  VW::cs_label label;
  label.costs.push_back({FLT_MAX, 1, 0.f, 0.f});
  label.costs.push_back({FLT_MAX, 2, 0.f, 0.f});
  label.costs.push_back({FLT_MAX, 3, 0.f, 0.f});
  EXPECT_TRUE(label.is_test_label());
}

TEST(CoverageLabelParser, CSIsTestLabelWithRealCosts)
{
  VW::cs_label label;
  label.costs.push_back({0.5f, 1, 0.f, 0.f});
  EXPECT_FALSE(label.is_test_label());
}

TEST(CoverageLabelParser, CSIsTestLabelMixed)
{
  VW::cs_label label;
  label.costs.push_back({FLT_MAX, 1, 0.f, 0.f});
  label.costs.push_back({0.5f, 2, 0.f, 0.f});
  EXPECT_FALSE(label.is_test_label());
}

TEST(CoverageLabelParser, CSResetToDefault)
{
  VW::cs_label label;
  label.costs.push_back({1.0f, 1, 0.f, 0.f});
  label.costs.push_back({2.0f, 2, 0.f, 0.f});
  EXPECT_EQ(label.costs.size(), 2);
  label.reset_to_default();
  EXPECT_EQ(label.costs.size(), 0);
}

TEST(CoverageLabelParser, CSWeightAlwaysOne)
{
  auto lp = VW::cs_label_parser_global;
  VW::polylabel label;
  VW::reduction_features red_features;
  parse_cs_label_b12(lp, "1:0.5 2:1.0", label);
  EXPECT_FLOAT_EQ(lp.get_weight(label, red_features), 1.0f);
}

TEST(CoverageLabelParser, CSCacheRoundTripMultipleClasses)
{
  auto lp = VW::cs_label_parser_global;
  VW::polylabel original;
  parse_cs_label_b12(lp, "1:0.5 2:1.0 3:0.25", original);

  VW::io_buf write_buf;
  auto backing = std::make_shared<std::vector<char>>();
  write_buf.add_file(VW::io::create_vector_writer(backing));
  VW::reduction_features red_features;
  lp.cache_label(original, red_features, write_buf, "test", false);
  write_buf.flush();

  VW::io_buf read_buf;
  read_buf.add_file(VW::io::create_buffer_view(backing->data(), backing->size()));
  VW::polylabel restored;
  lp.default_label(restored);
  lp.read_cached_label(restored, red_features, read_buf);

  EXPECT_EQ(restored.cs.costs.size(), original.cs.costs.size());
  for (size_t i = 0; i < original.cs.costs.size(); i++)
  {
    EXPECT_FLOAT_EQ(restored.cs.costs[i].x, original.cs.costs[i].x);
    EXPECT_EQ(restored.cs.costs[i].class_index, original.cs.costs[i].class_index);
    EXPECT_FLOAT_EQ(restored.cs.costs[i].partial_prediction, original.cs.costs[i].partial_prediction);
    EXPECT_FLOAT_EQ(restored.cs.costs[i].wap_value, original.cs.costs[i].wap_value);
  }
}

TEST(CoverageLabelParser, CSCacheRoundTripEmptyLabel)
{
  auto lp = VW::cs_label_parser_global;
  VW::polylabel original;
  parse_cs_label_b12(lp, "", original);

  VW::io_buf write_buf;
  auto backing = std::make_shared<std::vector<char>>();
  write_buf.add_file(VW::io::create_vector_writer(backing));
  VW::reduction_features red_features;
  lp.cache_label(original, red_features, write_buf, "test", false);
  write_buf.flush();

  VW::io_buf read_buf;
  read_buf.add_file(VW::io::create_buffer_view(backing->data(), backing->size()));
  VW::polylabel restored;
  lp.default_label(restored);
  lp.read_cached_label(restored, red_features, read_buf);

  EXPECT_EQ(restored.cs.costs.size(), 0);
}

TEST(CoverageLabelParser, CSCacheRoundTripSharedLabel)
{
  auto lp = VW::cs_label_parser_global;
  VW::polylabel original;
  parse_cs_label_b12(lp, "shared", original);

  VW::io_buf write_buf;
  auto backing = std::make_shared<std::vector<char>>();
  write_buf.add_file(VW::io::create_vector_writer(backing));
  VW::reduction_features red_features;
  lp.cache_label(original, red_features, write_buf, "test", false);
  write_buf.flush();

  VW::io_buf read_buf;
  read_buf.add_file(VW::io::create_buffer_view(backing->data(), backing->size()));
  VW::polylabel restored;
  lp.default_label(restored);
  lp.read_cached_label(restored, red_features, read_buf);

  EXPECT_EQ(restored.cs.costs.size(), 1);
  EXPECT_FLOAT_EQ(restored.cs.costs[0].x, -FLT_MAX);
  EXPECT_EQ(restored.cs.costs[0].class_index, 0u);
}

TEST(CoverageLabelParser, CSLabelTypeIsCS)
{
  auto lp = VW::cs_label_parser_global;
  EXPECT_EQ(lp.label_type, VW::label_type_t::CS);
}

TEST(CoverageLabelParser, CSIsExampleHeaderShared)
{
  VW::example ex;
  ex.l.cs.costs.clear();
  VW::cs_class header{-FLT_MAX, 0, 0.f, 0.f};
  ex.l.cs.costs.push_back(header);
  EXPECT_TRUE(VW::is_cs_example_header(ex));
}

TEST(CoverageLabelParser, CSIsExampleHeaderNonZeroIndex)
{
  VW::example ex;
  ex.l.cs.costs.clear();
  VW::cs_class header{-FLT_MAX, 1, 0.f, 0.f};
  ex.l.cs.costs.push_back(header);
  EXPECT_FALSE(VW::is_cs_example_header(ex));
}

TEST(CoverageLabelParser, CSIsExampleHeaderWrongCost)
{
  VW::example ex;
  ex.l.cs.costs.clear();
  VW::cs_class header{0.5f, 0, 0.f, 0.f};
  ex.l.cs.costs.push_back(header);
  EXPECT_FALSE(VW::is_cs_example_header(ex));
}

TEST(CoverageLabelParser, CSIsExampleHeaderMultipleCosts)
{
  VW::example ex;
  ex.l.cs.costs.clear();
  ex.l.cs.costs.push_back({-FLT_MAX, 0, 0.f, 0.f});
  ex.l.cs.costs.push_back({1.0f, 1, 0.f, 0.f});
  EXPECT_FALSE(VW::is_cs_example_header(ex));
}

TEST(CoverageLabelParser, CSIsExampleHeaderEmpty)
{
  VW::example ex;
  ex.l.cs.costs.clear();
  EXPECT_FALSE(VW::is_cs_example_header(ex));
}

TEST(CoverageLabelParser, CSClassEquality)
{
  VW::cs_class a{1.0f, 5, 0.f, 0.f};
  VW::cs_class b{2.0f, 5, 0.1f, 0.2f};
  VW::cs_class c{1.0f, 3, 0.f, 0.f};

  EXPECT_TRUE(a == b);   // equality based on class_index
  EXPECT_FALSE(a == c);
}

TEST(CoverageLabelParser, CSIntegrationWithCSOAA)
{
  auto vw = VW::initialize(vwtest::make_args("--csoaa", "3", "--quiet"));

  auto* ex = VW::read_example(*vw, "1:0.5 2:1.0 3:0.25 | a b c");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);

  auto* ex_pred = VW::read_example(*vw, "| a b c");
  vw->predict(*ex_pred);
  EXPECT_GE(ex_pred->pred.multiclass, 1u);
  EXPECT_LE(ex_pred->pred.multiclass, 3u);
  VW::finish_example(*vw, *ex_pred);
}

TEST(CoverageLabelParser, CSModelSerializationCSClass)
{
  VW::cs_class original{1.5f, 3, 0.25f, 0.75f};

  VW::io_buf write_buf;
  auto backing = std::make_shared<std::vector<char>>();
  write_buf.add_file(VW::io::create_vector_writer(backing));
  VW::model_utils::write_model_field(write_buf, original, "test", false);
  write_buf.flush();

  VW::io_buf read_buf;
  read_buf.add_file(VW::io::create_buffer_view(backing->data(), backing->size()));
  VW::cs_class restored;
  VW::model_utils::read_model_field(read_buf, restored);

  EXPECT_FLOAT_EQ(restored.x, original.x);
  EXPECT_EQ(restored.class_index, original.class_index);
  EXPECT_FLOAT_EQ(restored.partial_prediction, original.partial_prediction);
  EXPECT_FLOAT_EQ(restored.wap_value, original.wap_value);
}

TEST(CoverageLabelParser, CSDefaultLabelViaParser)
{
  auto lp = VW::cs_label_parser_global;
  VW::polylabel label;
  lp.default_label(label);
  EXPECT_EQ(label.cs.costs.size(), 0);
  EXPECT_TRUE(lp.test_label(label));
}

TEST(CoverageLabelParser, CSTestLabelViaParser)
{
  auto lp = VW::cs_label_parser_global;
  VW::polylabel label;
  parse_cs_label_b12(lp, "1:0.5", label);
  EXPECT_FALSE(lp.test_label(label));
}

// ==================== 2. Multiclass Labels ====================

TEST(CoverageLabelParser, MulticlassParseBasicLabel)
{
  auto lp = VW::multiclass_label_parser_global;
  VW::polylabel label;
  parse_multiclass_label_b12(lp, "3", label);
  EXPECT_EQ(label.multi.label, 3u);
  EXPECT_FLOAT_EQ(label.multi.weight, 1.0f);
}

TEST(CoverageLabelParser, MulticlassParseWithWeight)
{
  auto lp = VW::multiclass_label_parser_global;
  VW::polylabel label;
  parse_multiclass_label_b12(lp, "3 0.5", label);
  EXPECT_EQ(label.multi.label, 3u);
  EXPECT_FLOAT_EQ(label.multi.weight, 0.5f);
}

TEST(CoverageLabelParser, MulticlassParseEmptyLabel)
{
  auto lp = VW::multiclass_label_parser_global;
  VW::polylabel label;
  parse_multiclass_label_b12(lp, "", label);
  EXPECT_EQ(label.multi.label, std::numeric_limits<uint32_t>::max());
  EXPECT_FALSE(label.multi.is_labeled());
}

TEST(CoverageLabelParser, MulticlassLabelIsLabeled)
{
  VW::multiclass_label label(5, 1.0f);
  EXPECT_TRUE(label.is_labeled());
}

TEST(CoverageLabelParser, MulticlassLabelIsNotLabeled)
{
  VW::multiclass_label label;
  EXPECT_FALSE(label.is_labeled());
}

TEST(CoverageLabelParser, MulticlassTestLabel)
{
  VW::multiclass_label label;
  EXPECT_TRUE(VW::test_multiclass_label(label));
  label.label = 5;
  EXPECT_FALSE(VW::test_multiclass_label(label));
}

TEST(CoverageLabelParser, MulticlassParseInvalidComma)
{
  auto lp = VW::multiclass_label_parser_global;
  VW::polylabel label;
  EXPECT_THROW(parse_multiclass_label_b12(lp, "1,2,3", label), VW::vw_exception);
}

TEST(CoverageLabelParser, MulticlassParseInvalidTrailingChar)
{
  auto lp = VW::multiclass_label_parser_global;
  VW::polylabel label;
  EXPECT_THROW(parse_multiclass_label_b12(lp, "1a", label), VW::vw_exception);
}

TEST(CoverageLabelParser, MulticlassParseTooManyWords)
{
  auto lp = VW::multiclass_label_parser_global;
  VW::polylabel label;
  EXPECT_THROW(parse_multiclass_label_b12(lp, "1 2 3", label), VW::vw_exception);
}

TEST(CoverageLabelParser, MulticlassNamedLabels)
{
  auto lp = VW::multiclass_label_parser_global;
  VW::named_labels ldict("cat,dog,bird");

  VW::polylabel label;
  parse_multiclass_label_with_ldict_b12(lp, "dog", label, &ldict);
  EXPECT_EQ(label.multi.label, 2u);  // dog is second (1-indexed)
}

TEST(CoverageLabelParser, MulticlassNamedLabelsWithWeight)
{
  auto lp = VW::multiclass_label_parser_global;
  VW::named_labels ldict("cat,dog,bird");

  VW::polylabel label;
  parse_multiclass_label_with_ldict_b12(lp, "bird 3.0", label, &ldict);
  EXPECT_EQ(label.multi.label, 3u);
  EXPECT_FLOAT_EQ(label.multi.weight, 3.0f);
}

TEST(CoverageLabelParser, MulticlassWeightZero)
{
  auto lp = VW::multiclass_label_parser_global;
  VW::polylabel label;
  VW::reduction_features red_features;
  lp.default_label(label);
  label.multi.weight = 0.0f;
  EXPECT_FLOAT_EQ(lp.get_weight(label, red_features), 0.0f);
}

TEST(CoverageLabelParser, MulticlassWeightNegative)
{
  auto lp = VW::multiclass_label_parser_global;
  VW::polylabel label;
  VW::reduction_features red_features;
  lp.default_label(label);
  label.multi.weight = -1.0f;
  EXPECT_FLOAT_EQ(lp.get_weight(label, red_features), 0.0f);
}

TEST(CoverageLabelParser, MulticlassCacheRoundTrip)
{
  auto lp = VW::multiclass_label_parser_global;
  VW::polylabel original;
  parse_multiclass_label_b12(lp, "7 3.5", original);

  VW::io_buf write_buf;
  auto backing = std::make_shared<std::vector<char>>();
  write_buf.add_file(VW::io::create_vector_writer(backing));
  VW::reduction_features red_features;
  lp.cache_label(original, red_features, write_buf, "test", false);
  write_buf.flush();

  VW::io_buf read_buf;
  read_buf.add_file(VW::io::create_buffer_view(backing->data(), backing->size()));
  VW::polylabel restored;
  lp.default_label(restored);
  lp.read_cached_label(restored, red_features, read_buf);

  EXPECT_EQ(restored.multi.label, 7u);
  EXPECT_FLOAT_EQ(restored.multi.weight, 3.5f);
}

TEST(CoverageLabelParser, MulticlassDefaultLabel)
{
  auto lp = VW::multiclass_label_parser_global;
  VW::polylabel label;
  lp.default_label(label);
  EXPECT_EQ(label.multi.label, std::numeric_limits<uint32_t>::max());
  EXPECT_FLOAT_EQ(label.multi.weight, 1.0f);
}

TEST(CoverageLabelParser, MulticlassResetToDefault)
{
  VW::multiclass_label label(5, 2.0f);
  label.reset_to_default();
  EXPECT_EQ(label.label, std::numeric_limits<uint32_t>::max());
  EXPECT_FLOAT_EQ(label.weight, 1.0f);
}

TEST(CoverageLabelParser, MulticlassLabelTypeIsMulticlass)
{
  auto lp = VW::multiclass_label_parser_global;
  EXPECT_EQ(lp.label_type, VW::label_type_t::MULTICLASS);
}

TEST(CoverageLabelParser, MulticlassIntegrationOAA)
{
  auto vw = VW::initialize(vwtest::make_args("--oaa", "5", "--quiet"));
  auto* ex = VW::read_example(*vw, "3 | a b c");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);

  auto* ex2 = VW::read_example(*vw, "| a b c");
  vw->predict(*ex2);
  EXPECT_GE(ex2->pred.multiclass, 1u);
  EXPECT_LE(ex2->pred.multiclass, 5u);
  VW::finish_example(*vw, *ex2);
}

// ==================== 3. No-Label ====================

TEST(CoverageLabelParser, NoLabelParseEmpty)
{
  auto lp = VW::no_label_parser_global;
  VW::polylabel label;
  std::vector<VW::string_view> words;
  lp.default_label(label);
  VW::label_parser_reuse_mem mem;
  VW::reduction_features red_features;
  auto null_logger = VW::io::create_null_logger();
  lp.parse_label(label, red_features, mem, nullptr, words, null_logger);
  // Should not throw
}

TEST(CoverageLabelParser, NoLabelDefaultLabel)
{
  auto lp = VW::no_label_parser_global;
  VW::polylabel label;
  lp.default_label(label);
  // no_label's default is a no-op, just verify it does not throw
}

TEST(CoverageLabelParser, NoLabelTestLabelReturnsFalse)
{
  auto lp = VW::no_label_parser_global;
  VW::polylabel label;
  lp.default_label(label);
  EXPECT_FALSE(lp.test_label(label));
}

TEST(CoverageLabelParser, NoLabelGetWeightReturnsOne)
{
  auto lp = VW::no_label_parser_global;
  VW::polylabel label;
  VW::reduction_features red_features;
  lp.default_label(label);
  EXPECT_FLOAT_EQ(lp.get_weight(label, red_features), 1.0f);
}

TEST(CoverageLabelParser, NoLabelCacheLabelReturnsOne)
{
  auto lp = VW::no_label_parser_global;
  VW::polylabel label;
  VW::reduction_features red_features;
  lp.default_label(label);
  VW::io_buf cache;
  size_t result = lp.cache_label(label, red_features, cache, "", false);
  EXPECT_EQ(result, 1u);
}

TEST(CoverageLabelParser, NoLabelReadCachedLabelReturnsOne)
{
  auto lp = VW::no_label_parser_global;
  VW::polylabel label;
  VW::reduction_features red_features;
  VW::io_buf cache;
  size_t result = lp.read_cached_label(label, red_features, cache);
  EXPECT_EQ(result, 1u);
}

TEST(CoverageLabelParser, NoLabelLabelType)
{
  auto lp = VW::no_label_parser_global;
  EXPECT_EQ(lp.label_type, VW::label_type_t::NOLABEL);
}

TEST(CoverageLabelParser, NoLabelParseWithTooManyTokens)
{
  // Triggers the default case in parse_no_label (error path)
  auto lp = VW::no_label_parser_global;
  VW::polylabel label;
  std::vector<VW::string_view> words;
  words.push_back("unexpected");
  words.push_back("tokens");
  lp.default_label(label);
  VW::label_parser_reuse_mem mem;
  VW::reduction_features red_features;
  auto null_logger = VW::io::create_null_logger();
  // This triggers the error log path but does not throw
  lp.parse_label(label, red_features, mem, nullptr, words, null_logger);
}

TEST(CoverageLabelParser, NoLabelIntegration)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* ex = VW::read_example(*vw, "| a b c");
  vw->predict(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageLabelParser, NoLabelOutputAndAccount)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* ex = VW::read_example(*vw, "| a b c");
  vw->predict(*ex);
  VW::details::output_and_account_no_label_example(*vw, *ex);
  VW::finish_example(*vw, *ex);
}

// ==================== 4. Slates Labels ====================

TEST(CoverageLabelParser, SlatesParseSharedUnlabeled)
{
  VW::slates::label label;
  VW::slates::default_label(label);
  VW::label_parser_reuse_mem mem;
  std::vector<VW::string_view> words;
  VW::tokenize(' ', VW::string_view("slates shared"), words);
  auto null_logger = VW::io::create_null_logger();
  VW::slates::parse_label(label, mem, words, null_logger);

  EXPECT_EQ(label.type, VW::slates::example_type::SHARED);
  EXPECT_FLOAT_EQ(label.cost, 0.f);
  EXPECT_EQ(label.labeled, false);
}

TEST(CoverageLabelParser, SlatesParseSharedLabeled)
{
  VW::slates::label label;
  VW::slates::default_label(label);
  VW::label_parser_reuse_mem mem;
  std::vector<VW::string_view> words;
  VW::tokenize(' ', VW::string_view("slates shared 2.5"), words);
  auto null_logger = VW::io::create_null_logger();
  VW::slates::parse_label(label, mem, words, null_logger);

  EXPECT_EQ(label.type, VW::slates::example_type::SHARED);
  EXPECT_FLOAT_EQ(label.cost, 2.5f);
  EXPECT_EQ(label.labeled, true);
}

TEST(CoverageLabelParser, SlatesParseAction)
{
  VW::slates::label label;
  VW::slates::default_label(label);
  VW::label_parser_reuse_mem mem;
  std::vector<VW::string_view> words;
  VW::tokenize(' ', VW::string_view("slates action 3"), words);
  auto null_logger = VW::io::create_null_logger();
  VW::slates::parse_label(label, mem, words, null_logger);

  EXPECT_EQ(label.type, VW::slates::example_type::ACTION);
  EXPECT_EQ(label.slot_id, 3u);
}

TEST(CoverageLabelParser, SlatesParseSlotUnlabeled)
{
  VW::slates::label label;
  VW::slates::default_label(label);
  VW::label_parser_reuse_mem mem;
  std::vector<VW::string_view> words;
  VW::tokenize(' ', VW::string_view("slates slot"), words);
  auto null_logger = VW::io::create_null_logger();
  VW::slates::parse_label(label, mem, words, null_logger);

  EXPECT_EQ(label.type, VW::slates::example_type::SLOT);
  EXPECT_EQ(label.labeled, false);
}

TEST(CoverageLabelParser, SlatesParseSlotLabeled)
{
  VW::slates::label label;
  VW::slates::default_label(label);
  VW::label_parser_reuse_mem mem;
  std::vector<VW::string_view> words;
  VW::tokenize(' ', VW::string_view("slates slot 0:0.8,1:0.2"), words);
  auto null_logger = VW::io::create_null_logger();
  VW::slates::parse_label(label, mem, words, null_logger);

  EXPECT_EQ(label.type, VW::slates::example_type::SLOT);
  EXPECT_EQ(label.labeled, true);
  EXPECT_EQ(label.probabilities.size(), 2);
  EXPECT_EQ(label.probabilities[0].action, 0u);
  EXPECT_FLOAT_EQ(label.probabilities[0].score, 0.8f);
  EXPECT_EQ(label.probabilities[1].action, 1u);
  EXPECT_FLOAT_EQ(label.probabilities[1].score, 0.2f);
}

TEST(CoverageLabelParser, SlatesParseSlotSingleAction)
{
  VW::slates::label label;
  VW::slates::default_label(label);
  VW::label_parser_reuse_mem mem;
  std::vector<VW::string_view> words;
  VW::tokenize(' ', VW::string_view("slates slot 0:1.0"), words);
  auto null_logger = VW::io::create_null_logger();
  VW::slates::parse_label(label, mem, words, null_logger);

  EXPECT_EQ(label.type, VW::slates::example_type::SLOT);
  EXPECT_EQ(label.labeled, true);
  EXPECT_EQ(label.probabilities.size(), 1);
}

TEST(CoverageLabelParser, SlatesParseEmptyThrows)
{
  VW::slates::label label;
  VW::label_parser_reuse_mem mem;
  std::vector<VW::string_view> words;
  auto null_logger = VW::io::create_null_logger();
  EXPECT_THROW(VW::slates::parse_label(label, mem, words, null_logger), VW::vw_exception);
}

TEST(CoverageLabelParser, SlatesParseNoTypeThrows)
{
  VW::slates::label label;
  VW::label_parser_reuse_mem mem;
  std::vector<VW::string_view> words;
  VW::tokenize(' ', VW::string_view("slates"), words);
  auto null_logger = VW::io::create_null_logger();
  EXPECT_THROW(VW::slates::parse_label(label, mem, words, null_logger), VW::vw_exception);
}

TEST(CoverageLabelParser, SlatesParseInvalidTypeThrows)
{
  VW::slates::label label;
  VW::label_parser_reuse_mem mem;
  std::vector<VW::string_view> words;
  VW::tokenize(' ', VW::string_view("slates bogus"), words);
  auto null_logger = VW::io::create_null_logger();
  EXPECT_THROW(VW::slates::parse_label(label, mem, words, null_logger), VW::vw_exception);
}

TEST(CoverageLabelParser, SlatesParseActionMissingSlotIdThrows)
{
  VW::slates::label label;
  VW::label_parser_reuse_mem mem;
  std::vector<VW::string_view> words;
  VW::tokenize(' ', VW::string_view("slates action"), words);
  auto null_logger = VW::io::create_null_logger();
  EXPECT_THROW(VW::slates::parse_label(label, mem, words, null_logger), VW::vw_exception);
}

TEST(CoverageLabelParser, SlatesSharedTooManyArgsThrows)
{
  VW::slates::label label;
  VW::label_parser_reuse_mem mem;
  std::vector<VW::string_view> words;
  VW::tokenize(' ', VW::string_view("slates shared 1.0 extra"), words);
  auto null_logger = VW::io::create_null_logger();
  EXPECT_THROW(VW::slates::parse_label(label, mem, words, null_logger), VW::vw_exception);
}

TEST(CoverageLabelParser, SlatesSlotTooManyArgsThrows)
{
  VW::slates::label label;
  VW::label_parser_reuse_mem mem;
  std::vector<VW::string_view> words;
  VW::tokenize(' ', VW::string_view("slates slot 0:0.5 extra"), words);
  auto null_logger = VW::io::create_null_logger();
  EXPECT_THROW(VW::slates::parse_label(label, mem, words, null_logger), VW::vw_exception);
}

TEST(CoverageLabelParser, SlatesNotStartingWithSlatesThrows)
{
  VW::slates::label label;
  VW::label_parser_reuse_mem mem;
  std::vector<VW::string_view> words;
  VW::tokenize(' ', VW::string_view("shared"), words);
  auto null_logger = VW::io::create_null_logger();
  EXPECT_THROW(VW::slates::parse_label(label, mem, words, null_logger), VW::vw_exception);
}

TEST(CoverageLabelParser, SlatesDefaultLabel)
{
  VW::slates::label label;
  VW::slates::default_label(label);
  EXPECT_EQ(label.type, VW::slates::example_type::UNSET);
  EXPECT_FLOAT_EQ(label.weight, 1.0f);
  EXPECT_FALSE(label.labeled);
  EXPECT_FLOAT_EQ(label.cost, 0.0f);
  EXPECT_EQ(label.slot_id, 0u);
  EXPECT_EQ(label.probabilities.size(), 0u);
}

TEST(CoverageLabelParser, SlatesTestLabel)
{
  auto lp = VW::slates::slates_label_parser;
  VW::polylabel plabel;
  lp.default_label(plabel);
  EXPECT_TRUE(lp.test_label(plabel));
  plabel.slates.labeled = true;
  EXPECT_FALSE(lp.test_label(plabel));
}

TEST(CoverageLabelParser, SlatesCacheRoundTripSlot)
{
  auto lp = VW::slates::slates_label_parser;

  VW::polylabel original;
  lp.default_label(original);
  original.slates.type = VW::slates::example_type::SLOT;
  original.slates.labeled = true;
  original.slates.probabilities.push_back({0, 0.6f});
  original.slates.probabilities.push_back({1, 0.4f});

  VW::io_buf write_buf;
  auto backing = std::make_shared<std::vector<char>>();
  write_buf.add_file(VW::io::create_vector_writer(backing));
  VW::reduction_features red_features;
  lp.cache_label(original, red_features, write_buf, "test", false);
  write_buf.flush();

  VW::io_buf read_buf;
  read_buf.add_file(VW::io::create_buffer_view(backing->data(), backing->size()));
  VW::polylabel restored;
  lp.default_label(restored);
  lp.read_cached_label(restored, red_features, read_buf);

  EXPECT_EQ(restored.slates.type, VW::slates::example_type::SLOT);
  EXPECT_EQ(restored.slates.labeled, true);
  EXPECT_EQ(restored.slates.probabilities.size(), 2);
  EXPECT_FLOAT_EQ(restored.slates.probabilities[0].score, 0.6f);
  EXPECT_FLOAT_EQ(restored.slates.probabilities[1].score, 0.4f);
}

TEST(CoverageLabelParser, SlatesToStringTypes)
{
  EXPECT_EQ(VW::to_string(VW::slates::example_type::UNSET), "example_type::UNSET");
  EXPECT_EQ(VW::to_string(VW::slates::example_type::SHARED), "example_type::SHARED");
  EXPECT_EQ(VW::to_string(VW::slates::example_type::ACTION), "example_type::ACTION");
  EXPECT_EQ(VW::to_string(VW::slates::example_type::SLOT), "example_type::SLOT");
}

TEST(CoverageLabelParser, SlatesLabelTypeIsSlates)
{
  auto lp = VW::slates::slates_label_parser;
  EXPECT_EQ(lp.label_type, VW::label_type_t::SLATES);
}

// ==================== 5. CB Labels ====================

TEST(CoverageLabelParser, CBParseActionCostProb)
{
  auto lp = VW::cb_label_parser_global;
  VW::polylabel label;
  parse_cb_label_b12(lp, "1:0.5:0.3", label);

  EXPECT_EQ(label.cb.costs.size(), 1);
  EXPECT_FLOAT_EQ(label.cb.costs[0].cost, 0.5f);
  EXPECT_FLOAT_EQ(label.cb.costs[0].probability, 0.3f);
}

TEST(CoverageLabelParser, CBParseActionOnly)
{
  auto lp = VW::cb_label_parser_global;
  VW::polylabel label;
  parse_cb_label_b12(lp, "1", label);

  EXPECT_EQ(label.cb.costs.size(), 1);
  EXPECT_FLOAT_EQ(label.cb.costs[0].cost, FLT_MAX);
  EXPECT_FLOAT_EQ(label.cb.costs[0].probability, 0.0f);
}

TEST(CoverageLabelParser, CBParseActionCostOnly)
{
  auto lp = VW::cb_label_parser_global;
  VW::polylabel label;
  parse_cb_label_b12(lp, "1:0.5", label);

  EXPECT_EQ(label.cb.costs.size(), 1);
  EXPECT_FLOAT_EQ(label.cb.costs[0].cost, 0.5f);
  EXPECT_FLOAT_EQ(label.cb.costs[0].probability, 0.0f);
}

TEST(CoverageLabelParser, CBParseShared)
{
  auto lp = VW::cb_label_parser_global;
  VW::polylabel label;
  parse_cb_label_b12(lp, "shared", label);

  EXPECT_EQ(label.cb.costs.size(), 1);
  EXPECT_FLOAT_EQ(label.cb.costs[0].probability, -1.0f);
}

TEST(CoverageLabelParser, CBParseEmptyLabel)
{
  auto lp = VW::cb_label_parser_global;
  VW::polylabel label;
  parse_cb_label_b12(lp, "", label);

  EXPECT_TRUE(label.cb.costs.empty());
  EXPECT_TRUE(label.cb.is_test_label());
}

TEST(CoverageLabelParser, CBParseMultipleActions)
{
  auto lp = VW::cb_label_parser_global;
  VW::polylabel label;
  parse_cb_label_b12(lp, "1:0.5:0.25 2:0.3:0.75", label);

  EXPECT_EQ(label.cb.costs.size(), 2);
  EXPECT_FLOAT_EQ(label.cb.costs[0].cost, 0.5f);
  EXPECT_FLOAT_EQ(label.cb.costs[1].cost, 0.3f);
}

TEST(CoverageLabelParser, CBProbabilityClampedAboveOne)
{
  auto vw = VW::initialize(vwtest::make_args("--cb", "2", "--quiet"));
  auto* ex = VW::read_example(*vw, "1:0.5:1.5 | a");
  EXPECT_FLOAT_EQ(ex->l.cb.costs[0].probability, 1.0f);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageLabelParser, CBProbabilityClampedBelowZero)
{
  auto vw = VW::initialize(vwtest::make_args("--cb", "2", "--quiet"));
  auto* ex = VW::read_example(*vw, "1:0.5:-0.5 | a");
  EXPECT_FLOAT_EQ(ex->l.cb.costs[0].probability, 0.0f);
  VW::finish_example(*vw, *ex);
}

TEST(CoverageLabelParser, CBIsTestLabelEmpty)
{
  VW::cb_label label;
  EXPECT_TRUE(label.is_test_label());
}

TEST(CoverageLabelParser, CBIsTestLabelFLTMaxCost)
{
  VW::cb_label label;
  label.costs.push_back(VW::cb_class{FLT_MAX, 1, 0.5f});
  EXPECT_TRUE(label.is_test_label());
}

TEST(CoverageLabelParser, CBIsTestLabelZeroProbability)
{
  VW::cb_label label;
  label.costs.push_back(VW::cb_class{1.0f, 1, 0.0f});
  EXPECT_TRUE(label.is_test_label());
}

TEST(CoverageLabelParser, CBIsTestLabelValid)
{
  VW::cb_label label;
  label.costs.push_back(VW::cb_class{1.0f, 1, 0.5f});
  EXPECT_FALSE(label.is_test_label());
}

TEST(CoverageLabelParser, CBIsLabeledOppositeOfTest)
{
  VW::cb_label label;
  EXPECT_EQ(label.is_labeled(), !label.is_test_label());
  label.costs.push_back(VW::cb_class{1.0f, 1, 0.5f});
  EXPECT_EQ(label.is_labeled(), !label.is_test_label());
}

TEST(CoverageLabelParser, CBResetToDefault)
{
  VW::cb_label label;
  label.costs.push_back(VW::cb_class{0.5f, 1, 0.5f});
  label.weight = 2.0f;
  label.reset_to_default();
  EXPECT_TRUE(label.costs.empty());
  EXPECT_FLOAT_EQ(label.weight, 1.0f);
}

TEST(CoverageLabelParser, CBCacheRoundTrip)
{
  auto lp = VW::cb_label_parser_global;
  VW::polylabel original;
  parse_cb_label_b12(lp, "1:0.5:0.3", original);
  original.cb.weight = 2.0f;

  VW::io_buf write_buf;
  auto backing = std::make_shared<std::vector<char>>();
  write_buf.add_file(VW::io::create_vector_writer(backing));
  VW::reduction_features red_features;
  lp.cache_label(original, red_features, write_buf, "test", false);
  write_buf.flush();

  VW::io_buf read_buf;
  read_buf.add_file(VW::io::create_buffer_view(backing->data(), backing->size()));
  VW::polylabel restored;
  lp.default_label(restored);
  lp.read_cached_label(restored, red_features, read_buf);

  EXPECT_EQ(restored.cb.costs.size(), 1);
  EXPECT_FLOAT_EQ(restored.cb.costs[0].cost, 0.5f);
  EXPECT_FLOAT_EQ(restored.cb.costs[0].probability, 0.3f);
  EXPECT_FLOAT_EQ(restored.cb.weight, 2.0f);
}

TEST(CoverageLabelParser, CBLabelTypeIsCB)
{
  auto lp = VW::cb_label_parser_global;
  EXPECT_EQ(lp.label_type, VW::label_type_t::CB);
}

TEST(CoverageLabelParser, CBGetWeight)
{
  auto lp = VW::cb_label_parser_global;
  VW::polylabel label;
  VW::reduction_features red_features;
  lp.default_label(label);
  label.cb.weight = 3.5f;
  EXPECT_FLOAT_EQ(lp.get_weight(label, red_features), 3.5f);
}

TEST(CoverageLabelParser, CBEcIsExampleHeaderTrue)
{
  VW::example ex;
  ex.l.cb.costs.clear();
  ex.l.cb.costs.push_back(VW::cb_class{0.f, 0, -1.f});
  EXPECT_TRUE(VW::ec_is_example_header_cb(ex));
}

TEST(CoverageLabelParser, CBEcIsExampleHeaderFalseMultiple)
{
  VW::example ex;
  ex.l.cb.costs.clear();
  ex.l.cb.costs.push_back(VW::cb_class{0.f, 0, -1.f});
  ex.l.cb.costs.push_back(VW::cb_class{1.f, 1, 0.5f});
  EXPECT_FALSE(VW::ec_is_example_header_cb(ex));
}

TEST(CoverageLabelParser, CBEcIsExampleHeaderFalseNonShared)
{
  VW::example ex;
  ex.l.cb.costs.clear();
  ex.l.cb.costs.push_back(VW::cb_class{1.f, 1, 0.5f});
  EXPECT_FALSE(VW::ec_is_example_header_cb(ex));
}

TEST(CoverageLabelParser, CBGetObservedCostFound)
{
  VW::cb_label label;
  label.costs.push_back(VW::cb_class{1.5f, 2, 0.3f});
  auto result = VW::get_observed_cost_cb(label);
  EXPECT_TRUE(result.first);
  EXPECT_FLOAT_EQ(result.second.cost, 1.5f);
}

TEST(CoverageLabelParser, CBGetObservedCostNotFound)
{
  VW::cb_label label;
  label.costs.push_back(VW::cb_class{FLT_MAX, 1, 0.0f});
  auto result = VW::get_observed_cost_cb(label);
  EXPECT_FALSE(result.first);
}

TEST(CoverageLabelParser, CBGetObservedCostEmpty)
{
  VW::cb_label label;
  auto result = VW::get_observed_cost_cb(label);
  EXPECT_FALSE(result.first);
}

// ==================== CB Eval ====================

TEST(CoverageLabelParser, CBEvalDefaultLabel)
{
  auto lp = VW::cb_eval_label_parser_global;
  VW::polylabel label;
  lp.default_label(label);
  EXPECT_EQ(label.cb_eval.action, 0u);
  EXPECT_TRUE(label.cb_eval.event.costs.empty());
}

TEST(CoverageLabelParser, CBEvalLabelType)
{
  auto lp = VW::cb_eval_label_parser_global;
  EXPECT_EQ(lp.label_type, VW::label_type_t::CB_EVAL);
}

TEST(CoverageLabelParser, CBEvalCacheRoundTrip)
{
  VW::cb_eval_label original;
  original.action = 5;
  original.event.costs.push_back({0.5f, 1, 0.2f});
  original.event.weight = 1.5f;

  VW::io_buf write_buf;
  auto backing = std::make_shared<std::vector<char>>();
  write_buf.add_file(VW::io::create_vector_writer(backing));
  VW::model_utils::write_model_field(write_buf, original, "test", false);
  write_buf.flush();

  VW::io_buf read_buf;
  read_buf.add_file(VW::io::create_buffer_view(backing->data(), backing->size()));
  VW::cb_eval_label restored;
  VW::model_utils::read_model_field(read_buf, restored);

  EXPECT_EQ(restored.action, 5u);
  EXPECT_EQ(restored.event.costs.size(), 1);
  EXPECT_FLOAT_EQ(restored.event.weight, 1.5f);
}

TEST(CoverageLabelParser, CBEvalTestLabel)
{
  auto lp = VW::cb_eval_label_parser_global;
  VW::polylabel label;
  lp.default_label(label);
  EXPECT_TRUE(lp.test_label(label));
}

TEST(CoverageLabelParser, CBEvalGetWeight)
{
  auto lp = VW::cb_eval_label_parser_global;
  VW::polylabel label;
  VW::reduction_features red_features;
  lp.default_label(label);
  label.cb_eval.event.weight = 2.5f;
  EXPECT_FLOAT_EQ(lp.get_weight(label, red_features), 2.5f);
}

// ==================== 6. CB with Observations ====================

TEST(CoverageLabelParser, CBWithObsDefaultLabel)
{
  auto lp = VW::cb_with_observations_global;
  VW::polylabel label;
  lp.default_label(label);

  EXPECT_TRUE(label.cb_with_observations.event.costs.empty());
  EXPECT_FALSE(label.cb_with_observations.is_observation);
  EXPECT_FALSE(label.cb_with_observations.is_definitely_bad);
}

TEST(CoverageLabelParser, CBWithObsResetToDefault)
{
  VW::cb_with_observations_label label;
  label.event.costs.push_back(VW::cb_class{0.5f, 1, 0.5f});
  label.is_observation = true;
  label.is_definitely_bad = true;
  label.reset_to_default();

  EXPECT_TRUE(label.event.costs.empty());
  EXPECT_FALSE(label.is_observation);
  EXPECT_FALSE(label.is_definitely_bad);
}

TEST(CoverageLabelParser, CBWithObsIsTestLabel)
{
  VW::cb_with_observations_label label;
  label.reset_to_default();
  EXPECT_TRUE(label.is_test_label());

  label.event.costs.push_back(VW::cb_class{1.0f, 1, 0.5f});
  EXPECT_FALSE(label.is_test_label());
}

TEST(CoverageLabelParser, CBWithObsCacheRoundTrip)
{
  VW::cb_with_observations_label original;
  original.event.weight = 3.0f;
  original.event.costs.push_back(VW::cb_class{0.5f, 1, 0.3f});
  original.is_observation = true;
  original.is_definitely_bad = true;

  VW::io_buf write_buf;
  auto backing = std::make_shared<std::vector<char>>();
  write_buf.add_file(VW::io::create_vector_writer(backing));
  VW::model_utils::write_model_field(write_buf, original, "test", false);
  write_buf.flush();

  VW::io_buf read_buf;
  read_buf.add_file(VW::io::create_buffer_view(backing->data(), backing->size()));
  VW::cb_with_observations_label restored;
  restored.reset_to_default();
  VW::model_utils::read_model_field(read_buf, restored);

  EXPECT_FLOAT_EQ(restored.event.weight, 3.0f);
  EXPECT_EQ(restored.event.costs.size(), 1);
  EXPECT_FLOAT_EQ(restored.event.costs[0].cost, 0.5f);
  EXPECT_TRUE(restored.is_observation);
  EXPECT_TRUE(restored.is_definitely_bad);
}

TEST(CoverageLabelParser, CBWithObsTextParseThrows)
{
  auto lp = VW::cb_with_observations_global;
  VW::polylabel label;
  lp.default_label(label);
  std::vector<VW::string_view> words;
  words.push_back("something");
  VW::label_parser_reuse_mem mem;
  VW::reduction_features red_features;
  auto null_logger = VW::io::create_null_logger();
  EXPECT_THROW(lp.parse_label(label, red_features, mem, nullptr, words, null_logger), VW::vw_exception);
}

TEST(CoverageLabelParser, CBWithObsLabelType)
{
  auto lp = VW::cb_with_observations_global;
  EXPECT_EQ(lp.label_type, VW::label_type_t::CB_WITH_OBSERVATIONS);
}

TEST(CoverageLabelParser, CBWithObsExampleHeaderTrue)
{
  VW::example ex;
  ex.l.cb_with_observations.event.costs.clear();
  ex.l.cb_with_observations.event.costs.push_back(VW::cb_class{0.f, 0, -1.f});
  EXPECT_TRUE(VW::ec_is_example_header_cb_with_observations(ex));
}

TEST(CoverageLabelParser, CBWithObsExampleHeaderFalse)
{
  VW::example ex;
  ex.l.cb_with_observations.event.costs.clear();
  ex.l.cb_with_observations.event.costs.push_back(VW::cb_class{0.f, 1, 0.5f});
  EXPECT_FALSE(VW::ec_is_example_header_cb_with_observations(ex));
}

TEST(CoverageLabelParser, CBWithObsExampleHeaderMultiple)
{
  VW::example ex;
  ex.l.cb_with_observations.event.costs.clear();
  ex.l.cb_with_observations.event.costs.push_back(VW::cb_class{0.f, 0, -1.f});
  ex.l.cb_with_observations.event.costs.push_back(VW::cb_class{1.f, 1, 0.5f});
  EXPECT_FALSE(VW::ec_is_example_header_cb_with_observations(ex));
}

TEST(CoverageLabelParser, CBWithObsGetWeight)
{
  auto lp = VW::cb_with_observations_global;
  VW::polylabel label;
  VW::reduction_features red_features;
  lp.default_label(label);
  label.cb_with_observations.event.weight = 4.0f;
  EXPECT_FLOAT_EQ(lp.get_weight(label, red_features), 4.0f);
}

// ==================== 7. Multilabel ====================

TEST(CoverageLabelParser, MultilabelParseEmpty)
{
  auto lp = VW::multilabel_label_parser_global;
  VW::polylabel label;
  parse_multilabel_b12(lp, "", label);
  EXPECT_TRUE(label.multilabels.label_v.empty());
  EXPECT_TRUE(label.multilabels.is_test());
}

TEST(CoverageLabelParser, MultilabelParseSingle)
{
  auto lp = VW::multilabel_label_parser_global;
  VW::polylabel label;
  parse_multilabel_b12(lp, "3", label);
  EXPECT_EQ(label.multilabels.label_v.size(), 1);
  EXPECT_EQ(label.multilabels.label_v[0], 3u);
  EXPECT_FALSE(label.multilabels.is_test());
}

TEST(CoverageLabelParser, MultilabelParseMultiple)
{
  auto lp = VW::multilabel_label_parser_global;
  VW::polylabel label;
  parse_multilabel_b12(lp, "1,3,5", label);
  EXPECT_EQ(label.multilabels.label_v.size(), 3);
  EXPECT_EQ(label.multilabels.label_v[0], 1u);
  EXPECT_EQ(label.multilabels.label_v[1], 3u);
  EXPECT_EQ(label.multilabels.label_v[2], 5u);
}

TEST(CoverageLabelParser, MultilabelResetToDefault)
{
  VW::multilabel_label label;
  label.label_v.push_back(1);
  label.label_v.push_back(2);
  label.reset_to_default();
  EXPECT_TRUE(label.label_v.empty());
}

TEST(CoverageLabelParser, MultilabelIsTest)
{
  VW::multilabel_label label;
  EXPECT_TRUE(label.is_test());
  label.label_v.push_back(1);
  EXPECT_FALSE(label.is_test());
}

TEST(CoverageLabelParser, MultilabelWeightAlwaysOne)
{
  auto lp = VW::multilabel_label_parser_global;
  VW::polylabel label;
  VW::reduction_features red_features;
  parse_multilabel_b12(lp, "1,2,3", label);
  EXPECT_FLOAT_EQ(lp.get_weight(label, red_features), 1.0f);
}

TEST(CoverageLabelParser, MultilabelCacheRoundTrip)
{
  auto lp = VW::multilabel_label_parser_global;
  VW::polylabel original;
  parse_multilabel_b12(lp, "1,3,5,7", original);

  VW::io_buf write_buf;
  auto backing = std::make_shared<std::vector<char>>();
  write_buf.add_file(VW::io::create_vector_writer(backing));
  VW::reduction_features red_features;
  lp.cache_label(original, red_features, write_buf, "test", false);
  write_buf.flush();

  VW::io_buf read_buf;
  read_buf.add_file(VW::io::create_buffer_view(backing->data(), backing->size()));
  VW::polylabel restored;
  lp.default_label(restored);
  lp.read_cached_label(restored, red_features, read_buf);

  EXPECT_EQ(restored.multilabels.label_v.size(), 4);
  EXPECT_EQ(restored.multilabels.label_v[0], 1u);
  EXPECT_EQ(restored.multilabels.label_v[1], 3u);
  EXPECT_EQ(restored.multilabels.label_v[2], 5u);
  EXPECT_EQ(restored.multilabels.label_v[3], 7u);
}

TEST(CoverageLabelParser, MultilabelDefaultLabel)
{
  auto lp = VW::multilabel_label_parser_global;
  VW::polylabel label;
  lp.default_label(label);
  EXPECT_TRUE(label.multilabels.label_v.empty());
}

TEST(CoverageLabelParser, MultilabelLabelType)
{
  auto lp = VW::multilabel_label_parser_global;
  EXPECT_EQ(lp.label_type, VW::label_type_t::MULTILABEL);
}

TEST(CoverageLabelParser, MultilabelToStringEmpty)
{
  VW::multilabel_label label;
  EXPECT_EQ(VW::to_string(label), "");
}

TEST(CoverageLabelParser, MultilabelToString)
{
  VW::multilabel_label label;
  label.label_v.push_back(1);
  label.label_v.push_back(3);
  label.label_v.push_back(5);
  EXPECT_EQ(VW::to_string(label), "1,3,5");
}

TEST(CoverageLabelParser, MultilabelPredictionToString)
{
  VW::multilabel_prediction pred;
  pred.label_v.push_back(2);
  pred.label_v.push_back(4);
  EXPECT_EQ(VW::to_string(pred), "2,4");
}

TEST(CoverageLabelParser, MultilabelIntegration)
{
  auto vw = VW::initialize(vwtest::make_args("--multilabel_oaa", "5", "--quiet"));

  auto* ex = VW::read_example(*vw, "1,3,5 | a b c");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);

  auto* ex2 = VW::read_example(*vw, "| a b c");
  vw->predict(*ex2);
  VW::finish_example(*vw, *ex2);
}

// ==================== 8. Simple Labels ====================

TEST(CoverageLabelParser, SimpleParseBasic)
{
  auto lp = VW::simple_label_parser_global;
  VW::polylabel label;
  VW::reduction_features red_features;
  parse_simple_label_b12(lp, "1.0", label, red_features);

  EXPECT_FLOAT_EQ(label.simple.label, 1.0f);
  auto& slrf = red_features.template get<VW::simple_label_reduction_features>();
  EXPECT_FLOAT_EQ(slrf.weight, 1.0f);
  EXPECT_FLOAT_EQ(slrf.initial, 0.0f);
}

TEST(CoverageLabelParser, SimpleParseWithWeight)
{
  auto lp = VW::simple_label_parser_global;
  VW::polylabel label;
  VW::reduction_features red_features;
  parse_simple_label_b12(lp, "1.0 2.0", label, red_features);

  EXPECT_FLOAT_EQ(label.simple.label, 1.0f);
  auto& slrf = red_features.template get<VW::simple_label_reduction_features>();
  EXPECT_FLOAT_EQ(slrf.weight, 2.0f);
  EXPECT_FLOAT_EQ(slrf.initial, 0.0f);
}

TEST(CoverageLabelParser, SimpleParseWithWeightAndInitial)
{
  auto lp = VW::simple_label_parser_global;
  VW::polylabel label;
  VW::reduction_features red_features;
  parse_simple_label_b12(lp, "1.0 2.0 3.0", label, red_features);

  EXPECT_FLOAT_EQ(label.simple.label, 1.0f);
  auto& slrf = red_features.template get<VW::simple_label_reduction_features>();
  EXPECT_FLOAT_EQ(slrf.weight, 2.0f);
  EXPECT_FLOAT_EQ(slrf.initial, 3.0f);
}

TEST(CoverageLabelParser, SimpleParseEmpty)
{
  auto lp = VW::simple_label_parser_global;
  VW::polylabel label;
  VW::reduction_features red_features;
  parse_simple_label_b12(lp, "", label, red_features);

  // Empty label keeps default (0.0 from simple_label constructor)
  EXPECT_FLOAT_EQ(label.simple.label, FLT_MAX);
}

TEST(CoverageLabelParser, SimpleParseNegativeLabel)
{
  auto lp = VW::simple_label_parser_global;
  VW::polylabel label;
  VW::reduction_features red_features;
  parse_simple_label_b12(lp, "-5.5", label, red_features);

  EXPECT_FLOAT_EQ(label.simple.label, -5.5f);
}

TEST(CoverageLabelParser, SimpleParseZeroLabel)
{
  auto lp = VW::simple_label_parser_global;
  VW::polylabel label;
  VW::reduction_features red_features;
  parse_simple_label_b12(lp, "0.0", label, red_features);

  EXPECT_FLOAT_EQ(label.simple.label, 0.0f);
}

TEST(CoverageLabelParser, SimpleParseTooManyTokens)
{
  // 4+ tokens triggers error path
  auto lp = VW::simple_label_parser_global;
  VW::polylabel label;
  VW::reduction_features red_features;
  // Does not throw, just logs error
  parse_simple_label_b12(lp, "1.0 2.0 3.0 4.0", label, red_features);
}

TEST(CoverageLabelParser, SimpleIsTestLabel)
{
  auto lp = VW::simple_label_parser_global;
  VW::polylabel label;
  lp.default_label(label);
  // Default simple label has label=FLT_MAX which is test
  EXPECT_TRUE(lp.test_label(label));

  VW::reduction_features red_features;
  parse_simple_label_b12(lp, "1.0", label, red_features);
  EXPECT_FALSE(lp.test_label(label));
}

TEST(CoverageLabelParser, SimpleResetToDefault)
{
  VW::simple_label label(5.0f);
  EXPECT_FLOAT_EQ(label.label, 5.0f);
  label.reset_to_default();
  EXPECT_FLOAT_EQ(label.label, FLT_MAX);
}

TEST(CoverageLabelParser, SimpleDefaultLabel)
{
  auto lp = VW::simple_label_parser_global;
  VW::polylabel label;
  lp.default_label(label);
  EXPECT_FLOAT_EQ(label.simple.label, FLT_MAX);
}

TEST(CoverageLabelParser, SimpleGetWeight)
{
  auto lp = VW::simple_label_parser_global;
  VW::polylabel label;
  VW::reduction_features red_features;
  parse_simple_label_b12(lp, "1.0 2.5", label, red_features);

  EXPECT_FLOAT_EQ(lp.get_weight(label, red_features), 2.5f);
}

TEST(CoverageLabelParser, SimpleReductionFeaturesDefault)
{
  VW::simple_label_reduction_features slrf;
  EXPECT_FLOAT_EQ(slrf.weight, 1.0f);
  EXPECT_FLOAT_EQ(slrf.initial, 0.0f);
}

TEST(CoverageLabelParser, SimpleReductionFeaturesParameterized)
{
  VW::simple_label_reduction_features slrf(3.0f, 0.5f);
  EXPECT_FLOAT_EQ(slrf.weight, 3.0f);
  EXPECT_FLOAT_EQ(slrf.initial, 0.5f);
}

TEST(CoverageLabelParser, SimpleReductionFeaturesResetToDefault)
{
  VW::simple_label_reduction_features slrf(3.0f, 0.5f);
  slrf.reset_to_default();
  EXPECT_FLOAT_EQ(slrf.weight, 1.0f);
  EXPECT_FLOAT_EQ(slrf.initial, 0.0f);
}

TEST(CoverageLabelParser, SimpleCacheRoundTrip)
{
  auto lp = VW::simple_label_parser_global;
  VW::polylabel original;
  VW::reduction_features red_features_write;
  parse_simple_label_b12(lp, "1.5 2.5 0.75", original, red_features_write);

  VW::io_buf write_buf;
  auto backing = std::make_shared<std::vector<char>>();
  write_buf.add_file(VW::io::create_vector_writer(backing));
  lp.cache_label(original, red_features_write, write_buf, "test", false);
  write_buf.flush();

  VW::io_buf read_buf;
  read_buf.add_file(VW::io::create_buffer_view(backing->data(), backing->size()));
  VW::polylabel restored;
  VW::reduction_features red_features_read;
  lp.default_label(restored);
  lp.read_cached_label(restored, red_features_read, read_buf);

  EXPECT_FLOAT_EQ(restored.simple.label, 1.5f);
  auto& slrf = red_features_read.template get<VW::simple_label_reduction_features>();
  EXPECT_FLOAT_EQ(slrf.weight, 2.5f);
  EXPECT_FLOAT_EQ(slrf.initial, 0.75f);
}

TEST(CoverageLabelParser, SimpleLabelType)
{
  auto lp = VW::simple_label_parser_global;
  EXPECT_EQ(lp.label_type, VW::label_type_t::SIMPLE);
}

TEST(CoverageLabelParser, SimpleLabelEquality)
{
  VW::simple_label a(1.0f);
  VW::simple_label b(1.0f);
  VW::simple_label c(2.0f);
  EXPECT_TRUE(a == b);
  EXPECT_FALSE(a == c);
  EXPECT_TRUE(a != c);
  EXPECT_FALSE(a != b);
}

TEST(CoverageLabelParser, SimpleIntegration)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* ex = VW::read_example(*vw, "1.0 | a b c");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);

  auto* ex2 = VW::read_example(*vw, "| a b c");
  vw->predict(*ex2);
  VW::finish_example(*vw, *ex2);
}

TEST(CoverageLabelParser, SimpleModelSerializationLabel)
{
  VW::simple_label original(3.14f);

  VW::io_buf write_buf;
  auto backing = std::make_shared<std::vector<char>>();
  write_buf.add_file(VW::io::create_vector_writer(backing));
  VW::model_utils::write_model_field(write_buf, original, "test", false);
  write_buf.flush();

  VW::io_buf read_buf;
  read_buf.add_file(VW::io::create_buffer_view(backing->data(), backing->size()));
  VW::simple_label restored;
  VW::model_utils::read_model_field(read_buf, restored);

  EXPECT_FLOAT_EQ(restored.label, 3.14f);
}

TEST(CoverageLabelParser, SimpleModelSerializationReductionFeatures)
{
  VW::simple_label_reduction_features original(2.5f, 0.75f);

  VW::io_buf write_buf;
  auto backing = std::make_shared<std::vector<char>>();
  write_buf.add_file(VW::io::create_vector_writer(backing));
  VW::model_utils::write_model_field(write_buf, original, "test", false);
  write_buf.flush();

  VW::io_buf read_buf;
  read_buf.add_file(VW::io::create_buffer_view(backing->data(), backing->size()));
  VW::simple_label_reduction_features restored;
  VW::model_utils::read_model_field(read_buf, restored);

  EXPECT_FLOAT_EQ(restored.weight, 2.5f);
  EXPECT_FLOAT_EQ(restored.initial, 0.75f);
}
