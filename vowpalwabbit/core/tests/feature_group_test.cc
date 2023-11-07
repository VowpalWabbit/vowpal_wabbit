// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/feature_group.h"

#include "vw/common/hash.h"
#include "vw/core/scope_exit.h"
#include "vw/core/unique_sort.h"
#include "vw/core/vw.h"
#include "vw/test_common/test_common.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace ::testing;

TEST(FeatureGroup, UniqueFeatureGroupTest)
{
  VW::features fs;
  fs.push_back(1.f, 1);
  fs.push_back(1.f, 2);
  fs.push_back(1.f, 1);
  fs.push_back(1.f, 1);
  fs.push_back(1.f, 25);
  fs.start_ns_extent(1);
  fs.push_back(1.f, 3);
  fs.push_back(1.f, 3);
  fs.push_back(1.f, 3);
  fs.push_back(1.f, 5);
  fs.end_ns_extent();
  fs.push_back(1.f, 7);
  fs.start_ns_extent(2);
  fs.push_back(1.f, 13);
  fs.push_back(1.f, 11);
  fs.push_back(1.f, 12);
  fs.end_ns_extent();

  const auto parse_mask = (static_cast<uint64_t>(1) << 18) - 1;
  fs.sort(parse_mask);

  auto fs_copy1 = fs;
  auto fs_copy2 = fs;
  auto fs_copy3 = fs;
  auto fs_copy4 = fs;

  // Cap at 5
  VW::unique_features(fs, 5);
  EXPECT_THAT(fs.indices, ElementsAre(1, 2, 3, 5, 7));
  EXPECT_THAT(fs.namespace_extents, ContainerEq(std::vector<VW::namespace_extent>{{2, 4, 1}}));

  // Uncapped
  VW::unique_features(fs_copy1);
  EXPECT_THAT(fs_copy1.indices, ElementsAre(1, 2, 3, 5, 7, 11, 12, 13, 25));
  EXPECT_THAT(fs_copy1.namespace_extents, ContainerEq(std::vector<VW::namespace_extent>{{2, 4, 1}, {5, 8, 2}}));

  // Special case at max 1
  VW::unique_features(fs_copy2, 1);
  EXPECT_THAT(fs_copy2.indices, ElementsAre(1));
  EXPECT_TRUE(fs_copy2.namespace_extents.empty());

  // Special case for max 0
  VW::unique_features(fs_copy3, 0);
  EXPECT_TRUE(fs_copy3.empty());
  EXPECT_TRUE(fs_copy3.namespace_extents.empty());

  // Explicit negative input that isn't -1
  VW::unique_features(fs_copy4, -10);
  EXPECT_THAT(fs_copy4.indices, ElementsAre(1, 2, 3, 5, 7, 11, 12, 13, 25));
  EXPECT_THAT(fs_copy4.namespace_extents, ContainerEq(std::vector<VW::namespace_extent>{{2, 4, 1}, {5, 8, 2}}));

  // Special case for max 0
  VW::features empty_features;
  VW::unique_features(empty_features, 0);
  EXPECT_TRUE(empty_features.empty());
  EXPECT_TRUE(empty_features.namespace_extents.empty());

  VW::features fs_size_one;
  fs_size_one.push_back(1.f, 1);
  VW::unique_features(fs_size_one);
  EXPECT_THAT(fs_size_one.indices, ElementsAre(1));
  EXPECT_TRUE(fs_size_one.namespace_extents.empty());
}

TEST(FeatureGroup, FlattenThenUnflattenNamespaceExtentsTest)
{
  std::vector<VW::namespace_extent> extents{{0, 1, 1}, {1, 2, 2}};

  auto flat_list = VW::details::flatten_namespace_extents(extents, 2);
  auto unflattened_list = VW::details::unflatten_namespace_extents(flat_list);
  EXPECT_TRUE(unflattened_list == extents);

  extents = {{1, 2, 123}};

  flat_list = VW::details::flatten_namespace_extents(extents, 2);
  unflattened_list = VW::details::unflatten_namespace_extents(flat_list);
  EXPECT_TRUE(unflattened_list == extents);

  extents = {{1, 2, 1}, {5, 8, 2}, {8, 12, 3}, {13, 14, 4}};
  flat_list = VW::details::flatten_namespace_extents(extents, 18);
  unflattened_list = VW::details::unflatten_namespace_extents(flat_list);
  EXPECT_TRUE(unflattened_list == extents);
}

TEST(FeatureGroup, SortFeatureGroupTest)
{
  VW::features fs;
  fs.push_back(1.f, 1);
  fs.push_back(1.f, 25);
  fs.start_ns_extent(1);
  fs.push_back(1.f, 3);
  fs.push_back(1.f, 5);
  fs.end_ns_extent();
  fs.push_back(1.f, 7);
  fs.start_ns_extent(2);
  fs.push_back(1.f, 13);
  fs.push_back(1.f, 11);
  fs.push_back(1.f, 12);
  fs.end_ns_extent();

  const auto parse_mask = (static_cast<uint64_t>(1) << 18) - 1;
  fs.sort(parse_mask);

  EXPECT_THAT(fs.indices, ElementsAre(1, 3, 5, 7, 11, 12, 13, 25));
  EXPECT_THAT(fs.namespace_extents, ContainerEq(std::vector<VW::namespace_extent>{{1, 3, 1}, {4, 7, 2}}));
}

TEST(FeatureGroup, IterateExtentsTest)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* ex = VW::read_example(*vw, "|user_info a b c |user_geo a b c d |other a b c d e |user_info a b");
  auto cleanup = VW::scope_exit([&]() { VW::finish_example(*vw, *ex); });

  {
    auto begin = ex->feature_space['u'].hash_extents_begin(VW::hash_space(*vw, "user_info"));
    const auto end = ex->feature_space['u'].hash_extents_end(VW::hash_space(*vw, "user_info"));
    EXPECT_EQ(std::distance(begin, end), 2);
    EXPECT_EQ(std::distance((*begin).first, (*begin).second), 3);
    ++begin;
    EXPECT_EQ(std::distance((*begin).first, (*begin).second), 2);
  }

  // seek first
  {
    auto begin = ex->feature_space['u'].hash_extents_begin(VW::hash_space(*vw, "user_geo"));
    const auto end = ex->feature_space['u'].hash_extents_end(VW::hash_space(*vw, "user_geo"));
    EXPECT_EQ(std::distance(begin, end), 1);
    EXPECT_EQ(std::distance((*begin).first, (*begin).second), 4);
  }

  // Different first char
  {
    auto begin = ex->feature_space['o'].hash_extents_begin(VW::hash_space(*vw, "other"));
    const auto end = ex->feature_space['o'].hash_extents_end(VW::hash_space(*vw, "other"));
    EXPECT_EQ(std::distance(begin, end), 1);
    EXPECT_EQ(std::distance((*begin).first, (*begin).second), 5);
  }
}
