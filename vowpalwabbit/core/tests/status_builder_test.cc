// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/api_status.h"
#include "vw/core/error_constants.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace err = VW::experimental::error_code;
using api_status = VW::experimental::api_status;

int testfn()
{
  api_status s;
  RETURN_ERROR_LS(&s, not_implemented) << "Error msg: " << 5;
}

TEST(StatusBuilder, Usage)
{
  const auto scode = testfn();
  EXPECT_EQ(scode, err::not_implemented);
}