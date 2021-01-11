#ifndef STATIC_LINK_VW
#  define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "version.h"
#include "vw_versions.h"

using namespace VW;

BOOST_AUTO_TEST_CASE(verify_vw_versions)
{
  version_struct temp;
  temp.from_string(VERSION_FILE_WITH_RANK_IN_HEADER);

  BOOST_CHECK_EQUAL(true, temp < VERSION_FILE_WITH_INTERACTIONS);
}