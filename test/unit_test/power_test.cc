#ifndef STATIC_LINK_VW
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "fast_pow10.h"
#include "test_common.h"

bool are_same(float a, float b) { return std::abs(a - b) < std::numeric_limits<float>::epsilon(); }

BOOST_AUTO_TEST_CASE(pow10_tests)
{
  // In reality most of these tests would all evaluate to the same. The reason is that the epsilon is usually around 1.2e-7
  const float base = 10;
  BOOST_CHECK(are_same(VW::fast_pow10(-127), std::pow(base, -127)));
  BOOST_CHECK(are_same(VW::fast_pow10(-46), std::pow(base, -46)));
  BOOST_CHECK(are_same(VW::fast_pow10(-45), std::pow(base, -45)));
  BOOST_CHECK(are_same(VW::fast_pow10(-44), std::pow(base, -44)));
  BOOST_CHECK(are_same(VW::fast_pow10(-40), std::pow(base, -40)));
  BOOST_CHECK(are_same(VW::fast_pow10(-38), std::pow(base, -38)));
  BOOST_CHECK(are_same(VW::fast_pow10(-37), std::pow(base, -37)));
  BOOST_CHECK(are_same(VW::fast_pow10(-10), std::pow(base, -10)));
  BOOST_CHECK(are_same(VW::fast_pow10(-5), std::pow(base, -5)));
  BOOST_CHECK_CLOSE(VW::fast_pow10(0), 1, FLOAT_TOL);
  BOOST_CHECK_CLOSE(VW::fast_pow10(5), 1e5, FLOAT_TOL);
  BOOST_CHECK_CLOSE(VW::fast_pow10(10), 1e10, FLOAT_TOL);
  BOOST_CHECK_CLOSE(VW::fast_pow10(37), 1e37, FLOAT_TOL);
  BOOST_CHECK_CLOSE(VW::fast_pow10(38), 1e38, FLOAT_TOL);
  BOOST_CHECK(std::isinf(VW::fast_pow10(39)));
  BOOST_CHECK(std::isinf(VW::fast_pow10(127)));
}
