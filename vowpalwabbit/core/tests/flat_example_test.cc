// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/config/options_cli.h"
#include "vw/core/example.h"
#include "vw/core/shared_data.h"
#include "vw/core/vw.h"
#include "vw/test_common/test_common.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(FlatExample, SansInteractionTest)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--noconstant"));

  auto* ex = VW::read_example(*vw, "1 |x a:2 |y b:3");
  VW::features fs;
  VW::flatten_features(*vw, *ex, fs);

  EXPECT_THAT(fs.values, testing::UnorderedElementsAre(2, 3));
  EXPECT_EQ(fs.sum_feat_sq, 13);

  VW::finish_example(*vw, *ex);
}

TEST(FlatExample, WithInteractionTest)
{
  auto vw = VW::initialize(vwtest::make_args("--interactions", "xy", "--quiet", "--noconstant"));

  auto* ex = VW::read_example(*vw, "1 |x a:2 |y b:3");
  VW::features fs;
  VW::flatten_features(*vw, *ex, fs);

  EXPECT_THAT(fs.values, testing::UnorderedElementsAre(2, 3, 6));
  EXPECT_EQ(fs.sum_feat_sq, 49);

  VW::finish_example(*vw, *ex);
}

TEST(FlatExample, EmptyExampleTest)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--noconstant"));

  auto* ex = VW::read_example(*vw, "1 |x a:0");
  VW::features fs;
  VW::flatten_features(*vw, *ex, fs);

  EXPECT_TRUE(fs.empty());

  VW::finish_example(*vw, *ex);
}
