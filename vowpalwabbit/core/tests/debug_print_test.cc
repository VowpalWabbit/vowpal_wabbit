// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/debug_print.h"

#include "vw/core/example.h"
#include "vw/core/vw.h"
#include "vw/test_common/test_common.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace testing;

TEST(DebugPrintTest, SimpleLabelToString)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* ex = VW::read_example(*vw, "1.5 |f a b");
  VW::setup_example(*vw, ex);

  std::string result = VW::debug::simple_label_to_string(*ex);

  EXPECT_THAT(result, HasSubstr("l=1.5"));
  EXPECT_THAT(result, HasSubstr("w="));

  vw->finish_example(*ex);
}

TEST(DebugPrintTest, CbLabelToString)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--cb_explore_adf"));

  VW::multi_ex examples;
  examples.push_back(VW::read_example(*vw, "shared |User user=Tom"));
  examples.push_back(VW::read_example(*vw, "0:0.5:0.25 |Action a=1"));
  examples.push_back(VW::read_example(*vw, "|Action a=2"));
  VW::setup_examples(*vw, examples);

  // The CB label example should have costs
  std::string result = VW::debug::cb_label_to_string(*examples[1]);

  EXPECT_THAT(result, HasSubstr("l.cb="));
  EXPECT_THAT(result, HasSubstr("c="));  // cost
  EXPECT_THAT(result, HasSubstr("a="));  // action
  EXPECT_THAT(result, HasSubstr("p="));  // probability

  vw->finish_example(examples);
}

TEST(DebugPrintTest, CbLabelToStringEmpty)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--cb_explore_adf"));

  VW::multi_ex examples;
  examples.push_back(VW::read_example(*vw, "shared |User user=Tom"));
  examples.push_back(VW::read_example(*vw, "|Action a=1"));
  VW::setup_examples(*vw, examples);

  // Action without label should have empty costs
  std::string result = VW::debug::cb_label_to_string(*examples[1]);

  EXPECT_THAT(result, HasSubstr("[l.cb={}]"));

  vw->finish_example(examples);
}

TEST(DebugPrintTest, ScalarPredToString)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* ex = VW::read_example(*vw, "1 |f a b");
  VW::setup_example(*vw, ex);
  vw->predict(*ex);

  std::string result = VW::debug::scalar_pred_to_string(*ex);

  EXPECT_THAT(result, HasSubstr("[p="));
  EXPECT_THAT(result, HasSubstr("pp="));

  vw->finish_example(*ex);
}

TEST(DebugPrintTest, ActionScoresPredToString)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--cb_explore_adf"));

  VW::multi_ex examples;
  examples.push_back(VW::read_example(*vw, "shared |User user=Tom"));
  examples.push_back(VW::read_example(*vw, "|Action a=1"));
  examples.push_back(VW::read_example(*vw, "|Action a=2"));
  VW::setup_examples(*vw, examples);
  vw->predict(examples);

  // Shared example should have action scores
  std::string result = VW::debug::a_s_pred_to_string(*examples[0]);

  EXPECT_THAT(result, HasSubstr("ec.pred.a_s"));

  vw->finish_example(examples);
}

TEST(DebugPrintTest, MulticlassPredToString)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--oaa", "3"));
  auto* ex = VW::read_example(*vw, "1 |f a b");
  VW::setup_example(*vw, ex);
  vw->predict(*ex);

  std::string result = VW::debug::multiclass_pred_to_string(*ex);

  EXPECT_THAT(result, HasSubstr("ec.pred.multiclass = "));

  vw->finish_example(*ex);
}

TEST(DebugPrintTest, FeaturesToString)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* ex = VW::read_example(*vw, "1 |f a b c");
  VW::setup_example(*vw, ex);

  std::string result = VW::debug::features_to_string(*ex);

  EXPECT_THAT(result, HasSubstr("[off="));
  EXPECT_THAT(result, HasSubstr("[h="));
  EXPECT_THAT(result, HasSubstr("v="));

  vw->finish_example(*ex);
}

TEST(DebugPrintTest, FeaturesToStringEmpty)
{
  VW::example_predict ep;

  std::string result = VW::debug::features_to_string(ep);

  EXPECT_THAT(result, HasSubstr("[off="));
}

TEST(DebugPrintTest, DebugDepthIndentStringZero)
{
  std::string result = VW::debug::debug_depth_indent_string(0);

  EXPECT_EQ(result, "- ");
}

TEST(DebugPrintTest, DebugDepthIndentStringNonZero)
{
  std::string result1 = VW::debug::debug_depth_indent_string(1);
  EXPECT_EQ(result1, "- ");

  std::string result2 = VW::debug::debug_depth_indent_string(2);
  EXPECT_EQ(result2, "  - ");

  std::string result3 = VW::debug::debug_depth_indent_string(3);
  EXPECT_EQ(result3, "    - ");
}

TEST(DebugPrintTest, DebugDepthIndentStringFromExample)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* ex = VW::read_example(*vw, "1 |f a b");
  VW::setup_example(*vw, ex);

  ex->debug_current_reduction_depth = 2;
  std::string result = VW::debug::debug_depth_indent_string(*ex);

  EXPECT_EQ(result, "  - ");

  vw->finish_example(*ex);
}

TEST(DebugPrintTest, DebugDepthIndentStringFromMultiEx)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--cb_explore_adf"));

  VW::multi_ex examples;
  examples.push_back(VW::read_example(*vw, "shared |User user=Tom"));
  examples.push_back(VW::read_example(*vw, "|Action a=1"));
  VW::setup_examples(*vw, examples);

  examples[0]->debug_current_reduction_depth = 3;
  std::string result = VW::debug::debug_depth_indent_string(examples);

  EXPECT_EQ(result, "    - ");

  vw->finish_example(examples);
}
