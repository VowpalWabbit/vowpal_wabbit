// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/stable_unique.h"

#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>
#include <vector>

BOOST_AUTO_TEST_CASE(stable_unique_sorted)
{
  std::vector<int> v = {1,1,2,2,2,6,78,89,89};
  std::vector<int> expected = {1,2,6,78,89};

  v.erase(stable_unique(v.begin(), v.end()), v.end());

  BOOST_CHECK_EQUAL_COLLECTIONS(v.begin(), v.end(), expected.begin(), expected.end());
}

BOOST_AUTO_TEST_CASE(stable_unique_unsorted)
{
  std::vector<int> v = {45,65,76,23,45,11,34,23,45,-34,-1,22,23,22,0,0,11,23,109};
  std::vector<int> expected = {45,65,76,23,11,34,-34,-1,22,0,109};

  v.erase(stable_unique(v.begin(), v.end()), v.end());

  BOOST_CHECK_EQUAL_COLLECTIONS(v.begin(), v.end(), expected.begin(), expected.end());
}
