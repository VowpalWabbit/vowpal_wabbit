// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "scope_exit.h"

BOOST_AUTO_TEST_CASE(scope_exit_execute_on_scope_end)
{
  int calls = 0;
  {
    auto guard = VW::scope_exit([&calls]() { calls++; });
  }
  BOOST_CHECK_EQUAL(calls, 1);
}

BOOST_AUTO_TEST_CASE(scope_exit_cancel)
{
  int calls = 0;
  {
    auto guard = VW::scope_exit([&calls]() { calls++; });
    guard.cancel();
  }
  BOOST_CHECK_EQUAL(calls, 0);
}

BOOST_AUTO_TEST_CASE(scope_exit_explicit_call)
{
  int calls = 0;
  {
    auto guard = VW::scope_exit([&calls]() { calls++; });
    BOOST_CHECK_EQUAL(calls, 0);

    guard.call();
    BOOST_CHECK_EQUAL(calls, 1);
  }
  BOOST_CHECK_EQUAL(calls, 1);
}