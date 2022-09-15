// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/object_pool.h"

#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>
#include <string>
#include <vector>

struct obj
{
  int i;
};

// If there are memory issues valgrind will catch this test.
BOOST_AUTO_TEST_CASE(object_pool_test)
{
  VW::object_pool<obj> pool{};

  // Returned as unique_ptr
  auto o1 = pool.get_object();
  // Deleted by unique_ptr
  auto o2 = pool.get_object();
  // Returned as raw ptr
  auto* o3 = pool.get_object().release();

  pool.return_object(std::move(o1));
  pool.return_object(std::move(o3));
}
