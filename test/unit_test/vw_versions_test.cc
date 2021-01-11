#ifndef STATIC_LINK_VW
#  define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "vw_versions.h"

BOOST_AUTO_TEST_CASE(verify_vw_versions)
{
  // less than boost check
  BOOST_CHECK_LT(2, 4);

  BOOST_CHECK_LT(VERSION_FILE_WITH_RANK_IN_HEADER, VERSION_FILE_WITH_INTERACTIONS);
}