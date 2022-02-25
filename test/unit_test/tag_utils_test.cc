// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "test_common.h"
#include "memory.h"
#include "vw.h"
#include "example.h"
#include "config/options_cli.h"

#include <vector>
#include "tag_utils.h"
#include "test_common.h"

BOOST_AUTO_TEST_CASE(tag_with_seed__seed_extraction)
{
  auto opts = VW::make_unique<VW::config::options_cli>(std::vector<std::string>{"--json","--chain_hash","--no_stdin","--quiet"});
  auto vw = VW::initialize_experimental(std::move(opts));
  std::string json = R"(
  {
    "_label": 1,
    "_tag": "seed=test_seed",
    "features": {
      "f":0
    }
  })";

  auto examples = parse_json(*vw, json);
  auto example = examples[0];

  VW::string_view expected{"test_seed"};

  VW::string_view seed;

  auto extracted = VW::try_extract_random_seed(*example, seed);
  BOOST_CHECK_EQUAL(true, extracted);
  BOOST_CHECK_EQUAL(expected, seed);
  VW::finish_example(*vw, examples);
}

BOOST_AUTO_TEST_CASE(tag_without_seed__seed_extraction)
{
  auto vw = VW::initialize("--json --chain_hash --no_stdin --quiet", nullptr, false, nullptr, nullptr);
  std::string json = R"(
  {
    "_label": 1,
    "_tag": "some tag without seed",
    "features": {
      "f":0
    }
  })";

  auto examples = parse_json(*vw, json);
  auto example = examples[0];

  VW::string_view seed;

  auto extracted = VW::try_extract_random_seed(*example, seed);
  BOOST_CHECK_EQUAL(false, extracted);

  VW::finish_example(*vw, examples);
  VW::finish(*vw);
}

BOOST_AUTO_TEST_CASE(no_tag__seed_extraction)
{
  auto vw = VW::initialize("--json --chain_hash --no_stdin --quiet", nullptr, false, nullptr, nullptr);
  std::string json = R"(
  {
    "_label": 1,
    "features": {
      "f":0
    }
  })";

  auto examples = parse_json(*vw, json);
  auto example = examples[0];

  VW::string_view seed;

  auto extracted = VW::try_extract_random_seed(*example, seed);
  BOOST_CHECK_EQUAL(false, extracted);

  VW::finish_example(*vw, examples);
  VW::finish(*vw);
}
