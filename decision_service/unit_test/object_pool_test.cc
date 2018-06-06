#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif

#include <boost/test/unit_test.hpp>
#include "object_pool.h"

using namespace reinforcement_learning;
using namespace std;

class my_object
{
public:
  int _id;

  my_object(int id) : _id(id)
  { }
};

class my_object_factory
{
public:
  int _count;

  my_object_factory() : _count(0)
  { }

  my_object* operator()()
  {
    return new my_object(_count++);
  }
};

BOOST_AUTO_TEST_CASE(object_pool_nothing)
{
  // pool owns factory
  my_object_factory* factory = new my_object_factory;
  object_pool<my_object, my_object_factory> pool(factory);

  BOOST_CHECK_EQUAL(factory->_count, 0);
}

BOOST_AUTO_TEST_CASE(object_pool_get_same_object)
{
  // pool owns factory
  my_object_factory* factory = new my_object_factory;
  object_pool<my_object, my_object_factory> pool(factory);

  {
    pooled_object_guard<my_object, my_object_factory> guard(pool, pool.get_or_create());
    BOOST_CHECK_EQUAL(guard->_id, 0);
  }

  // let's make sure we get the same object back
  {
    pooled_object_guard<my_object, my_object_factory> guard(pool, pool.get_or_create());
    BOOST_CHECK_EQUAL(guard->_id, 0);
  }

   BOOST_CHECK_EQUAL(factory->_count, 1);
}

BOOST_AUTO_TEST_CASE(object_pool_get_2_objects)
{
  // pool owns factory
  my_object_factory* factory = new my_object_factory;
  object_pool<my_object, my_object_factory> pool(factory);

  {
    pooled_object_guard<my_object, my_object_factory> guard1(pool, pool.get_or_create());
    BOOST_CHECK_EQUAL(guard1->_id, 0);

    pooled_object_guard<my_object, my_object_factory> guard2(pool, pool.get_or_create());
    BOOST_CHECK_EQUAL(guard2->_id, 1);
  }

  BOOST_CHECK_EQUAL(factory->_count, 2);
}

BOOST_AUTO_TEST_CASE(object_pool_update_factory)
{
  // pool owns factory
  object_pool<my_object, my_object_factory> pool(new my_object_factory);

  pooled_object_guard<my_object, my_object_factory> guard1(pool, pool.get_or_create());
  BOOST_CHECK_EQUAL(guard1->_id, 0);

  // update factory -> this resets the counter
  pool.update_factory(new my_object_factory);

  pooled_object_guard<my_object, my_object_factory> guard2(pool, pool.get_or_create());
  BOOST_CHECK_EQUAL(guard2->_id, 0); // _id is 0 as this is created from the new factory
}
