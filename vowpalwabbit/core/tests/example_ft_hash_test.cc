// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/vw.h"
#include "vw/test_common/test_common.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(ExampleFtHash, CheckLargeStringOneSmallDiff)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  uint64_t hash_1;
  uint64_t hash_2;

  {
    auto* ex = VW::read_example(*vw,
        "|Action politics:3 sports:93 music:18 food:165 math:141 science:33 computers:93 literature:84 "
        "social_media:153 |f dancing:105 news:81 celebrities:0 coffee:45 baking:138 ai:9 taxes:15 technology:120 "
        "pandemic:135 |m world_war_i:93 world_war_ii:147 religion:120 vaccination:141 the_queen:3 immigration:51 "
        "economics:117 fashion:90 NYC:51 LA:45 elections:81 brunch:18 space_exploration:45 climate_change:129 dogs:141 "
        "cats:45 inflation:138 |o sourdough:102 vegan:117 vacation:114 asteroid:147 eyewear:147 stationary:21 "
        "movies:36 "
        "oscars:12 acupuncture:6 hiking:99 swimming:156 barrys:111");
    hash_1 = ex->get_or_calculate_order_independent_feature_space_hash();

    VW::finish_example(*vw, *ex);
  }
  {
    auto* ex = VW::read_example(*vw,
        "|Action politics:3 sports:93 music:18 food:165 math:141 science:33 computers:93 literature:84 "
        "social_media:153 |f dancing:105 news:81 celebrities:0 coffee:45 baking:138 ai:9 taxes:15 technology:120 "
        "pandemic:135 |m world_war_i:93 world_war_ii:147 religion:120 vaccination:141 the_queen:3 immigration:51 "
        "economics:117 fashion:90 NYC:51 LA:45 elections:81 brunch:18 space_exploration:45 climate_change:129 dogs:141 "
        "cats:45 inflation:138 |o sourdough:102 vegan:117 vacation:114 asteroid:147 eyewear:147 stationary:21 "
        "movies:36 "
        "oscars:12 acupuncture:6 hiking:99 swimming:156 barrys:112");
    hash_2 = ex->get_or_calculate_order_independent_feature_space_hash();

    VW::finish_example(*vw, *ex);
  }

  EXPECT_NE(hash_1, hash_2);
  EXPECT_NE(hash_1, 0);
  EXPECT_NE(hash_2, 0);
}

TEST(ExampleFtHash, CheckOrderDoesNotCount)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--chain_hash"));

  std::vector<std::string> bagofwords = {"politics:3", "sports:93", "music:18", "food:165", "math:141", "science:33",
      "computers:93", "literature:84", "social_media:153", "dancing:105", "news:81", "celebrities:0", "coffee:45",
      "baking:138", "ai:9", "taxes:15", "technology:120", "pandemic:135", "world_war_i:93", "world_war_ii:147",
      "religion:120", "vaccination:141", "the_queen:3", "immigration:51", "economics:117", "fashion:90", "NYC:51",
      "LA:45", "elections:81", "brunch:18", "space_exploration:45", "climate_change:129", "dogs:141", "cats:45",
      "inflation:138", "sourdough:102", "vegan:117", "vacation:114", "asteroid:147", "eyewear:147", "stationary:21",
      "movies:36", "oscars:12", "acupuncture:6", "hiking:99", "swimming:156", "barrys:111"};

  uint64_t hash_1;
  uint64_t hash_2;

  std::string s1;
  std::string s2;

  {
    std::set<size_t> picked;
    srand(0);
    std::stringstream ss;
    ss << "|";
    while (picked.size() != bagofwords.size())
    {
      size_t ind = rand() % bagofwords.size();
      if (picked.find(ind) == picked.end())
      {
        picked.insert(ind);
        ss << " " << bagofwords[ind];
      }
    }

    s1 = ss.str();

    auto* ex = VW::read_example(*vw, s1);
    hash_1 = ex->get_or_calculate_order_independent_feature_space_hash();

    VW::finish_example(*vw, *ex);
  }
  {
    std::set<size_t> picked;
    srand(10);
    std::stringstream ss;
    ss << "|";
    while (picked.size() != bagofwords.size())
    {
      size_t ind = rand() % bagofwords.size();
      if (picked.find(ind) == picked.end())
      {
        picked.insert(ind);
        ss << " " << bagofwords[ind];
      }
    }

    s2 = ss.str();

    auto* ex = VW::read_example(*vw, s2);
    hash_2 = ex->get_or_calculate_order_independent_feature_space_hash();

    VW::finish_example(*vw, *ex);
  }

  EXPECT_NE(s1, s2);
  EXPECT_EQ(hash_1, hash_2);
  EXPECT_NE(hash_1, 0);
}

TEST(ExampleFtHash, CheckDefaultValueMissingSameHash)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  uint64_t hash_1;
  uint64_t hash_2;

  {
    auto* ex = VW::read_example(*vw, "| a");
    hash_1 = ex->get_or_calculate_order_independent_feature_space_hash();

    VW::finish_example(*vw, *ex);
  }
  {
    auto* ex = VW::read_example(*vw, "| a:1");
    hash_2 = ex->get_or_calculate_order_independent_feature_space_hash();

    VW::finish_example(*vw, *ex);
  }

  EXPECT_EQ(hash_1, hash_2);
  EXPECT_NE(hash_1, 0);
}

TEST(ExampleFtHash, CheckZeroHash)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--noconstant"));
  {
    auto* ex = VW::read_example(*vw, "| a:0");
    EXPECT_EQ(ex->get_or_calculate_order_independent_feature_space_hash(), 0);

    VW::finish_example(*vw, *ex);
  }

  {
    auto* ex = VW::read_example(*vw, "shared |");
    EXPECT_EQ(ex->get_or_calculate_order_independent_feature_space_hash(), 0);

    VW::finish_example(*vw, *ex);
  }
}

TEST(ExampleFtHash, SmallDiffs)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--noconstant"));
  uint64_t hash_1;
  uint64_t hash_2;

  {
    auto* ex = VW::read_example(*vw, "| a1:0.0000001 a2");
    hash_1 = ex->get_or_calculate_order_independent_feature_space_hash();

    VW::finish_example(*vw, *ex);
  }
  {
    auto* ex = VW::read_example(*vw, "| a1:0.000001 a2");
    hash_2 = ex->get_or_calculate_order_independent_feature_space_hash();

    VW::finish_example(*vw, *ex);
  }

  EXPECT_NE(hash_1, hash_2);
  EXPECT_NE(hash_1, 0);
  EXPECT_NE(hash_2, 0);
}