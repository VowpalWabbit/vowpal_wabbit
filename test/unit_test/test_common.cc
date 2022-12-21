// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "test_common.h"

bool is_invoked_with(const std::string& arg)
{
  for (size_t i = 0; i < boost::unit_test::framework::master_test_suite().argc; i++)
  {
    if (VW::string_view(boost::unit_test::framework::master_test_suite().argv[i]).find(arg) != std::string::npos)
    {
      return true;
    }
  }
  return false;
}