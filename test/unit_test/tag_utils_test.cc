#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "test_common.h"
#include "vw.h"
#include "example.h"

#include <vector>
#include "tag_utils.h"
#include "test_common.h"

BOOST_AUTO_TEST_CASE(tag_with_seed__seed_extraction)
{
  auto vw = VW::initialize("--json --no_stdin --quiet", nullptr, false, nullptr, nullptr);
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

  auto seed = VW::extract_random_seed(*example);
  BOOST_CHECK(seed);
  BOOST_CHECK_EQUAL(expected, *seed);

  VW::finish_example(*vw, examples);
}

BOOST_AUTO_TEST_CASE(tag_without_seed__seed_extraction)
{
  auto vw = VW::initialize("--json --no_stdin --quiet", nullptr, false, nullptr, nullptr);
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

  VW::string_view expected{"test_seed"};

  auto seed = VW::extract_random_seed(*example);
  BOOST_CHECK_EQUAL(false, static_cast<bool>(seed));

  VW::finish_example(*vw, examples);
}

BOOST_AUTO_TEST_CASE(no_tag__seed_extraction)
{
  auto vw = VW::initialize("--json --no_stdin --quiet", nullptr, false, nullptr, nullptr);
  std::string json = R"(
  {
    "_label": 1,
    "features": {
      "f":0
    }
  })";

  auto examples = parse_json(*vw, json);
  auto example = examples[0];

  VW::string_view expected{"test_seed"};

  auto seed = VW::extract_random_seed(*example);
  BOOST_CHECK_EQUAL(false, static_cast<bool>(seed));

  VW::finish_example(*vw, examples);
}


