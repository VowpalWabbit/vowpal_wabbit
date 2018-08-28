#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif

#include "str_util.h"
#include <boost/test/unit_test.hpp>

using namespace reinforcement_learning::utility;
using namespace std;

BOOST_AUTO_TEST_CASE(str_functions) {
  string tval = " TRUE ";
  str_util::to_lower(tval);
  BOOST_CHECK_EQUAL(tval, " true ");
  str_util::ltrim(tval);
  BOOST_CHECK_EQUAL(tval, "true ");
  str_util::rtrim(tval);
  BOOST_CHECK_EQUAL(tval, "true");

  tval = "  FALSE   ";
  str_util::trim(tval);
  BOOST_CHECK_EQUAL(tval, "FALSE");
}
