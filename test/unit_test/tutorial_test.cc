#ifndef STATIC_LINK_VW
#  define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "simulator.h"

BOOST_AUTO_TEST_CASE(cpp_simulator_without_interaction)
{
  auto ctr = simulator::_test_helper("--cb_explore_adf --quiet --epsilon 0.2 --random_seed 5");
  BOOST_CHECK_GT(ctr.back(), 0.37f);
  BOOST_CHECK_LT(ctr.back(), 0.49f);
}

BOOST_AUTO_TEST_CASE(cpp_simulator_with_interaction)
{
  auto ctr = simulator::_test_helper("--cb_explore_adf -q UA --quiet --epsilon 0.2 --random_seed 5");
  float without_save = ctr.back();
  BOOST_CHECK_GT(without_save, 0.7f);

  ctr = simulator::_test_helper_save_load("--cb_explore_adf -q UA --quiet --epsilon 0.2 --random_seed 5 --save_resume");
  float with_save = ctr.back();
  BOOST_CHECK_GT(with_save, 0.7f);

  BOOST_CHECK_CLOSE(without_save, with_save, FLOAT_TOL);
}