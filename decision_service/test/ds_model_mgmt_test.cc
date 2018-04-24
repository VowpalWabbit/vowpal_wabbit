#define BOOST_TEST_MODULE ds_model_mgmt

#include <boost/test/unit_test.hpp>

int add(int i, int j)
{
  return i + j;
}

BOOST_AUTO_TEST_CASE(simpleUsage)
{
  BOOST_CHECK(add(2, 2) == 4);
}
