// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <boost/test/unit_test.hpp>

#include "vw_exception.h"

#include <cerrno>

BOOST_AUTO_TEST_CASE(check_strerr_can_retrieve_error_message) {
  // EIO is just a randomly chosen error to test with.
  auto message = VW::strerror_to_string(EIO);
  // If the error message contains unknown, then the error retrieval failed and it returned a generic message.
  BOOST_CHECK_EQUAL(message.find("unknown"), std::string::npos);
}