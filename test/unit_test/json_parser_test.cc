#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "test_common.h"

#include <vector>
#include "conditional_contextual_bandit.h"
#include "parse_example_json.h"

v_array<example*> parse_json(vw& all, std::string line)
{
  v_array<example*> examples = v_init<example*>();
  examples.push_back(&VW::get_unused_example(&all));
  VW::read_line_json<true>(
      all, examples, (char*)line.c_str(), (VW::example_factory_t)&VW::get_unused_example, (void*)&all);

  return examples;
}

BOOST_AUTO_TEST_CASE(parse_json_cb) {
  auto vw = VW::initialize("--cb --json --no_stdin", nullptr, false, nullptr, nullptr);

  std::string example = R"({
  "s_": "1",
  "s_": "2",
  "_labelIndex": 0,
  "_label_Action": 0,
  "_label_Cost": 1,
  "_label_Probability": 0.5,
  "_multi": [
    {
      "a_": "1",
      "b_": "1",
      "c_": "1"
    },
    {
      "a_": "2",
      "b_": "2",
      "c_": "2"
    },
    {
      "a_": "3",
      "b_": "3",
      "c_": "3"
    }
  ]
})";

  auto examples = parse_json(*vw, "{}");

}




BOOST_AUTO_TEST_CASE(parse_json_simple) {
  auto vw = VW::initialize("--json --no_stdin", nullptr, false, nullptr, nullptr);

  std::string example = R"({
  "_label": 1,
  "features": {
    "13": 3.9656971e-02,
    "24303": 2.2660980e-01,
    "const": 0.01
  }
})";

  auto examples = parse_json(*vw, "{}");

}

BOOST_AUTO_TEST_CASE(parse_json_ccb) {
  auto vw = VW::initialize("--cb --json --no_stdin", nullptr, false, nullptr, nullptr);

  std::string example = R"({
  "s_": "1",
  "s_": "2",
  "_labelIndex": 0,
  "_label_Action": 0,
  "_label_Cost": 1,
  "_label_Probability": 0.5,
  "_multi": [
    {
      "a_": "1",
      "b_": "1",
      "c_": "1"
    },
    {
      "a_": "2",
      "b_": "2",
      "c_": "2"
    },
    {
      "a_": "3",
      "b_": "3",
      "c_": "3"
    }
  ]
})";

  auto examples = parse_json(*vw, "{}");

}


