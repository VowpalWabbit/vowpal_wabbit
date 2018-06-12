#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif

#include <boost/test/unit_test.hpp>
#include <cpprest/json.h>
#include "err_constants.h"
#include "context_helper.h"

using namespace reinforcement_learning;
namespace rlutil = reinforcement_learning::utility;

BOOST_AUTO_TEST_CASE(basic_json_test) {
  auto const context = R"({
    "UserAge":15,
    "_multi":[
      {"_text":"elections maine", "Source":"TV"},
      {"Source":"www", "topic":4, "_label":"2:3:.3"}
    ]
  })";
  size_t count = 0;
  const auto scode = rlutil::get_action_count(count, context);
  BOOST_CHECK_EQUAL(scode, error_code::success);
  BOOST_CHECK_EQUAL(count, 2);
}

BOOST_AUTO_TEST_CASE(json_no_multi) {
  auto context = R"({
    "UserAge":15,
    "ulti":[
      {"_text":"elections maine", "Source":"TV"},
      {"Source":"www", "topic":4, "_label":"2:3:.3"}
    ]
  })";
  size_t count = 0;
  auto scode = rlutil::get_action_count(count, context);
  BOOST_CHECK_EQUAL(scode, error_code::json_no_actions_found);

  context = R"({"UserAge":15})";
  scode = rlutil::get_action_count(count, context);
  BOOST_CHECK_EQUAL(scode, error_code::json_no_actions_found);
}


BOOST_AUTO_TEST_CASE(json_malformed) {
  const auto context = R"({"UserAgeq09898u)(**&^(*&^*^* })";
  size_t count = 0;
  const auto scode = rlutil::get_action_count(count, context);
  BOOST_CHECK_EQUAL(scode, error_code::json_parse_error);
}