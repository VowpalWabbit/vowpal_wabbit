#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif

#include <boost/test/unit_test.hpp>
#include "api_status.h"

namespace err = reinforcement_learning::error_code;

int testfn() {
  reinforcement_learning::api_status s;
  RETURN_ERROR_LS(&s, create_fn_exception) << "Error msg: " << 5;
}

BOOST_AUTO_TEST_CASE(status_builder_usage) {
  const auto scode = testfn();
  BOOST_CHECK_EQUAL(scode, err::create_fn_exception);
}
