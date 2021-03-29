#ifndef STATIC_LINK_VW
#  define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "version.h"
#include "vw_versions.h"
#include "global_data.h"

using namespace VW;

BOOST_AUTO_TEST_CASE(verify_vw_versions)
{
  // check default vw version value
  vw dummy_vw;
  BOOST_CHECK(dummy_vw.model_file_ver == EMPTY_VERSION_FILE);
  BOOST_CHECK(dummy_vw.model_file_ver < VERSION_FILE_WITH_CB_ADF_SAVE);

  version_struct temp;

  temp.from_string(VERSION_FILE_WITH_RANK_IN_HEADER);
  BOOST_CHECK(temp < VERSION_FILE_WITH_INTERACTIONS);

  temp.from_string(VERSION_FILE_WITH_CB_ADF_SAVE);
  BOOST_CHECK(temp < VERSION_FILE_WITH_CCB_MULTI_SLOTS_SEEN_FLAG);
}