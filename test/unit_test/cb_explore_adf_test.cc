// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <boost/test/unit_test.hpp>

#include "vw.h"

BOOST_AUTO_TEST_CASE(cb_explore_adf_should_throw_empty_multi_example) {
  auto vw = VW::initialize("--cb_explore_adf --quiet", nullptr, false, nullptr, nullptr);
  VW::multi_ex example_collection;

  // An empty example collection is invalid and so should throw.
  BOOST_REQUIRE_THROW(vw->learn(example_collection), VW::vw_exception);
  VW::finish(*vw);
}
