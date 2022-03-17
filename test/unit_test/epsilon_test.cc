// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <boost/test/tools/old/interface.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "test_common.h"
#include "reduction_features.h"
#include "epsilon_reduction_features.h"

BOOST_AUTO_TEST_CASE(set_epsilon_test)
{
  auto vw = VW::initialize("--quiet --cb_explore_adf");
  VW::multi_ex examples;
  examples.push_back(VW::read_example(*vw, std::string("")));
  auto& ep_fts = examples[0]->_reduction_features.template get<VW::cb_explore_adf::greedy::reduction_features>();
  BOOST_CHECK_EQUAL(ep_fts.epsilon, -1.f);
  ep_fts.epsilon = 0.5f;
  auto& ep_fts2 = examples[0]->_reduction_features.template get<VW::cb_explore_adf::greedy::reduction_features>();
  BOOST_CHECK_EQUAL(ep_fts2.epsilon, 0.5f);
  ep_fts2.reset_to_default();
  BOOST_CHECK_EQUAL(ep_fts2.epsilon, -1.f);
  vw->finish_example(examples);
  VW::finish(*vw);
}
