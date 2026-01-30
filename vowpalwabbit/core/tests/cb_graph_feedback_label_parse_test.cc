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
#include "vw/test_common/test_common.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <vector>

#ifdef VW_FEAT_CB_GRAPH_FEEDBACK_ENABLED
using namespace testing;
constexpr float EXPLICIT_FLOAT_TOL = 0.01f;

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

void check_graph_and_preds(const VW::multi_ex& examples)
{
  EXPECT_EQ(examples.size(), 3);

  EXPECT_FLOAT_EQ(examples[0]->l.cb.costs[0].probability, -1.f);
  auto& graph_reduction_features =
      examples[0]->ex_reduction_features.template get<VW::cb_graph_feedback::reduction_features>();

  EXPECT_EQ(graph_reduction_features.triplets.size(), 2);

  EXPECT_EQ(graph_reduction_features.triplets[0].row, 0);
  EXPECT_EQ(graph_reduction_features.triplets[0].col, 1);
  EXPECT_EQ(graph_reduction_features.triplets[0].val, 2);

  EXPECT_EQ(graph_reduction_features.triplets[1].row, 1);
  EXPECT_EQ(graph_reduction_features.triplets[1].col, 1);
  EXPECT_EQ(graph_reduction_features.triplets[1].val, 3);

  float sum = 0;
  for (auto& action_score : examples[0]->pred.a_s) { sum += action_score.score; }
  EXPECT_NEAR(sum, 1, EXPLICIT_FLOAT_TOL);
}

TEST(cb_fg_label_tests, parse_json_label)
{
  std::string json_text = R"(
{
  "Version": "1",
  "EventId": "event_id",
  "GUser": {
      "shared_feature": "feature"
    },
    "_graph" : [{"row": 0, "col": 1, "val": 2}, {"row": 1, "col": 1, "val": 3}],
    "_multi": [
      {
        "TAction": {
          "feature1": 3.0,
          "feature2": "name1"
        }
      },
      {
        "TAction2": {
          "feature3": 3.0,
          "feature4": "name2"
        }
      }
    ],
  "VWState": {
    "m": "N/A"
  }
}
)";

  auto vw = VW::initialize(vwtest::make_args("--json", "--quiet", "--cb_explore_adf", "--graph_feedback"));
  auto examples = vwtest::parse_json(*vw, json_text);

  vw->predict(examples);

  check_graph_and_preds(examples);

  VW::finish_example(*vw, examples);
}

TEST(cb_fg_label_tests, parse_json_label_graph_before_shared)
{
  std::string json_text = R"(
{
  "Version": "1",
  "EventId": "event_id",
  "_graph" : [{"row": 0, "col": 1, "val": 2}, {"row": 1, "col": 1, "val": 3}],
  "GUser": {
      "shared_feature": "feature"
    },
    "_multi": [
      {
        "TAction": {
          "feature1": 3.0,
          "feature2": "name1"
        }
      },
      {
        "TAction2": {
          "feature3": 3.0,
          "feature4": "name2"
        }
      }
    ],
  "VWState": {
    "m": "N/A"
  }
}
)";

  auto vw = VW::initialize(vwtest::make_args("--json", "--quiet", "--cb_explore_adf", "--graph_feedback"));
  auto examples = vwtest::parse_json(*vw, json_text);

  vw->predict(examples);

  check_graph_and_preds(examples);

  VW::finish_example(*vw, examples);
}

TEST(cb_fg_label_tests, parse_json_label_no_shared)
{
  std::string json_text = R"(
{
  "Version": "1",
  "EventId": "event_id",
    "_multi": [
      {
        "TAction": {
          "feature1": 3.0,
          "feature2": "name1"
        }
      },
      {
        "TAction2": {
          "feature3": 3.0,
          "feature4": "name2"
        }
      }
    ],
  "_graph" : [{"row": 0, "col": 1, "val": 2}, {"row": 1, "col": 1, "val": 3}],
  "VWState": {
    "m": "N/A"
  }
}
)";

  auto vw = VW::initialize(vwtest::make_args("--json", "--quiet", "--cb_explore_adf", "--graph_feedback"));
  auto examples = vwtest::parse_json(*vw, json_text);

  vw->predict(examples);

  check_graph_and_preds(examples);

  VW::finish_example(*vw, examples);
}

TEST(cb_fg_label_tests, parse_json_label_bad_graph)
{
  std::string json_text = R"(
{
  "Version": "1",
  "EventId": "event_id",
  "_graph" : [{"row": 0, "col": 1, "val": 2}, {"row": 5, "col": 5, "val": 3}],
    "_multi": [
      {
        "TAction": {
          "feature1": 3.0,
          "feature2": "name1"
        }
      },
      {
        "TAction2": {
          "feature3": 3.0,
          "feature4": "name2"
        }
      }
    ],
  "VWState": {
    "m": "N/A"
  }
}
)";

  auto vw = VW::initialize(vwtest::make_args("--json", "--quiet", "--cb_explore_adf", "--graph_feedback"));
  auto examples = vwtest::parse_json(*vw, json_text);

  vw->predict(examples);

  float sum = 0;
  for (auto& action_score : examples[0]->pred.a_s) { sum += action_score.score; }
  EXPECT_NEAR(sum, 1, EXPLICIT_FLOAT_TOL);

  VW::finish_example(*vw, examples);
}
#endif