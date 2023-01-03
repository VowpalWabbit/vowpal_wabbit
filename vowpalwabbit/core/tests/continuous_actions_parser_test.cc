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
