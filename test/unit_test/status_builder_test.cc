// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#include <boost/test/unit_test.hpp>

#include "vw/core/api_status.h"
#include "vw/core/error_constants.h"

namespace err = VW::experimental::error_code;
using api_status = VW::experimental::api_status;

int testfn()
{
  api_status s;
  RETURN_ERROR_LS(&s, not_implemented) << "Error msg: " << 5;
}

BOOST_AUTO_TEST_CASE(status_builder_usage)
{
  const auto scode = testfn();
  BOOST_CHECK_EQUAL(scode, err::not_implemented);
}