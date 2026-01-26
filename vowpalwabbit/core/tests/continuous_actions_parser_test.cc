// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/common/string_view.h"
#include "vw/common/text_utils.h"
#include "vw/core/cb_continuous_label.h"
#include "vw/core/memory.h"
#include "vw/core/parse_primitives.h"
#include "vw/core/parser.h"
#include "vw/io/logger.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>
#include <vector>

void parse_label(VW::label_parser& lp, VW::string_view label, VW::polylabel& l, VW::reduction_features& red_fts)
{
  std::vector<VW::string_view> words;
  VW::tokenize(' ', label, words);
  lp.default_label(l);
  VW::label_parser_reuse_mem mem;
  auto null_logger = VW::io::create_null_logger();
  lp.parse_label(l, red_fts, mem, nullptr, words, null_logger);
}

TEST(Cats, ParseLabel)
{
  auto lp = VW::cb_continuous::the_label_parser;
  {
    auto plabel = VW::make_unique<VW::polylabel>();
    VW::reduction_features red_features;
    parse_label(lp, "ca 185.121:0.657567:6.20426e-05", *plabel, red_features);
    EXPECT_FLOAT_EQ(plabel->cb_cont.costs[0].pdf_value, 6.20426e-05);
    EXPECT_FLOAT_EQ(plabel->cb_cont.costs[0].cost, 0.657567);
    EXPECT_FLOAT_EQ(plabel->cb_cont.costs[0].action, 185.121);

    const auto& cats_reduction_features = red_features.template get<VW::continuous_actions::reduction_features>();
    EXPECT_EQ(cats_reduction_features.is_chosen_action_set(), false);
    EXPECT_EQ(cats_reduction_features.is_pdf_set(), false);
  }
}

TEST(Cats, ParseLabelAndPdf)
{
  auto lp = VW::cb_continuous::the_label_parser;
  {
    auto plabel = VW::make_unique<VW::polylabel>();
    VW::reduction_features red_features;

    parse_label(lp, "ca 185.121:0.657567:6.20426e-05 pdf 185:8109.67:2.10314e-06 8109.67:23959:6.20426e-05", *plabel,
        red_features);
    // check label
    EXPECT_FLOAT_EQ(plabel->cb_cont.costs[0].pdf_value, 6.20426e-05);
    EXPECT_FLOAT_EQ(plabel->cb_cont.costs[0].cost, 0.657567);
    EXPECT_FLOAT_EQ(plabel->cb_cont.costs[0].action, 185.121);

    // check pdf
    const auto& cats_reduction_features = red_features.template get<VW::continuous_actions::reduction_features>();
    EXPECT_EQ(cats_reduction_features.pdf.size(), 2);
    EXPECT_EQ(cats_reduction_features.is_chosen_action_set(), false);
    EXPECT_FLOAT_EQ(cats_reduction_features.pdf[0].left, 185.);
    EXPECT_FLOAT_EQ(cats_reduction_features.pdf[0].right, 8109.67);
    EXPECT_FLOAT_EQ(cats_reduction_features.pdf[0].pdf_value, 2.10314e-06);
    EXPECT_FLOAT_EQ(cats_reduction_features.pdf[1].left, 8109.67);
    EXPECT_FLOAT_EQ(cats_reduction_features.pdf[1].right, 23959.);
    EXPECT_FLOAT_EQ(cats_reduction_features.pdf[1].pdf_value, 6.20426e-05);
  }
}

TEST(Cats, ParseOnlyPdfNoLabel)
{
  auto lp = VW::cb_continuous::the_label_parser;
  {
    auto plabel = VW::make_unique<VW::polylabel>();
    VW::reduction_features red_features;
    parse_label(lp, "ca pdf 185:8109.67:2.10314e-06 8109.67:23959:6.20426e-05", *plabel, red_features);
    EXPECT_EQ(plabel->cb_cont.costs.size(), 0);

    const auto& cats_reduction_features = red_features.template get<VW::continuous_actions::reduction_features>();
    EXPECT_EQ(cats_reduction_features.pdf.size(), 2);
    EXPECT_EQ(cats_reduction_features.is_chosen_action_set(), false);
    EXPECT_FLOAT_EQ(cats_reduction_features.pdf[0].left, 185.);
    EXPECT_FLOAT_EQ(cats_reduction_features.pdf[0].right, 8109.67);
    EXPECT_FLOAT_EQ(cats_reduction_features.pdf[0].pdf_value, 2.10314e-06);
    EXPECT_FLOAT_EQ(cats_reduction_features.pdf[1].left, 8109.67);
    EXPECT_FLOAT_EQ(cats_reduction_features.pdf[1].right, 23959.);
    EXPECT_FLOAT_EQ(cats_reduction_features.pdf[1].pdf_value, 6.20426e-05);
  }
}

