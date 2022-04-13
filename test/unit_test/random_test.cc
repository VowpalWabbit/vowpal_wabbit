// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "rand48.h"
#include "rand_state.h"
#include "test_common.h"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(reproduce_max_boundary_issue)
{
  uint64_t seed = 58587211;
  const uint64_t new_random_seed = VW::common::uniform_hash(&seed, sizeof(seed), seed);
  BOOST_CHECK_EQUAL(new_random_seed, 2244123448);

  float random_draw = merand48_noadvance(new_random_seed);
  BOOST_CHECK_CLOSE(random_draw, 0.99999f, 0.001f);

  const float range_max = 7190.0f;
  const float range_min = -121830.0f;
  const float interval_size = (range_max - range_min) / (32);
  float chosen_value = interval_size * (random_draw + 31) + range_min;
  BOOST_CHECK_CLOSE(chosen_value, range_max, 0.000000001f);
}

BOOST_AUTO_TEST_CASE(check_rand_state_cross_platform)
{
  VW::rand_state random_state;
  random_state.set_random_state(10);
  BOOST_CHECK_CLOSE(random_state.get_and_update_random(), 0.0170166492, FLOAT_TOL);
  BOOST_CHECK_CLOSE(random_state.get_and_update_random(), 0.370730162, FLOAT_TOL);
  BOOST_CHECK_CLOSE(random_state.get_and_update_random(), 0.166691661, FLOAT_TOL);
  BOOST_CHECK_CLOSE(random_state.get_and_update_random(), 0.362691283, FLOAT_TOL);
  BOOST_CHECK_CLOSE(random_state.get_and_update_random(), 0.969445825, FLOAT_TOL);
}
