
#ifndef STATIC_LINK_VW
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>

#include "vw.h"

BOOST_AUTO_TEST_CASE(cb_explore_adf_should_throw_empty_multi_example) {
  auto vw = VW::initialize("--cb_explore_adf --quiet", nullptr, false, nullptr, nullptr);
  multi_ex example_collection;

  // An empty example collection is invalid and so should throw.
  BOOST_REQUIRE_THROW(vw->learn(example_collection), VW::vw_exception);
  VW::finish(*vw);
}
