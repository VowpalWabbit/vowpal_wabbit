// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/parse_args.h"

#include <gtest/gtest.h>

using namespace VW::details;
using namespace ::testing;

#include <string>

TEST(TestUtils, ExtractIgnoredFeatureTest)
{
  auto namespace_feature = extract_ignored_feature("namespace|feature");
  auto ns = std::get<0>(namespace_feature);
  auto feature_name = std::get<1>(namespace_feature);
  std::string expected_ns = "namespace";
  std::string expected_feature = "feature";
  EXPECT_EQ(expected_ns, ns);
  EXPECT_EQ(expected_feature, feature_name);

  auto namespace_feature2 = extract_ignored_feature("");
  auto ns2 = std::get<0>(namespace_feature2);
  auto feature_name2 = std::get<1>(namespace_feature2);
  std::string expected_ns2;
  std::string expected_feature2;
  EXPECT_EQ(expected_ns2, ns2);
  EXPECT_EQ(expected_feature2, feature_name2);

  auto namespace_feature3 = extract_ignored_feature("|feature3");
  auto ns3 = std::get<0>(namespace_feature3);
  auto feature_name3 = std::get<1>(namespace_feature3);
  std::string expected_ns3 = " ";
  std::string expected_feature3 = "feature3";
  EXPECT_EQ(expected_ns3, ns3);
  EXPECT_EQ(expected_feature3, feature_name3);
}