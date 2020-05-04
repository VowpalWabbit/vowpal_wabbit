#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>
#include <explore_internal.h>
namespace test = boost::test_tools;

BOOST_AUTO_TEST_CASE(reproduce_max_boundary_issue)
{
  uint64_t seed = 58587211;
  const uint64_t new_random_seed = uniform_hash(&seed, sizeof(seed), seed);
  BOOST_CHECK_EQUAL(new_random_seed, 2244123448);

  float random_draw = exploration::uniform_random_merand48(new_random_seed);
  BOOST_TEST(random_draw == 0.99999f, test::tolerance(0.00001f));

  const float range_max = 7190.0f;
  const float range_min = -121830.0f;
  const float interval_size = (range_max - range_min) / (32);
  float chosen_value = interval_size * (random_draw + 31) + range_min;
  BOOST_TEST(chosen_value == range_max, test::tolerance(0.000000001f));
}


