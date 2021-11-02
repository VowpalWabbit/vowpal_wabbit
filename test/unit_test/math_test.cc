// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "test_common.h"

#include "vw_math.h"

BOOST_AUTO_TEST_CASE(math_factorial_tests)
{
  BOOST_CHECK_EQUAL(VW::math::factorial(0), 1);
  BOOST_CHECK_EQUAL(VW::math::factorial(1), 1);
  BOOST_CHECK_EQUAL(VW::math::factorial(2), 2);
  BOOST_CHECK_EQUAL(VW::math::factorial(3), 6);
  BOOST_CHECK_EQUAL(VW::math::factorial(4), 24);
  BOOST_CHECK_EQUAL(VW::math::factorial(5), 120);
  BOOST_CHECK_EQUAL(VW::math::factorial(10), 3628800);
}

BOOST_AUTO_TEST_CASE(math_number_of_combinations_with_repetition_tests)
{
  BOOST_CHECK_EQUAL(VW::math::number_of_combinations_with_repetition(1, 1), 1);
  BOOST_CHECK_EQUAL(VW::math::number_of_combinations_with_repetition(5, 1), 5);
  BOOST_CHECK_EQUAL(VW::math::number_of_combinations_with_repetition(1, 5), 1);
  BOOST_CHECK_EQUAL(VW::math::number_of_combinations_with_repetition(5, 5), 126);
  BOOST_CHECK_EQUAL(VW::math::number_of_combinations_with_repetition(10, 2), 55);
}

BOOST_AUTO_TEST_CASE(math_number_of_permutations_with_repetition_tests)
{
  BOOST_CHECK_EQUAL(VW::math::number_of_permutations_with_repetition(1, 1), 1);
  BOOST_CHECK_EQUAL(VW::math::number_of_permutations_with_repetition(5, 1), 5);
  BOOST_CHECK_EQUAL(VW::math::number_of_permutations_with_repetition(1, 5), 1);
  BOOST_CHECK_EQUAL(VW::math::number_of_permutations_with_repetition(5, 5), 3125);
  BOOST_CHECK_EQUAL(VW::math::number_of_permutations_with_repetition(10, 2), 100);
}

BOOST_AUTO_TEST_CASE(math_sign_tests)
{
  BOOST_CHECK_CLOSE(VW::math::sign(1.f), 1.f, FLOAT_TOL);
  BOOST_CHECK_CLOSE(VW::math::sign(-1.f), -1.f, FLOAT_TOL);
  BOOST_CHECK_CLOSE(VW::math::sign(0.f), -1.f, FLOAT_TOL);
  BOOST_CHECK_CLOSE(VW::math::sign(0.1f), 1.f, FLOAT_TOL);
  BOOST_CHECK_CLOSE(VW::math::sign(1000.f), 1.f, FLOAT_TOL);
  BOOST_CHECK_CLOSE(VW::math::sign(-999.f), -1.f, FLOAT_TOL);
}

BOOST_AUTO_TEST_CASE(math_choose_tests)
{
  BOOST_CHECK_EQUAL(VW::math::choose(1, 1), 1);
  BOOST_CHECK_EQUAL(VW::math::choose(4, 2), 6);
  BOOST_CHECK_EQUAL(VW::math::choose(2, 4), 0);
  BOOST_CHECK_EQUAL(VW::math::choose(0, 0), 1);
  BOOST_CHECK_EQUAL(VW::math::choose(0, 1), 0);
  BOOST_CHECK_EQUAL(VW::math::choose(1, 0), 1);
}