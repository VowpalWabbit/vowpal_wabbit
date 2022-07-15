// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "test_common.h"
#include "vw/common/vw_exception.h"
#include "vw/core/numeric_casts.h"

#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(numeric_cast_tests)
{
  // Correct negative
  BOOST_CHECK_EQUAL(VW::cast_to_smaller_type<int8_t>(static_cast<int32_t>(-7)), -7);
  // Correct positive
  BOOST_CHECK_EQUAL(VW::cast_to_smaller_type<int8_t>(static_cast<int32_t>(4)), 4);
  // Too small
  BOOST_CHECK_THROW(VW::cast_to_smaller_type<int8_t>(static_cast<int32_t>(-500)), VW::vw_exception);
  // Too large
  BOOST_CHECK_THROW(VW::cast_to_smaller_type<int8_t>(static_cast<int32_t>(50000)), VW::vw_exception);

  // Negative
  BOOST_CHECK_THROW(VW::cast_signed_to_unsigned<uint32_t>(static_cast<int32_t>(-5)), VW::vw_exception);
  // Correct
  BOOST_CHECK_EQUAL(VW::cast_signed_to_unsigned<uint32_t>(static_cast<int32_t>(10)), 10);
  // Larger unsigned to smaller signed
  BOOST_CHECK_EQUAL(VW::cast_signed_to_unsigned<uint8_t>(static_cast<int32_t>(10)), 10);
}