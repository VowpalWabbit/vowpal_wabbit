// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/common/text_utils.h"
#include "vw/core/text_utils.h"

#include <gtest/gtest.h>

using namespace ::testing;

TEST(test_utils_tests, extract_ignored_feature_test)
{
  auto namespace_feature = VW::extract_ignored_feature("namespace|feature");
  auto ns = std::get<0>(namespace_feature);
  auto feature_name = std::get<1>(namespace_feature);
  auto expected_ns = "namespace";
  auto expected_feature = "feature";
  EXPECT_EQ(expected_ns, ns);
  EXPECT_EQ(expected_feature, feature_name);
  
  auto namespace_feature2 = VW::extract_ignored_feature("");
  auto ns2 = std::get<0>(namespace_feature2);
  auto feature_name2 = std::get<1>(namespace_feature2);
  std::string expected_ns2;
  std::string expected_feature2;
  EXPECT_EQ(expected_ns2, ns2);
  EXPECT_EQ(expected_feature2, feature_name2);
}