TEST(Cats, ParseMalformedPdf)
{
  auto lp = VW::cb_continuous::the_label_parser;
  {
    auto plabel = VW::make_unique<VW::polylabel>();
    VW::reduction_features red_features;

    parse_label(lp, "ca pdf 185:8109.67 8109.67:23959:6.20426e-05", *plabel, red_features);

    // check pdf
    const auto& cats_reduction_features = red_features.template get<VW::continuous_actions::reduction_features>();
    EXPECT_EQ(cats_reduction_features.pdf.size(), 0);
    EXPECT_EQ(cats_reduction_features.is_chosen_action_set(), false);
    EXPECT_EQ(cats_reduction_features.is_pdf_set(), false);
  }
}

TEST(Cats, ParseLabelAndChosenAction)
{
  auto lp = VW::cb_continuous::the_label_parser;
  {
    auto plabel = VW::make_unique<VW::polylabel>();
    VW::reduction_features red_features;
    parse_label(lp, "ca 185.121:0.657567:6.20426e-05 chosen_action 8110.121", *plabel, red_features);

    // check label
    EXPECT_FLOAT_EQ(plabel->cb_cont.costs[0].pdf_value, 6.20426e-05);
    EXPECT_FLOAT_EQ(plabel->cb_cont.costs[0].cost, 0.657567);
    EXPECT_FLOAT_EQ(plabel->cb_cont.costs[0].action, 185.121);

    // check chosen action
    const auto& cats_reduction_features = red_features.template get<VW::continuous_actions::reduction_features>();
    EXPECT_EQ(cats_reduction_features.is_pdf_set(), false);
    EXPECT_EQ(cats_reduction_features.is_chosen_action_set(), true);
    EXPECT_FLOAT_EQ(cats_reduction_features.chosen_action, 8110.121);
  }
}

TEST(Cats, ChosenActionOnlyNoLabel)
{
  auto lp = VW::cb_continuous::the_label_parser;
  {
    auto plabel = VW::make_unique<VW::polylabel>();
    VW::reduction_features red_features;
    parse_label(lp, "ca chosen_action 8110.121", *plabel, red_features);

    EXPECT_EQ(plabel->cb_cont.costs.size(), 0);
    // check chosen action
    const auto& cats_reduction_features = red_features.template get<VW::continuous_actions::reduction_features>();
    EXPECT_EQ(cats_reduction_features.is_pdf_set(), false);
    EXPECT_EQ(cats_reduction_features.is_chosen_action_set(), true);
    EXPECT_FLOAT_EQ(cats_reduction_features.chosen_action, 8110.121);
  }
}

TEST(Cats, ParseLabelPdfAndChosenAction)
{
  auto lp = VW::cb_continuous::the_label_parser;
  {
    auto plabel = VW::make_unique<VW::polylabel>();
    VW::reduction_features red_features;
    parse_label(lp,
        "ca 185.121:0.657567:6.20426e-05 pdf 185:8109.67:2.10314e-06 8109.67:23959:6.20426e-05 chosen_action 8110.121",
        *plabel, red_features);

    // check label
    EXPECT_FLOAT_EQ(plabel->cb_cont.costs[0].pdf_value, 6.20426e-05);
    EXPECT_FLOAT_EQ(plabel->cb_cont.costs[0].cost, 0.657567);
    EXPECT_FLOAT_EQ(plabel->cb_cont.costs[0].action, 185.121);

    const auto& cats_reduction_features = red_features.template get<VW::continuous_actions::reduction_features>();

    // check chosen action
    EXPECT_EQ(cats_reduction_features.is_chosen_action_set(), true);
    EXPECT_FLOAT_EQ(cats_reduction_features.chosen_action, 8110.121);

    // check pdf
    EXPECT_EQ(cats_reduction_features.pdf.size(), 2);
    EXPECT_FLOAT_EQ(cats_reduction_features.pdf[0].left, 185.);
    EXPECT_FLOAT_EQ(cats_reduction_features.pdf[0].right, 8109.67);
    EXPECT_FLOAT_EQ(cats_reduction_features.pdf[0].pdf_value, 2.10314e-06);
    EXPECT_FLOAT_EQ(cats_reduction_features.pdf[1].left, 8109.67);
    EXPECT_FLOAT_EQ(cats_reduction_features.pdf[1].right, 23959.);
    EXPECT_FLOAT_EQ(cats_reduction_features.pdf[1].pdf_value, 6.20426e-05);
  }
}

