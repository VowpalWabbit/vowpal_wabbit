// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/tag_utils.h"

#include "vw/config/options_cli.h"
#include "vw/core/example.h"
#include "vw/core/memory.h"
#include "vw/core/vw.h"
#include "vw/test_common/test_common.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <vector>

TEST(TagUtils, TagWithSeedSeedExtraction)
{
  auto opts = VW::make_unique<VW::config::options_cli>(
      std::vector<std::string>{"--json", "--chain_hash", "--no_stdin", "--quiet"});
  auto vw = VW::initialize(std::move(opts));
  std::string json = R"(
  {
    "_label": 1,
    "_tag": "seed=test_seed",
    "features": {
      "f":0
    }
  })";

  auto examples = vwtest::parse_json(*vw, json);
  auto example = examples[0];

  VW::string_view expected{"test_seed"};

  VW::string_view seed;

  auto extracted = VW::try_extract_random_seed(*example, seed);
  EXPECT_EQ(true, extracted);
  EXPECT_EQ(expected, seed);
  VW::finish_example(*vw, examples);
}

TEST(TagUtils, TagWithoutSeedSeedExtraction)
{
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--no_stdin", "--quiet"));
  std::string json = R"(
  {
    "_label": 1,
    "_tag": "some tag without seed",
    "features": {
      "f":0
    }
  })";

  auto examples = vwtest::parse_json(*vw, json);
  auto example = examples[0];

  VW::string_view seed;

  auto extracted = VW::try_extract_random_seed(*example, seed);
  EXPECT_EQ(false, extracted);

  VW::finish_example(*vw, examples);
}

TEST(TagUtils, NoTagSeedExtraction)
{
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--no_stdin", "--quiet"));
  std::string json = R"(
  {
    "_label": 1,
    "features": {
      "f":0
    }
  })";

  auto examples = vwtest::parse_json(*vw, json);
  auto example = examples[0];

  VW::string_view seed;

  auto extracted = VW::try_extract_random_seed(*example, seed);
  EXPECT_EQ(false, extracted);

  VW::finish_example(*vw, examples);
}
