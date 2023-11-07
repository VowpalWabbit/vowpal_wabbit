// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/cb.h"
#include "vw/core/cost_sensitive.h"
#include "vw/core/reductions/conditional_contextual_bandit.h"
#include "vw/core/vw.h"
#include "vw/test_common/test_common.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(ExampleHeader, IsExampleHeaderCb)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--quiet"));
  VW::multi_ex examples;
  examples.push_back(VW::read_example(*vw, "shared | s_1 s_2"));
  examples.push_back(VW::read_example(*vw, "0:1.0:0.5 | a:1 b:1 c:1"));
  examples.push_back(VW::read_example(*vw, "| a:0.5 b:2 c:1"));

  EXPECT_EQ(VW::ec_is_example_header_cb(*examples[0]), true);
  EXPECT_EQ(VW::is_cs_example_header(*examples[0]), false);

  EXPECT_EQ(VW::ec_is_example_header_cb(*examples[1]), false);
  EXPECT_EQ(VW::is_cs_example_header(*examples[1]), false);

  EXPECT_EQ(VW::ec_is_example_header_cb(*examples[2]), false);
  EXPECT_EQ(VW::is_cs_example_header(*examples[2]), false);
  VW::finish_example(*vw, examples);
}

TEST(ExampleHeader, IsExampleHeaderCcb)
{
  auto vw = VW::initialize(vwtest::make_args("--ccb_explore_adf", "--quiet"));
  VW::multi_ex examples;
  examples.push_back(VW::read_example(*vw, "ccb shared |User f"));
  examples.push_back(VW::read_example(*vw, "ccb action |Action f"));

  EXPECT_EQ(VW::reductions::ccb::ec_is_example_header(*examples[0]), true);
  EXPECT_EQ(VW::reductions::ccb::ec_is_example_header(*examples[1]), false);
  VW::finish_example(*vw, examples);
}

TEST(ExampleHeader, IsExampleHeaderCsoaa)
{
  auto vw = VW::initialize(vwtest::make_args("--csoaa_ldf=multiline", "--quiet"));
  VW::multi_ex examples;
  examples.push_back(VW::read_example(*vw, "shared | a_2 b_2 c_2"));
  examples.push_back(VW::read_example(*vw, "3:2.0 | a_3 b_3 c_3"));

  EXPECT_EQ(VW::ec_is_example_header_cb(*examples[0]), false);
  EXPECT_EQ(VW::is_cs_example_header(*examples[0]), true);

  EXPECT_EQ(VW::ec_is_example_header_cb(*examples[1]), false);
  EXPECT_EQ(VW::is_cs_example_header(*examples[1]), false);
  VW::finish_example(*vw, examples);
}
