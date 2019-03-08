#ifndef STATIC_LINK_VW
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "object_pool.h"

#include <vector>
#include <string>

struct obj
{
  int i;
};

struct obj_factory
{
  obj* operator()() { return new obj{}; }
};

BOOST_AUTO_TEST_CASE(object_pool_test)
{
  {
    VW::unbounded_object_pool<obj, obj_factory> pool_with_size{50};
    BOOST_CHECK_EQUAL(pool_with_size.size(), 50);
    BOOST_CHECK_EQUAL(pool_with_size.available(), 50);
  }

  VW::unbounded_object_pool<obj, obj_factory> pool;
  BOOST_CHECK_EQUAL(pool.size(), 0);
  BOOST_CHECK_EQUAL(pool.empty(), true);
  BOOST_CHECK_EQUAL(pool.available(), 0);

  auto o1 = pool.get_object();
  BOOST_CHECK_EQUAL(pool.size(), 1);
  BOOST_CHECK_EQUAL(pool.empty(), true);
  BOOST_CHECK_EQUAL(pool.available(), 0);

  auto o2 = pool.get_object();
  BOOST_CHECK_EQUAL(pool.size(), 2);
  BOOST_CHECK_EQUAL(pool.empty(), true);
  BOOST_CHECK_EQUAL(pool.available(), 0);

  pool.return_object(o1);
  BOOST_CHECK_EQUAL(pool.size(), 2);
  BOOST_CHECK_EQUAL(pool.empty(), false);
  BOOST_CHECK_EQUAL(pool.available(), 1);

  auto other_obj = new obj{};
  BOOST_CHECK_EQUAL(pool.is_from_pool(o2), true);
  BOOST_CHECK_EQUAL(pool.is_from_pool(other_obj), false);
}
