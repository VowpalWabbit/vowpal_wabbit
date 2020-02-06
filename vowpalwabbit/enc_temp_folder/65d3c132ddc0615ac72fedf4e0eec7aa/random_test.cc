#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>
#include <explore_internal.h>
#include "rand48.h"
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

// Draw a random number between [range_min, range_max * edge_avoid_factor]
// and advance pseudo-random state
inline float internal_interval_draw(float range_min, float range_max, uint64_t* p_random_seed, float edge_avoid_factor)
{
  // Draw a float and then advance the pseudo-random state
  const float random_draw = edge_avoid_factor * merand48(*p_random_seed);
  const float interval_size = (range_max - range_min);
  const float chosen_value = interval_size * random_draw + range_min;
  return chosen_value;
}

// Draw a random number between [range_min, range_max)
// and advance pseudo-random state
float inline uniform_draw(float range_min, float range_max, uint64_t* p_random_seed)
{
  float chosen_value;

  do
  {
    const float edge_avoid_factor = 1.0001f;
    chosen_value = internal_interval_draw(range_min, range_max, p_random_seed, edge_avoid_factor);
  } while (chosen_value >= range_max);

  return chosen_value;
}

BOOST_AUTO_TEST_CASE(fix_max_boundary_issue)
{
  uint64_t random_state = 2244123448;

  const float range_max = 7190.0f;
  const float range_min = -121830.0f;

  const float interval_start = range_min + (range_max - range_min) * 31.0f / 32.0f;
  const float interval_end = range_max;

  const float chosen_value = uniform_draw(interval_start, interval_end, &random_state);
  BOOST_TEST(chosen_value != range_max, test::tolerance(0.000000001f));
}
