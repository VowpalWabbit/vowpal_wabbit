// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/example.h"

#include "vw/config/options_cli.h"
#include "vw/core/shared_data.h"
#include "vw/core/vw.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

TEST(flat_example_tests, sans_interaction_test)
{
  auto* vw = VW::initialize("--quiet --noconstant");

  auto* ex = VW::read_example(*vw, "1 |x a:2 |y b:3");
  auto& flat = *VW::flatten_sort_example(*vw, ex);

  EXPECT_THAT(flat.fs.values, testing::UnorderedElementsAre(2, 3));
  EXPECT_EQ(flat.total_sum_feat_sq, 13);

  VW::finish_example(*vw, *ex);
  delete vw;
}

TEST(flat_example_tests, with_interaction_test)
{
  auto* vw = VW::initialize("--interactions xy --quiet --noconstant");

  auto* ex = VW::read_example(*vw, "1 |x a:2 |y b:3");
  auto& flat = *VW::flatten_sort_example(*vw, ex);

  EXPECT_THAT(flat.fs.values, testing::UnorderedElementsAre(2, 3, 6));
  EXPECT_EQ(flat.total_sum_feat_sq, 49);

  VW::finish_example(*vw, *ex);
  delete vw;
}
