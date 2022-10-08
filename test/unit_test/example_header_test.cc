// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/cb.h"
#include "vw/core/cost_sensitive.h"
#include "vw/core/reductions/conditional_contextual_bandit.h"
#include "vw/core/vw.h"

#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(is_example_header_cb)
{
  auto& vw = *VW::initialize("--cb_explore_adf --quiet", nullptr, false, nullptr, nullptr);
  VW::multi_ex examples;
  examples.push_back(VW::read_example(vw, "shared | s_1 s_2"));
  examples.push_back(VW::read_example(vw, "0:1.0:0.5 | a:1 b:1 c:1"));
  examples.push_back(VW::read_example(vw, "| a:0.5 b:2 c:1"));

  BOOST_CHECK_EQUAL(VW::is_cb_example_header(*examples[0]), true);
  BOOST_CHECK_EQUAL(VW::is_cs_example_header(*examples[0]), false);

  BOOST_CHECK_EQUAL(VW::is_cb_example_header(*examples[1]), false);
  BOOST_CHECK_EQUAL(VW::is_cs_example_header(*examples[1]), false);

  BOOST_CHECK_EQUAL(VW::is_cb_example_header(*examples[2]), false);
  BOOST_CHECK_EQUAL(VW::is_cs_example_header(*examples[2]), false);
  VW::finish_example(vw, examples);
  VW::finish(vw);
}

BOOST_AUTO_TEST_CASE(is_example_header_ccb)
{
  auto& vw = *VW::initialize("--ccb_explore_adf --quiet", nullptr, false, nullptr, nullptr);
  VW::multi_ex examples;
  examples.push_back(VW::read_example(vw, "ccb shared |User f"));
  examples.push_back(VW::read_example(vw, "ccb action |Action f"));

  BOOST_CHECK_EQUAL(VW::reductions::ccb::ec_is_example_header(*examples[0]), true);
  BOOST_CHECK_EQUAL(VW::reductions::ccb::ec_is_example_header(*examples[1]), false);
  VW::finish_example(vw, examples);
  VW::finish(vw);
}

BOOST_AUTO_TEST_CASE(is_example_header_csoaa)
{
  auto& vw = *VW::initialize("--csoaa_ldf multiline --quiet", nullptr, false, nullptr, nullptr);
  VW::multi_ex examples;
  examples.push_back(VW::read_example(vw, "shared | a_2 b_2 c_2"));
  examples.push_back(VW::read_example(vw, "3:2.0 | a_3 b_3 c_3"));

  BOOST_CHECK_EQUAL(VW::is_cb_example_header(*examples[0]), false);
  BOOST_CHECK_EQUAL(VW::is_cs_example_header(*examples[0]), true);

  BOOST_CHECK_EQUAL(VW::is_cb_example_header(*examples[1]), false);
  BOOST_CHECK_EQUAL(VW::is_cs_example_header(*examples[1]), false);
  VW::finish_example(vw, examples);
  VW::finish(vw);
}
