// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/io/errno_handling.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cerrno>

TEST(ErrNo, CheckStrerrCanRetrieveErrorMessage)
{
  // EIO is just a randomly chosen error to test with.
  auto message = VW::io::strerror_to_string(EIO);
  // If the error message contains unknown, then the error retrieval failed and it returned a generic message.
  EXPECT_EQ(message.find("unknown"), std::string::npos);
}