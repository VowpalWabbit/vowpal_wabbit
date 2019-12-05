#ifndef STATIC_LINK_VW
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "vw.h"
#include "cb.h"
#include "cost_sensitive.h"
#include "conditional_contextual_bandit.h"

BOOST_AUTO_TEST_CASE(is_example_header_cb) {
  auto& vw = *VW::initialize("--cb_explore_adf --quiet", nullptr, false, nullptr, nullptr);
  multi_ex examples;
  examples.push_back(VW::read_example(vw, "shared | s_1 s_2"));
  examples.push_back(VW::read_example(vw, "0:1.0:0.5 | a:1 b:1 c:1"));
  examples.push_back(VW::read_example(vw, "| a:0.5 b:2 c:1"));

  BOOST_CHECK_EQUAL(CB::ec_is_example_header(*examples[0]), true);
  BOOST_CHECK_EQUAL(COST_SENSITIVE::ec_is_example_header(*examples[0]), false);

  BOOST_CHECK_EQUAL(CB::ec_is_example_header(*examples[1]), false);
  BOOST_CHECK_EQUAL(COST_SENSITIVE::ec_is_example_header(*examples[1]), false);

  BOOST_CHECK_EQUAL(CB::ec_is_example_header(*examples[2]), false);
  BOOST_CHECK_EQUAL(COST_SENSITIVE::ec_is_example_header(*examples[2]), false);
  VW::finish_example(vw, examples);
  VW::finish(vw);
}

BOOST_AUTO_TEST_CASE(is_example_header_ccb) {
  auto& vw = *VW::initialize("--ccb_explore_adf --quiet", nullptr, false, nullptr, nullptr);
  multi_ex examples;
  examples.push_back(VW::read_example(vw, "ccb shared |User f"));
  examples.push_back(VW::read_example(vw, "ccb action |Action f"));

  BOOST_CHECK_EQUAL(CCB::ec_is_example_header(*examples[0]), true);
  BOOST_CHECK_EQUAL(CCB::ec_is_example_header(*examples[1]), false);
  VW::finish_example(vw, examples);
  VW::finish(vw);
}

BOOST_AUTO_TEST_CASE(is_example_header_csoaa) {
  auto& vw = *VW::initialize("--csoaa_ldf multiline --quiet", nullptr, false, nullptr, nullptr);
  multi_ex examples;
  examples.push_back(VW::read_example(vw, "shared | a_2 b_2 c_2"));
  examples.push_back(VW::read_example(vw, "3:2.0 | a_3 b_3 c_3"));

  BOOST_CHECK_EQUAL(CB::ec_is_example_header(*examples[0]), false);
  BOOST_CHECK_EQUAL(COST_SENSITIVE::ec_is_example_header(*examples[0]), true);

  BOOST_CHECK_EQUAL(CB::ec_is_example_header(*examples[1]), false);
  BOOST_CHECK_EQUAL(COST_SENSITIVE::ec_is_example_header(*examples[1]), false);
  VW::finish_example(vw, examples);
  VW::finish(vw);
}
