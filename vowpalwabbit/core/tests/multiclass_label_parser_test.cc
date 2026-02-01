// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/common/string_view.h"
#include "vw/common/text_utils.h"
#include "vw/core/global_data.h"
#include "vw/core/io_buf.h"
#include "vw/core/memory.h"
#include "vw/core/model_utils.h"
#include "vw/core/multiclass.h"
#include "vw/core/named_labels.h"
#include "vw/core/parse_primitives.h"
#include "vw/core/parser.h"
#include "vw/core/shared_data.h"
#include "vw/core/vw.h"
#include "vw/test_common/test_common.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <climits>
#include <limits>

void parse_label(VW::label_parser& lp, VW::string_view label, VW::polylabel& l)
{
  std::vector<VW::string_view> words;
  VW::tokenize(' ', label, words);
  lp.default_label(l);
  VW::reduction_features red_fts;
  VW::label_parser_reuse_mem mem;
  auto null_logger = VW::io::create_null_logger();
  lp.parse_label(l, red_fts, mem, nullptr, words, null_logger);
}

void parse_label_with_ldict(
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

TEST(MulticlassLabelParser, MulticlassLabelParser)
{
  auto lp = VW::multiclass_label_parser_global;
  {
    auto plabel = VW::make_unique<VW::polylabel>();
    EXPECT_THROW(parse_label(lp, "1,2,3", *plabel), VW::vw_exception);
  }
  {
    auto plabel = VW::make_unique<VW::polylabel>();
    EXPECT_THROW(parse_label(lp, "1a", *plabel), VW::vw_exception);
  }
  {
    auto plabel = VW::make_unique<VW::polylabel>();
    EXPECT_THROW(parse_label(lp, "1 2 3", *plabel), VW::vw_exception);
  }
  {
    auto plabel = VW::make_unique<VW::polylabel>();
    parse_label(lp, "2", *plabel);
    EXPECT_TRUE(plabel->multi.label == 2);
    EXPECT_TRUE(plabel->multi.weight == 1.0);
  }
  {
    auto plabel = VW::make_unique<VW::polylabel>();
    parse_label(lp, "2 2", *plabel);
    EXPECT_TRUE(plabel->multi.label == 2);
    EXPECT_TRUE(plabel->multi.weight == 2.0);
  }
}

// --- Named labels (ldict) parsing path ---

TEST(MulticlassLabelParser, ParseWithNamedLabels)
{
  auto lp = VW::multiclass_label_parser_global;
  VW::named_labels ldict("cat,dog,bird");

  auto plabel = VW::make_unique<VW::polylabel>();
  parse_label_with_ldict(lp, "dog", *plabel, &ldict);
  EXPECT_EQ(plabel->multi.label, 2u);  // dog is the second label (1-indexed)
  EXPECT_FLOAT_EQ(plabel->multi.weight, 1.0f);
}

TEST(MulticlassLabelParser, ParseWithNamedLabelsAndWeight)
{
  auto lp = VW::multiclass_label_parser_global;
  VW::named_labels ldict("cat,dog,bird");

  auto plabel = VW::make_unique<VW::polylabel>();
  parse_label_with_ldict(lp, "bird 3.0", *plabel, &ldict);
  EXPECT_EQ(plabel->multi.label, 3u);  // bird is the third label
  EXPECT_FLOAT_EQ(plabel->multi.weight, 3.0f);
}

TEST(MulticlassLabelParser, ParseEmptyLabel)
{
  auto lp = VW::multiclass_label_parser_global;
  auto plabel = VW::make_unique<VW::polylabel>();
  parse_label(lp, "", *plabel);

  // Empty label should give default (test) label
  EXPECT_TRUE(VW::test_multiclass_label(plabel->multi));
}

// --- multiclass_label constructors ---

TEST(MulticlassLabelParser, DefaultConstructor)
{
  VW::multiclass_label ml;
  EXPECT_EQ(ml.label, std::numeric_limits<uint32_t>::max());
  EXPECT_FLOAT_EQ(ml.weight, 1.0f);
  EXPECT_FALSE(ml.is_labeled());
}

TEST(MulticlassLabelParser, ParameterizedConstructor)
{
  VW::multiclass_label ml(5, 2.0f);
  EXPECT_EQ(ml.label, 5u);
  EXPECT_FLOAT_EQ(ml.weight, 2.0f);
  EXPECT_TRUE(ml.is_labeled());
}

TEST(MulticlassLabelParser, ResetToDefault)
{
  VW::multiclass_label ml(5, 2.0f);
  ml.reset_to_default();
  EXPECT_EQ(ml.label, std::numeric_limits<uint32_t>::max());
  EXPECT_FLOAT_EQ(ml.weight, 1.0f);
}

// --- multiclass_label_weight ---

TEST(MulticlassLabelParser, GetWeightPositive)
{
  auto lp = VW::multiclass_label_parser_global;
  VW::polylabel label;
  VW::reduction_features red_fts;
  lp.default_label(label);
  label.multi.weight = 2.5f;
  EXPECT_FLOAT_EQ(lp.get_weight(label, red_fts), 2.5f);
}

TEST(MulticlassLabelParser, GetWeightZero)
{
  auto lp = VW::multiclass_label_parser_global;
  VW::polylabel label;
  VW::reduction_features red_fts;
  lp.default_label(label);
  label.multi.weight = 0.0f;
  EXPECT_FLOAT_EQ(lp.get_weight(label, red_fts), 0.0f);
}

TEST(MulticlassLabelParser, GetWeightNegative)
{
  auto lp = VW::multiclass_label_parser_global;
  VW::polylabel label;
  VW::reduction_features red_fts;
  lp.default_label(label);
  label.multi.weight = -1.0f;
  // Negative weight should return 0
  EXPECT_FLOAT_EQ(lp.get_weight(label, red_fts), 0.0f);
}

// --- Model serialization round-trip ---

TEST(MulticlassLabelParser, ModelSerializationRoundTrip)
{
  VW::multiclass_label original(7, 2.5f);

  // Serialize
  VW::io_buf write_buf;
  auto backing = std::make_shared<std::vector<char>>();
  write_buf.add_file(VW::io::create_vector_writer(backing));
  size_t written = VW::model_utils::write_model_field(write_buf, original, "test", false);
  write_buf.flush();
  EXPECT_GT(written, 0u);

  // Deserialize
  VW::io_buf read_buf;
  read_buf.add_file(VW::io::create_buffer_view(backing->data(), backing->size()));
  VW::multiclass_label restored;
  size_t read = VW::model_utils::read_model_field(read_buf, restored);
  EXPECT_GT(read, 0u);

  EXPECT_EQ(restored.label, 7u);
  EXPECT_FLOAT_EQ(restored.weight, 2.5f);
}

// --- Cache label round-trip via label_parser ---

TEST(MulticlassLabelParser, CacheLabelRoundTrip)
{
  auto lp = VW::multiclass_label_parser_global;
  VW::polylabel original;
  parse_label(lp, "5 2.0", original);

  // Cache
  VW::io_buf write_buf;
  auto backing = std::make_shared<std::vector<char>>();
  write_buf.add_file(VW::io::create_vector_writer(backing));
  VW::reduction_features red_fts;
  lp.cache_label(original, red_fts, write_buf, "test", false);
  write_buf.flush();

  // Read back
  VW::io_buf read_buf;
  read_buf.add_file(VW::io::create_buffer_view(backing->data(), backing->size()));
  VW::polylabel restored;
  lp.default_label(restored);
  lp.read_cached_label(restored, red_fts, read_buf);

  EXPECT_EQ(restored.multi.label, 5u);
  EXPECT_FLOAT_EQ(restored.multi.weight, 2.0f);
}

// --- Integration test with named labels for output paths ---

TEST(MulticlassLabelParser, IntegrationOAAWithNamedLabels)
{
  auto vw = VW::initialize(vwtest::make_args("--oaa", "3", "--named_labels", "cat,dog,bird", "--quiet"));

  auto* ex = VW::read_example(*vw, "cat | a b c");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);

  ex = VW::read_example(*vw, "dog | d e f");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);

  ex = VW::read_example(*vw, "bird | g h i");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);

  // Predict
  auto* pred_ex = VW::read_example(*vw, "| a b c");
  vw->predict(*pred_ex);
  EXPECT_GE(pred_ex->pred.multiclass, 1u);
  EXPECT_LE(pred_ex->pred.multiclass, 3u);
  VW::finish_example(*vw, *pred_ex);
}

// Integration with probabilities mode for print_probability path
TEST(MulticlassLabelParser, IntegrationOAAWithProbabilities)
{
  auto vw = VW::initialize(vwtest::make_args("--oaa", "3", "--probabilities", "--quiet"));

  for (int i = 0; i < 20; ++i)
  {
    std::string label = std::to_string((i % 3) + 1);
    std::string example = label + " |f x:" + std::to_string(i);
    auto* ex = VW::read_example(*vw, example);
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }

  // With --probabilities, prediction type is MULTICLASS_PROBS (scalars)
  auto* pred_ex = VW::read_example(*vw, "|f x:5");
  vw->predict(*pred_ex);
  // Should have 3 probability values
  EXPECT_EQ(pred_ex->pred.scalars.size(), 3u);
  // Probabilities should sum to approximately 1.0
  float sum = 0.0f;
  for (float p : pred_ex->pred.scalars) { sum += p; }
  EXPECT_NEAR(sum, 1.0f, 0.01f);
  VW::finish_example(*vw, *pred_ex);
}