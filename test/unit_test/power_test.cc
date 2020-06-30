#ifndef STATIC_LINK_VW
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "fast_pow10.h"
#include "test_common.h"

BOOST_AUTO_TEST_CASE(pow10_tests)
{
  BOOST_CHECK_CLOSE(VW::fast_pow10(-38), 1e-38, FLOAT_TOL);
  BOOST_CHECK_CLOSE(VW::fast_pow10(-37), 1e-37, FLOAT_TOL);
  BOOST_CHECK_CLOSE(VW::fast_pow10(-10), 1e-10, FLOAT_TOL);
  BOOST_CHECK_CLOSE(VW::fast_pow10(-5), 1e-5, FLOAT_TOL);
  BOOST_CHECK_CLOSE(VW::fast_pow10(0), 1, FLOAT_TOL);
  BOOST_CHECK_CLOSE(VW::fast_pow10(5), 1e5, FLOAT_TOL);
  BOOST_CHECK_CLOSE(VW::fast_pow10(10), 1e10, FLOAT_TOL);
  BOOST_CHECK_CLOSE(VW::fast_pow10(37), 1e37, FLOAT_TOL);
  BOOST_CHECK_CLOSE(VW::fast_pow10(38), 1e38, FLOAT_TOL);
  BOOST_CHECK(std::isnan(VW::fast_pow10(39)));
  BOOST_CHECK(std::isnan(VW::fast_pow10(-39)));
  BOOST_CHECK(std::isnan(VW::fast_pow10(-127)));
  BOOST_CHECK(std::isnan(VW::fast_pow10(127)));

}
