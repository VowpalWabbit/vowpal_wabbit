// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "io/logger.h"
#include "version.h"
#include "vw_versions.h"
#include "global_data.h"

using namespace VW;

BOOST_AUTO_TEST_CASE(verify_vw_versions)
{
  using namespace VW::version_definitions;

  // check default vw version value
  auto null_logger = VW::io::create_null_logger();
  workspace dummy_vw(null_logger);
  BOOST_CHECK(dummy_vw.model_file_ver == EMPTY_VERSION_FILE);
  BOOST_CHECK(dummy_vw.model_file_ver < VERSION_FILE_WITH_CB_ADF_SAVE);

  BOOST_CHECK(VERSION_FILE_WITH_RANK_IN_HEADER < VERSION_FILE_WITH_INTERACTIONS);
  BOOST_CHECK(VERSION_FILE_WITH_CB_ADF_SAVE < VERSION_FILE_WITH_CCB_MULTI_SLOTS_SEEN_FLAG);
  BOOST_CHECK(VERSION_FILE_WITH_CCB_MULTI_SLOTS_SEEN_FLAG < VERSION_FILE_WITH_CB_TO_CBADF);
}

BOOST_AUTO_TEST_CASE(verify_vw_version_operators)
{
  BOOST_CHECK((VW::version_struct{0, 0, 0} == VW::version_struct{0, 0, 0}));
  BOOST_CHECK((VW::version_struct{1, 1, 1} == VW::version_struct{1, 1, 1}));
  BOOST_CHECK((VW::version_struct{1, 1, 0} != VW::version_struct{1, 1, 1}));
  BOOST_CHECK((VW::version_struct{0, 0, 0} < VW::version_struct{0, 0, 1}));
  BOOST_CHECK((VW::version_struct{0, 0, 1} > VW::version_struct{0, 0, 0}));
  BOOST_CHECK((VW::version_struct{1, 0, 1} > VW::version_struct{0, 5, 0}));
  BOOST_CHECK((VW::version_struct{1, 2, 3} <= VW::version_struct{1, 2, 3}));
  BOOST_CHECK((VW::version_struct{1, 2, 2} <= VW::version_struct{1, 2, 3}));
  BOOST_CHECK((VW::version_struct{1, 2, 3} >= VW::version_struct{1, 2, 3}));
  BOOST_CHECK((VW::version_struct{1, 2, 4} >= VW::version_struct{1, 2, 3}));
  BOOST_CHECK((VW::version_struct{5, 70, 80} < VW::version_struct{11, 1, 1}));
}

BOOST_AUTO_TEST_CASE(verify_vw_version_tostring)
{
  BOOST_CHECK_EQUAL((VW::version_struct{0, 0, 0}.to_string()), "0.0.0");
  BOOST_CHECK_EQUAL((VW::version_struct{1, 1, 1}.to_string()), "1.1.1");
  BOOST_CHECK_EQUAL((VW::version_struct{1, 1, 0}.to_string()), "1.1.0");
  BOOST_CHECK_EQUAL((VW::version_struct{0, 0, 0}.to_string()), "0.0.0");
  BOOST_CHECK_EQUAL((VW::version_struct{0, 0, 1}.to_string()), "0.0.1");
  BOOST_CHECK_EQUAL((VW::version_struct{1, 0, 1}.to_string()), "1.0.1");
  BOOST_CHECK_EQUAL((VW::version_struct{1, 2, 3}.to_string()), "1.2.3");
  BOOST_CHECK_EQUAL((VW::version_struct{1, 2, 2}.to_string()), "1.2.2");
  BOOST_CHECK_EQUAL((VW::version_struct{1, 2, 3}.to_string()), "1.2.3");
  BOOST_CHECK_EQUAL((VW::version_struct{1, 2, 4}.to_string()), "1.2.4");
  BOOST_CHECK_EQUAL((VW::version_struct{5, 70, 80}.to_string()), "5.70.80");
}

BOOST_AUTO_TEST_CASE(verify_vw_version_fromstring)
{
  BOOST_CHECK((VW::version_struct{0, 0, 0} == VW::version_struct{"0.0.0"}));
  BOOST_CHECK((VW::version_struct{1, 1, 1} == VW::version_struct{"1.1.1"}));
  BOOST_CHECK((VW::version_struct{1, 1, 0} == VW::version_struct{"1.1.0"}));
  BOOST_CHECK((VW::version_struct{0, 0, 0} == VW::version_struct{"0.0.0"}));
  BOOST_CHECK((VW::version_struct{0, 0, 1} == VW::version_struct{"0.0.1"}));
  BOOST_CHECK((VW::version_struct{1, 0, 1} == VW::version_struct{"1.0.1"}));
  BOOST_CHECK((VW::version_struct{1, 2, 3} == VW::version_struct{"1.2.3"}));
  BOOST_CHECK((VW::version_struct{1, 2, 2} == VW::version_struct{"1.2.2"}));
  BOOST_CHECK((VW::version_struct{1, 2, 3} == VW::version_struct{"1.2.3"}));
  BOOST_CHECK((VW::version_struct{1, 2, 4} == VW::version_struct{"1.2.4"}));
  BOOST_CHECK((VW::version_struct{5, 70, 80} == VW::version_struct{"5.70.80"}));
}