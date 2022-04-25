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

struct obj_initializer
{
  obj* operator()(obj* o) { return o; }
};

BOOST_AUTO_TEST_CASE(object_pool_test)
{
  {
    VW::object_pool<obj> pool_with_size{50};
    BOOST_CHECK_EQUAL(pool_with_size.size(), 50);
  }

  {
    VW::object_pool<obj, obj_initializer> pool_with_small_chunks{0, obj_initializer{}, 2};
    BOOST_CHECK_EQUAL(pool_with_small_chunks.size(), 0);

    auto o1 = pool_with_small_chunks.get_object();
    BOOST_CHECK_EQUAL(pool_with_small_chunks.size(), 2);

    auto o2 = pool_with_small_chunks.get_object();
    BOOST_CHECK_EQUAL(pool_with_small_chunks.size(), 2);

    auto o3 = pool_with_small_chunks.get_object();
    BOOST_CHECK_EQUAL(pool_with_small_chunks.size(), 4);

    pool_with_small_chunks.return_object(o1);
    pool_with_small_chunks.return_object(o2);
    pool_with_small_chunks.return_object(o3);
  }

  VW::object_pool<obj, obj_initializer> pool{0, obj_initializer{}, 1};
  BOOST_CHECK_EQUAL(pool.size(), 0);
  BOOST_CHECK_EQUAL(pool.empty(), true);

  auto o1 = pool.get_object();
  BOOST_CHECK_EQUAL(pool.size(), 1);
  BOOST_CHECK_EQUAL(pool.empty(), true);

  auto o2 = pool.get_object();
  BOOST_CHECK_EQUAL(pool.size(), 2);
  BOOST_CHECK_EQUAL(pool.empty(), true);

  pool.return_object(o1);
  BOOST_CHECK_EQUAL(pool.size(), 2);
  BOOST_CHECK_EQUAL(pool.empty(), false);

  obj other_obj;
  BOOST_CHECK_EQUAL(pool.is_from_pool(o2), true);
  BOOST_CHECK_EQUAL(pool.is_from_pool(&other_obj), false);

  pool.return_object(o2);
}
