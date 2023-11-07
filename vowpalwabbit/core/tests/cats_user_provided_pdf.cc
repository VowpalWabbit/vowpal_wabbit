// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/test_common/test_common.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <vector>

TEST(Cats, NoModelActionProvided)
{
  std::string json_text = R"(
{
  "Version": "1",
  "EventId": "event_id",
  "pdf": [
    {"chosen_action": 185.121}
  ],
  "c": {
    "f":1
  },
  "VWState": {
    "m": "N/A"
  }
}
)";
  auto vw = VW::initialize(vwtest::make_args("--dsjson", "--chain_hash", "--cats", "4", "--min_value=185",
      "--max_value=23959", "--bandwidth", "1", "--no_stdin", "--quiet", "--first_only"));
  auto examples = vwtest::parse_dsjson(*vw, json_text);

  EXPECT_EQ(examples.size(), 1);

  const auto& reduction_features =
      examples[0]->ex_reduction_features.template get<VW::continuous_actions::reduction_features>();

  EXPECT_EQ(reduction_features.is_pdf_set(), false);
  EXPECT_EQ(reduction_features.is_chosen_action_set(), true);

  EXPECT_FLOAT_EQ(reduction_features.chosen_action, 185.121f);

  vw->predict(*examples[0]);

  EXPECT_GE(examples[0]->pred.pdf_value.action, 185);
  EXPECT_GT(examples[0]->pred.pdf_value.pdf_value, 0.);

  VW::finish_example(*vw, examples);
}

TEST(Cats, PdfNoModelActionProvided)
{
  std::string json_text = R"(
{
  "Version": "1",
  "EventId": "event_id",
  "pdf": [
    {"chosen_action": 185.121}
  ],
  "c": {
    "f":1
  },
  "VWState": {
    "m": "N/A"
  }
}
)";
  auto vw = VW::initialize(vwtest::make_args("--dsjson", "--chain_hash", "--cats_pdf", "32", "--min_value=185",
      "--max_value=23959", "--bandwidth", "1000", "--no_stdin", "--quiet", "--first_only"));
  auto examples = vwtest::parse_dsjson(*vw, json_text);

  EXPECT_EQ(examples.size(), 1);

  const auto& reduction_features =
      examples[0]->ex_reduction_features.template get<VW::continuous_actions::reduction_features>();

  EXPECT_EQ(reduction_features.is_pdf_set(), false);
  EXPECT_EQ(reduction_features.is_chosen_action_set(), true);

  EXPECT_FLOAT_EQ(reduction_features.chosen_action, 185.121f);

  vw->predict(*examples[0]);

  EXPECT_GT(examples[0]->pred.pdf.size(), 1);

  float sum = 0;
  for (auto& p : examples[0]->pred.pdf) { sum += (p.right - p.left) * p.pdf_value; }
  EXPECT_FLOAT_EQ(sum, 1.f);

  VW::finish_example(*vw, examples);
}

TEST(Cats, PdfNoModelUniformRandom)
{
  std::string json_text = R"(
{
  "Version": "1",
  "EventId": "event_id",
  "c": {
    "f":1
  },
  "VWState": {
    "m": "N/A"
  }
}
)";

  float min_value = 185;
  float max_value = 23959;
  float epsilon = 0.1f;
  auto vw = VW::initialize(vwtest::make_args("--dsjson", "--chain_hash", "--cats_pdf", "4", "--min_value",
      std::to_string(min_value), "--max_value", std::to_string(max_value), "--epsilon", std::to_string(epsilon),
      "--bandwidth", "1", "--no_stdin", "--quiet", "--first_only"));
  auto examples = vwtest::parse_dsjson(*vw, json_text);

  EXPECT_EQ(examples.size(), 1);

  const auto& reduction_features =
      examples[0]->ex_reduction_features.template get<VW::continuous_actions::reduction_features>();

  EXPECT_EQ(reduction_features.is_pdf_set(), false);
  EXPECT_EQ(reduction_features.is_chosen_action_set(), false);

  vw->predict(*examples[0]);

  float sum = 0;
  for (auto& p : examples[0]->pred.pdf) { sum += (p.right - p.left) * p.pdf_value; }
  EXPECT_FLOAT_EQ(sum, 1.f);

  EXPECT_EQ(examples[0]->pred.pdf.size(), 1);
  EXPECT_FLOAT_EQ(examples[0]->pred.pdf[0].left, min_value);
  EXPECT_FLOAT_EQ(examples[0]->pred.pdf[0].right, max_value);
  EXPECT_FLOAT_EQ(examples[0]->pred.pdf[0].pdf_value, static_cast<float>(1.f / (max_value - min_value)));

  VW::finish_example(*vw, examples);
}

TEST(Cats, PdfNoModelPdfProvided)
{
  std::string json_text = R"(
{
  "Version": "1",
  "EventId": "event_id",
  "pdf" : [{"left": 185, "right": 8109.67, "pdf_value": 2.10314e-06}, {"left": 8109.67, "right": 23959, "pdf_value": 6.20426e-05}],
  "c": {
    "f":1
  },
  "VWState": {
    "m": "N/A"
  }
}
)";

  float min_value = 185;
  float max_value = 23959;
  float epsilon = 0.1f;
  auto vw = VW::initialize(vwtest::make_args("--dsjson", "--chain_hash", "--cats_pdf", "32", "--min_value",
      std::to_string(min_value), "--max_value", std::to_string(max_value), "--epsilon", std::to_string(epsilon),
      "--bandwidth", "1000", "--no_stdin", "--quiet", "--first_only"));
  auto examples = vwtest::parse_dsjson(*vw, json_text);

  EXPECT_EQ(examples.size(), 1);

  const auto& reduction_features =
      examples[0]->ex_reduction_features.template get<VW::continuous_actions::reduction_features>();

  EXPECT_EQ(reduction_features.is_pdf_set(), true);
  EXPECT_EQ(reduction_features.is_chosen_action_set(), false);

  EXPECT_EQ(reduction_features.pdf.size(), 2);

  vw->predict(*examples[0]);

  float sum = 0;
  for (auto& p : examples[0]->pred.pdf) { sum += (p.right - p.left) * p.pdf_value; }
  EXPECT_FLOAT_EQ(sum, 1.f);

  EXPECT_EQ(examples[0]->pred.pdf.size(), 2);
  EXPECT_FLOAT_EQ(examples[0]->pred.pdf[0].left, min_value);
  EXPECT_FLOAT_EQ(examples[0]->pred.pdf[0].right, 8109.67);
  EXPECT_FLOAT_EQ(examples[0]->pred.pdf[0].pdf_value, 2.10314e-06);

  EXPECT_FLOAT_EQ(examples[0]->pred.pdf[1].left, 8109.67);
  EXPECT_FLOAT_EQ(examples[0]->pred.pdf[1].right, max_value);
  EXPECT_FLOAT_EQ(examples[0]->pred.pdf[1].pdf_value, 6.20426e-05);

  VW::finish_example(*vw, examples);
}