// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/vw_versions.h"

#include "vw/core/global_data.h"
#include "vw/core/version.h"
#include "vw/io/logger.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(Version, VerifyVwVersions)
{
  using namespace VW::version_definitions;

  // check default vw version value
  auto null_logger = VW::io::create_null_logger();
  VW::workspace dummy_vw(null_logger);
  EXPECT_TRUE(dummy_vw.model_file_ver == EMPTY_VERSION_FILE);
  EXPECT_TRUE(dummy_vw.model_file_ver < VERSION_FILE_WITH_CB_ADF_SAVE);

  EXPECT_TRUE(VERSION_FILE_WITH_RANK_IN_HEADER < VERSION_FILE_WITH_INTERACTIONS);
  EXPECT_TRUE(VERSION_FILE_WITH_CB_ADF_SAVE < VERSION_FILE_WITH_CCB_MULTI_SLOTS_SEEN_FLAG);
  EXPECT_TRUE(VERSION_FILE_WITH_CCB_MULTI_SLOTS_SEEN_FLAG < VERSION_FILE_WITH_CB_TO_CBADF);
}

TEST(Version, VerifyVwVersionOperators)
{
  EXPECT_TRUE((VW::version_struct{0, 0, 0} == VW::version_struct{0, 0, 0}));
  EXPECT_TRUE((VW::version_struct{1, 1, 1} == VW::version_struct{1, 1, 1}));
  EXPECT_TRUE((VW::version_struct{1, 1, 0} != VW::version_struct{1, 1, 1}));
  EXPECT_TRUE((VW::version_struct{0, 0, 0} < VW::version_struct{0, 0, 1}));
  EXPECT_TRUE((VW::version_struct{0, 0, 1} > VW::version_struct{0, 0, 0}));
  EXPECT_TRUE((VW::version_struct{1, 0, 1} > VW::version_struct{0, 5, 0}));
  EXPECT_TRUE((VW::version_struct{1, 2, 3} <= VW::version_struct{1, 2, 3}));
  EXPECT_TRUE((VW::version_struct{1, 2, 2} <= VW::version_struct{1, 2, 3}));
  EXPECT_TRUE((VW::version_struct{1, 2, 3} >= VW::version_struct{1, 2, 3}));
  EXPECT_TRUE((VW::version_struct{1, 2, 4} >= VW::version_struct{1, 2, 3}));
  EXPECT_TRUE((VW::version_struct{5, 70, 80} < VW::version_struct{11, 1, 1}));
}

TEST(Version, VerifyVwVersionTostring)
{
  EXPECT_EQ((VW::version_struct{0, 0, 0}.to_string()), "0.0.0");
  EXPECT_EQ((VW::version_struct{1, 1, 1}.to_string()), "1.1.1");
  EXPECT_EQ((VW::version_struct{1, 1, 0}.to_string()), "1.1.0");
  EXPECT_EQ((VW::version_struct{0, 0, 0}.to_string()), "0.0.0");
  EXPECT_EQ((VW::version_struct{0, 0, 1}.to_string()), "0.0.1");
  EXPECT_EQ((VW::version_struct{1, 0, 1}.to_string()), "1.0.1");
  EXPECT_EQ((VW::version_struct{1, 2, 3}.to_string()), "1.2.3");
  EXPECT_EQ((VW::version_struct{1, 2, 2}.to_string()), "1.2.2");
  EXPECT_EQ((VW::version_struct{1, 2, 3}.to_string()), "1.2.3");
  EXPECT_EQ((VW::version_struct{1, 2, 4}.to_string()), "1.2.4");
  EXPECT_EQ((VW::version_struct{5, 70, 80}.to_string()), "5.70.80");
}

TEST(Version, VerifyVwVersionFromstring)
{
  EXPECT_TRUE((VW::version_struct{0, 0, 0} == VW::version_struct{"0.0.0"}));
  EXPECT_TRUE((VW::version_struct{1, 1, 1} == VW::version_struct{"1.1.1"}));
  EXPECT_TRUE((VW::version_struct{1, 1, 0} == VW::version_struct{"1.1.0"}));
  EXPECT_TRUE((VW::version_struct{0, 0, 0} == VW::version_struct{"0.0.0"}));
  EXPECT_TRUE((VW::version_struct{0, 0, 1} == VW::version_struct{"0.0.1"}));
  EXPECT_TRUE((VW::version_struct{1, 0, 1} == VW::version_struct{"1.0.1"}));
  EXPECT_TRUE((VW::version_struct{1, 2, 3} == VW::version_struct{"1.2.3"}));
  EXPECT_TRUE((VW::version_struct{1, 2, 2} == VW::version_struct{"1.2.2"}));
  EXPECT_TRUE((VW::version_struct{1, 2, 3} == VW::version_struct{"1.2.3"}));
  EXPECT_TRUE((VW::version_struct{1, 2, 4} == VW::version_struct{"1.2.4"}));
  EXPECT_TRUE((VW::version_struct{5, 70, 80} == VW::version_struct{"5.70.80"}));
}