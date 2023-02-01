// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/vw.h"
#include "vw/test_common/test_common.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(CbExploreAdf, ShouldThrowEmptyMultiExample)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--quiet"));
  VW::multi_ex example_collection;

  // An empty example collection is invalid and so should throw.
  EXPECT_THROW(vw->learn(example_collection), VW::vw_exception);
}