TEST(Cats, ParseNoLabel)
{
  auto lp = VW::cb_continuous::the_label_parser;
  {
    auto plabel = VW::make_unique<VW::polylabel>();
    VW::reduction_features red_features;
    parse_label(lp, "", *plabel, red_features);

    EXPECT_EQ(plabel->cb_cont.costs.size(), 0);
    const auto& cats_reduction_features = red_features.template get<VW::continuous_actions::reduction_features>();
    EXPECT_EQ(cats_reduction_features.is_pdf_set(), false);
    EXPECT_EQ(cats_reduction_features.is_chosen_action_set(), false);
  }
}

TEST(Cats, ParseNoLabelWPrefix)
{
  auto lp = VW::cb_continuous::the_label_parser;
  {
    auto plabel = VW::make_unique<VW::polylabel>();
    VW::reduction_features red_features;
    parse_label(lp, "ca", *plabel, red_features);

    EXPECT_EQ(plabel->cb_cont.costs.size(), 0);
    const auto& cats_reduction_features = red_features.template get<VW::continuous_actions::reduction_features>();
    EXPECT_EQ(cats_reduction_features.is_pdf_set(), false);
    EXPECT_EQ(cats_reduction_features.is_chosen_action_set(), false);
  }
}

TEST(Cats, CheckLabelForPrefix)
{
  auto lp = VW::cb_continuous::the_label_parser;
  {
    auto plabel = VW::make_unique<VW::polylabel>();
    VW::reduction_features red_features;
    EXPECT_THROW(parse_label(lp, "185.121:0.657567:6.20426e-05", *plabel, red_features), VW::vw_exception);
  }
}

TEST(Cats, IsTestLabelEmpty)
{
  VW::cb_continuous::continuous_label label;
  label.costs.clear();
  // Empty costs is a test label
  EXPECT_TRUE(label.is_test_label());
}

TEST(Cats, IsTestLabelFLTMaxCost)
{
  VW::cb_continuous::continuous_label label;
  label.costs.clear();
  VW::cb_continuous::continuous_label_elm elm{1.0f, FLT_MAX, 0.0f};
  label.costs.push_back(elm);
  // FLT_MAX cost with zero pdf_value is a test label
  EXPECT_TRUE(label.is_test_label());
}

TEST(Cats, IsTestLabelZeroPdf)
{
  VW::cb_continuous::continuous_label label;
  label.costs.clear();
  VW::cb_continuous::continuous_label_elm elm{1.0f, 0.5f, 0.0f};
  label.costs.push_back(elm);
  // Zero pdf_value (even with valid cost) is a test label
  EXPECT_TRUE(label.is_test_label());
}

TEST(Cats, IsTestLabelWithValidCostAndPdf)
{
  VW::cb_continuous::continuous_label label;
  label.costs.clear();
  VW::cb_continuous::continuous_label_elm elm{1.0f, 0.5f, 0.001f};
  label.costs.push_back(elm);
  // Valid cost and non-zero pdf_value is NOT a test label
  EXPECT_FALSE(label.is_test_label());
}

TEST(Cats, IsLabeled)
{
  VW::cb_continuous::continuous_label labeled;
  labeled.costs.clear();
  VW::cb_continuous::continuous_label_elm elm{1.0f, 0.5f, 0.001f};
  labeled.costs.push_back(elm);
  EXPECT_TRUE(labeled.is_labeled());

  VW::cb_continuous::continuous_label unlabeled;
  unlabeled.costs.clear();
  EXPECT_FALSE(unlabeled.is_labeled());
}

TEST(Cats, ResetToDefault)
{
  VW::cb_continuous::continuous_label label;
  VW::cb_continuous::continuous_label_elm elm{1.0f, 0.5f, 0.001f};
  label.costs.push_back(elm);
  EXPECT_EQ(label.costs.size(), 1);

  label.reset_to_default();
  EXPECT_EQ(label.costs.size(), 0);
}

TEST(Cats, ToStringElement)
{
  VW::cb_continuous::continuous_label_elm elm{1.5f, 0.25f, 0.001f};
  std::string str = VW::to_string(elm, 2);
  // Should contain action, cost, and pdf_value
  EXPECT_FALSE(str.empty());
  EXPECT_NE(str.find("1.5"), std::string::npos);
}

TEST(Cats, ToStringLabel)
{
  VW::cb_continuous::continuous_label label;
  VW::cb_continuous::continuous_label_elm elm{1.5f, 0.25f, 0.001f};
  label.costs.push_back(elm);

  std::string str = VW::to_string(label, 2);
  EXPECT_FALSE(str.empty());
  EXPECT_NE(str.find("cb_cont"), std::string::npos);
}

