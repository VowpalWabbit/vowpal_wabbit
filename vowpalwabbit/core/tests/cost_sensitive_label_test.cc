// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/cost_sensitive.h"

#include "vw/common/string_view.h"
#include "vw/common/text_utils.h"
#include "vw/core/io_buf.h"
#include "vw/core/memory.h"
#include "vw/core/model_utils.h"
#include "vw/core/parse_primitives.h"
#include "vw/core/parser.h"
#include "vw/core/vw.h"
#include "vw/io/logger.h"
#include "vw/test_common/test_common.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cfloat>
#include <cmath>
#include <memory>
#include <vector>

namespace
{
void parse_cs_label(VW::label_parser& lp, VW::string_view label, VW::polylabel& l)
{
  std::vector<VW::string_view> words;
  VW::tokenize(' ', label, words);
  lp.default_label(l);
  VW::label_parser_reuse_mem mem;
  VW::reduction_features red_features;
  auto null_logger = VW::io::create_null_logger();
  lp.parse_label(l, red_features, mem, nullptr, words, null_logger);
}
}  // namespace

TEST(CostSensitive, IsCSExampleHeaderWithShared)
{
  VW::example ex;
  ex.l.cs.costs.clear();

  // Shared header: class_index=0, x=-FLT_MAX
  VW::cs_class header_class{-FLT_MAX, 0, 0.f, 0.f};
  ex.l.cs.costs.push_back(header_class);

  EXPECT_TRUE(VW::is_cs_example_header(ex));
}

TEST(CostSensitive, IsCSExampleHeaderWithNonZeroClassIndex)
{
  VW::example ex;
  ex.l.cs.costs.clear();

  // Non-zero class index - not a header
  VW::cs_class not_header{-FLT_MAX, 1, 0.f, 0.f};
  ex.l.cs.costs.push_back(not_header);

  EXPECT_FALSE(VW::is_cs_example_header(ex));
}

TEST(CostSensitive, IsCSExampleHeaderWithNonNegFLTMax)
{
  VW::example ex;
  ex.l.cs.costs.clear();

  // class_index=0 but x is not -FLT_MAX
  VW::cs_class not_header{0.5f, 0, 0.f, 0.f};
  ex.l.cs.costs.push_back(not_header);

  EXPECT_FALSE(VW::is_cs_example_header(ex));
}

TEST(CostSensitive, IsCSExampleHeaderMultipleCosts)
{
  VW::example ex;
  ex.l.cs.costs.clear();

  // More than one cost - not a header
  VW::cs_class cost1{-FLT_MAX, 0, 0.f, 0.f};
  VW::cs_class cost2{1.0f, 1, 0.f, 0.f};
  ex.l.cs.costs.push_back(cost1);
  ex.l.cs.costs.push_back(cost2);

  EXPECT_FALSE(VW::is_cs_example_header(ex));
}

TEST(CostSensitive, IsCSExampleHeaderEmpty)
{
  VW::example ex;
  ex.l.cs.costs.clear();

  // Empty costs - not a header
  EXPECT_FALSE(VW::is_cs_example_header(ex));
}

TEST(CostSensitive, ParseSharedLabel)
{
  auto lp = VW::cs_label_parser_global;
  VW::polylabel label;
  parse_cs_label(lp, "shared", label);

  EXPECT_EQ(label.cs.costs.size(), 1);
  EXPECT_EQ(label.cs.costs[0].class_index, 0);
  EXPECT_FLOAT_EQ(label.cs.costs[0].x, -FLT_MAX);
}

TEST(CostSensitive, ParseLabelWithCost)
{
  auto lp = VW::cs_label_parser_global;
  VW::polylabel label;
  parse_cs_label(lp, "1:0.5", label);

  EXPECT_EQ(label.cs.costs.size(), 1);
  EXPECT_NE(label.cs.costs[0].class_index, 0);
  EXPECT_FLOAT_EQ(label.cs.costs[0].x, 0.5f);
}

TEST(CostSensitive, ParseMultipleCosts)
{
  auto lp = VW::cs_label_parser_global;
  VW::polylabel label;
  parse_cs_label(lp, "1:0.5 2:1.0 3:0.25", label);

  EXPECT_EQ(label.cs.costs.size(), 3);
  EXPECT_FLOAT_EQ(label.cs.costs[0].x, 0.5f);
  EXPECT_FLOAT_EQ(label.cs.costs[1].x, 1.0f);
  EXPECT_FLOAT_EQ(label.cs.costs[2].x, 0.25f);
}

TEST(CostSensitive, IsTestLabelEmpty)
{
  VW::cs_label label;
  label.costs.clear();
  // Empty label is a test label
  EXPECT_TRUE(label.is_test_label());
}

TEST(CostSensitive, IsTestLabelAllFLTMax)
{
  VW::cs_label label;
  label.costs.clear();
  VW::cs_class cost1{FLT_MAX, 1, 0.f, 0.f};
  VW::cs_class cost2{FLT_MAX, 2, 0.f, 0.f};
  label.costs.push_back(cost1);
  label.costs.push_back(cost2);
  // All costs are FLT_MAX - test label
  EXPECT_TRUE(label.is_test_label());
}

