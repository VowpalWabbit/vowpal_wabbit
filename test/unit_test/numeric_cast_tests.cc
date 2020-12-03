#ifndef STATIC_LINK_VW
#  define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "test_common.h"

#include "numeric_casts.h"
#include "vw_exception.h"

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
}