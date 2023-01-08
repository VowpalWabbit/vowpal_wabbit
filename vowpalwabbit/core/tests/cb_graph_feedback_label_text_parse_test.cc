// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "gmock/gmock.h"
#include "vw/common/string_view.h"
#include "vw/common/text_utils.h"
#include "vw/core/cb.h"
#include "vw/core/parse_primitives.h"
#include "vw/core/parser.h"
#include "vw/io/logger.h"
#include "vw/test_common/matchers.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <vector>

using namespace testing;

void parse_cb_label(VW::label_parser& lp, VW::string_view label, VW::polylabel& l, VW::reduction_features& red_features)
{
  std::vector<VW::string_view> words;
  VW::tokenize(' ', label, words);
  lp.default_label(l);
  VW::label_parser_reuse_mem mem;
  auto null_logger = VW::io::create_null_logger();
  lp.parse_label(l, red_features, mem, nullptr, words, null_logger);
}

TEST(cb_gf_label_tests, cb_graph_parse_label)
{
  VW::polylabel label;
  auto lp = VW::cb_label_parser_global;
  VW::reduction_features red_features;
  parse_cb_label(lp, "shared graph 0,1,2 1,2,3 5,5,5 | s_1 s_2", label, red_features);
  // it is still a shared example
  EXPECT_FLOAT_EQ(label.cb.costs[0].probability, -1.f);
  auto& graph_reduction_features = red_features.template get<VW::cb_graph_feedback::reduction_features>();
  EXPECT_EQ(graph_reduction_features.triplets.size(), 3);

  EXPECT_EQ(graph_reduction_features.triplets[0].row, 0);
  EXPECT_EQ(graph_reduction_features.triplets[0].col, 1);
  EXPECT_EQ(graph_reduction_features.triplets[0].val, 2);

  EXPECT_EQ(graph_reduction_features.triplets[1].row, 1);
  EXPECT_EQ(graph_reduction_features.triplets[1].col, 2);
  EXPECT_EQ(graph_reduction_features.triplets[1].val, 3);

  EXPECT_EQ(graph_reduction_features.triplets[2].row, 5);
  EXPECT_EQ(graph_reduction_features.triplets[2].col, 5);
  EXPECT_EQ(graph_reduction_features.triplets[2].val, 5);
}

TEST(cb_gf_label_tests, cb_graph_parse_label_no_fts)
{
  VW::polylabel label;
  auto lp = VW::cb_label_parser_global;
  VW::reduction_features red_features;
  parse_cb_label(lp, "shared graph 0,1,2 1,2,3 5,5,5 |", label, red_features);
  // it is still a shared example
  EXPECT_FLOAT_EQ(label.cb.costs[0].probability, -1.f);
  auto& graph_reduction_features = red_features.template get<VW::cb_graph_feedback::reduction_features>();
  EXPECT_EQ(graph_reduction_features.triplets.size(), 3);

  EXPECT_EQ(graph_reduction_features.triplets[0].row, 0);
  EXPECT_EQ(graph_reduction_features.triplets[0].col, 1);
  EXPECT_EQ(graph_reduction_features.triplets[0].val, 2);

  EXPECT_EQ(graph_reduction_features.triplets[1].row, 1);
  EXPECT_EQ(graph_reduction_features.triplets[1].col, 2);
  EXPECT_EQ(graph_reduction_features.triplets[1].val, 3);

  EXPECT_EQ(graph_reduction_features.triplets[2].row, 5);
  EXPECT_EQ(graph_reduction_features.triplets[2].col, 5);
  EXPECT_EQ(graph_reduction_features.triplets[2].val, 5);
}

TEST(cb_gf_label_tests, throws_w_malformed_triplet)
{
  VW::polylabel label;
  auto lp = VW::cb_label_parser_global;
  VW::reduction_features red_features;
  EXPECT_THROW(parse_cb_label(lp, "shared graph 0,1,2 0,1,2 5 | s_1 s_2", label, red_features), VW::vw_exception);
}

TEST(cb_gf_label_tests, throws_w_standalone_graph_example)
{
  VW::polylabel label;
  auto lp = VW::cb_label_parser_global;
  VW::reduction_features red_features;
  EXPECT_THROW(parse_cb_label(lp, "graph 0,1,2 0,1,2 5,5,5 | s_1 s_2", label, red_features), VW::vw_exception);
}

TEST(cb_gf_label_tests, throws_w_graph_in_label)
{
  VW::polylabel label;
  auto lp = VW::cb_label_parser_global;
  VW::reduction_features red_features;
  EXPECT_THROW(
      parse_cb_label(lp, "0:1.0:0.5 graph 0,1,2 0,1,2 5,5,5  | a_1 b_1 c_1", label, red_features), VW::vw_exception);
}