TEST(CostSensitive, IsTestLabelWithRealCosts)
{
  VW::cs_label label;
  label.costs.clear();
  VW::cs_class cost1{0.5f, 1, 0.f, 0.f};
  VW::cs_class cost2{1.0f, 2, 0.f, 0.f};
  label.costs.push_back(cost1);
  label.costs.push_back(cost2);
  // Real costs - not a test label
  EXPECT_FALSE(label.is_test_label());
}

TEST(CostSensitive, IsTestLabelMixedCosts)
{
  VW::cs_label label;
  label.costs.clear();
  VW::cs_class cost1{FLT_MAX, 1, 0.f, 0.f};
  VW::cs_class cost2{0.5f, 2, 0.f, 0.f};
  label.costs.push_back(cost1);
  label.costs.push_back(cost2);
  // At least one non-FLT_MAX cost - not a test label
  EXPECT_FALSE(label.is_test_label());
}

TEST(CostSensitive, ResetToDefault)
{
  VW::cs_label label;
  VW::cs_class cost1{0.5f, 1, 0.f, 0.f};
  label.costs.push_back(cost1);

  EXPECT_EQ(label.costs.size(), 1);
  label.reset_to_default();
  EXPECT_EQ(label.costs.size(), 0);
}

TEST(CostSensitive, ModelSerializationCSClass)
{
  VW::cs_class original{1.5f, 3, 0.25f, 0.75f};

  // Serialize
  VW::io_buf write_buffer;
  auto backing_buffer_write = std::make_shared<std::vector<char>>();
  write_buffer.add_file(VW::io::create_vector_writer(backing_buffer_write));
  size_t written = VW::model_utils::write_model_field(write_buffer, original, "test", false);
  write_buffer.flush();
  EXPECT_GT(written, 0);

  // Deserialize
  VW::io_buf read_buffer;
  read_buffer.add_file(VW::io::create_buffer_view(backing_buffer_write->data(), backing_buffer_write->size()));
  VW::cs_class restored;
  size_t read = VW::model_utils::read_model_field(read_buffer, restored);
  EXPECT_GT(read, 0);

  EXPECT_FLOAT_EQ(restored.x, original.x);
  EXPECT_EQ(restored.class_index, original.class_index);
  EXPECT_FLOAT_EQ(restored.partial_prediction, original.partial_prediction);
  EXPECT_FLOAT_EQ(restored.wap_value, original.wap_value);
}

TEST(CostSensitive, ModelSerializationCSLabel)
{
  VW::cs_label original;
  original.costs.push_back({0.5f, 1, 0.1f, 0.2f});
  original.costs.push_back({1.0f, 2, 0.3f, 0.4f});

  // Serialize
  VW::io_buf write_buffer;
  auto backing_buffer_write = std::make_shared<std::vector<char>>();
  write_buffer.add_file(VW::io::create_vector_writer(backing_buffer_write));
  size_t written = VW::model_utils::write_model_field(write_buffer, original, "test", false);
  write_buffer.flush();
  EXPECT_GT(written, 0);

  // Deserialize
  VW::io_buf read_buffer;
  read_buffer.add_file(VW::io::create_buffer_view(backing_buffer_write->data(), backing_buffer_write->size()));
  VW::cs_label restored;
  size_t read = VW::model_utils::read_model_field(read_buffer, restored);
  EXPECT_GT(read, 0);

  EXPECT_EQ(restored.costs.size(), original.costs.size());
  for (size_t i = 0; i < original.costs.size(); i++)
  {
    EXPECT_FLOAT_EQ(restored.costs[i].x, original.costs[i].x);
    EXPECT_EQ(restored.costs[i].class_index, original.costs[i].class_index);
  }
}

TEST(CostSensitive, LabelTypeIsCS)
{
  auto lp = VW::cs_label_parser_global;
  EXPECT_EQ(lp.label_type, VW::label_type_t::CS);
}

TEST(CostSensitive, GetWeightReturnsOne)
{
  auto lp = VW::cs_label_parser_global;
  VW::polylabel label;
  VW::reduction_features red_features;
  lp.default_label(label);
  // cs_weight always returns 1.0
  EXPECT_FLOAT_EQ(lp.get_weight(label, red_features), 1.f);
}

TEST(CostSensitive, ParseTestLabel)
{
  auto lp = VW::cs_label_parser_global;
  VW::polylabel label;
  // Parsing just a class number without cost makes it FLT_MAX
  parse_cs_label(lp, "1", label);

  EXPECT_EQ(label.cs.costs.size(), 1);
  EXPECT_FLOAT_EQ(label.cs.costs[0].x, FLT_MAX);
  EXPECT_TRUE(label.cs.is_test_label());
}

TEST(CostSensitive, ParseEmptyLabel)
{
  auto lp = VW::cs_label_parser_global;
  VW::polylabel label;
  parse_cs_label(lp, "", label);

  EXPECT_EQ(label.cs.costs.size(), 0);
  EXPECT_TRUE(label.cs.is_test_label());
}

TEST(CostSensitive, IntegrationWithVWCSOAA)
{
  auto vw = VW::initialize(vwtest::make_args("--csoaa", "3", "--quiet"));

  auto* ex = VW::read_example(*vw, "1:0.5 2:1.0 3:0.25 | a b c");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);

  auto* ex_pred = VW::read_example(*vw, "| a b c");
  vw->predict(*ex_pred);
  EXPECT_GE(ex_pred->pred.multiclass, 1);
  EXPECT_LE(ex_pred->pred.multiclass, 3);
  VW::finish_example(*vw, *ex_pred);
}