TEST(Cats, ParseNegativePdfValue)
{
  auto lp = VW::cb_continuous::the_label_parser;
  {
    auto plabel = VW::make_unique<VW::polylabel>();
    VW::reduction_features red_features;
    // Negative pdf_value should be reset to 0 with a warning
    parse_label(lp, "ca 185.121:0.657567:-0.001", *plabel, red_features);
    // pdf_value should be reset to 0
    EXPECT_FLOAT_EQ(plabel->cb_cont.costs[0].pdf_value, 0.0f);
  }
}

TEST(Cats, LabelTypeIsContinuous)
{
  auto lp = VW::cb_continuous::the_label_parser;
  EXPECT_EQ(lp.label_type, VW::label_type_t::CONTINUOUS);
}

TEST(Cats, GetWeightReturnsOne)
{
  auto lp = VW::cb_continuous::the_label_parser;
  VW::polylabel label;
  VW::reduction_features red_features;
  lp.default_label(label);
  EXPECT_FLOAT_EQ(lp.get_weight(label, red_features), 1.f);
}

TEST(Cats, DefaultLabel)
{
  auto lp = VW::cb_continuous::the_label_parser;
  VW::polylabel label;
  lp.default_label(label);
  EXPECT_EQ(label.cb_cont.costs.size(), 0);
}

TEST(Cats, TestLabelViaParser)
{
  auto lp = VW::cb_continuous::the_label_parser;
  {
    auto plabel = VW::make_unique<VW::polylabel>();
    VW::reduction_features red_features;
    parse_label(lp, "", *plabel, red_features);
    EXPECT_TRUE(lp.test_label(*plabel));
  }
  {
    auto plabel = VW::make_unique<VW::polylabel>();
    VW::reduction_features red_features;
    parse_label(lp, "ca 185.121:0.657567:6.20426e-05", *plabel, red_features);
    EXPECT_FALSE(lp.test_label(*plabel));
  }
}

TEST(Cats, CacheAndReadCachedLabel)
{
  auto lp = VW::cb_continuous::the_label_parser;

  // Create a label
  auto plabel = VW::make_unique<VW::polylabel>();
  VW::reduction_features red_features;
  parse_label(lp, "ca 185.121:0.657567:6.20426e-05 pdf 185:8109.67:2.10314e-06 chosen_action 8110.121", *plabel,
      red_features);

  // Cache the label
  VW::io_buf write_buffer;
  auto backing_buffer_write = std::make_shared<std::vector<char>>();
  write_buffer.add_file(VW::io::create_vector_writer(backing_buffer_write));
  size_t written = lp.cache_label(*plabel, red_features, write_buffer, "test", false);
  write_buffer.flush();
  EXPECT_GT(written, 0);

  // Read cached label
  VW::io_buf read_buffer;
  read_buffer.add_file(VW::io::create_buffer_view(backing_buffer_write->data(), backing_buffer_write->size()));
  VW::polylabel restored_label;
  VW::reduction_features restored_red_features;
  lp.default_label(restored_label);
  size_t read = lp.read_cached_label(restored_label, restored_red_features, read_buffer);
  EXPECT_GT(read, 0);

  // Verify restored values
  EXPECT_EQ(restored_label.cb_cont.costs.size(), plabel->cb_cont.costs.size());
  EXPECT_FLOAT_EQ(restored_label.cb_cont.costs[0].action, plabel->cb_cont.costs[0].action);
  EXPECT_FLOAT_EQ(restored_label.cb_cont.costs[0].cost, plabel->cb_cont.costs[0].cost);
  EXPECT_FLOAT_EQ(restored_label.cb_cont.costs[0].pdf_value, plabel->cb_cont.costs[0].pdf_value);
}

TEST(Cats, MultipleChosenActionsLastUsed)
{
  auto lp = VW::cb_continuous::the_label_parser;
  {
    auto plabel = VW::make_unique<VW::polylabel>();
    VW::reduction_features red_features;
    // Parse with multiple chosen_action entries - last one wins
    // (each chosen_action keyword triggers parse_chosen_action which overwrites)
    parse_label(lp, "ca chosen_action 100.0 chosen_action 200.0", *plabel, red_features);

    const auto& cats_reduction_features = red_features.template get<VW::continuous_actions::reduction_features>();
    EXPECT_EQ(cats_reduction_features.is_chosen_action_set(), true);
    EXPECT_FLOAT_EQ(cats_reduction_features.chosen_action, 200.0f);
  }
}
