#ifndef STATIC_LINK_VW
#  define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "simulator.h"

std::vector<float> _test_helper(const std::string& vw_arg, int num_iterations = 3000, int seed = 10)
{
  auto vw = VW::initialize(vw_arg);
  simulator::cb_sim sim(seed);
  auto ctr = sim.run_simulation(vw, num_iterations);
  VW::finish(*vw);
  return ctr;
}

std::vector<float> _test_helper_save_load(const std::string& vw_arg, int num_iterations = 3000, int seed = 10)
{
  int split = 1500;
  int before_save = num_iterations - split;

  auto first_vw = VW::initialize(vw_arg);
  simulator::cb_sim sim(seed);
  // first chunk
  auto ctr = sim.run_simulation(first_vw, before_save);
  // save
  std::string model_file = "test_save_load.vw";
  VW::save_predictor(*first_vw, model_file);
  VW::finish(*first_vw);
  // reload in another instance
  auto other_vw = VW::initialize("--quiet -i " + model_file);
  // continue
  ctr = sim.run_simulation(other_vw, split, true, before_save + 1);
  VW::finish(*other_vw);
  return ctr;
}

BOOST_AUTO_TEST_CASE(cpp_simulator_without_interaction)
{
  auto ctr = _test_helper("--cb_explore_adf --quiet --epsilon 0.2 --random_seed 5");
  BOOST_CHECK(ctr.back() <= 0.49f && ctr.back() >= 0.38f);
}

BOOST_AUTO_TEST_CASE(cpp_simulator_with_interaction)
{
  auto ctr = _test_helper("--cb_explore_adf -q UA --quiet --epsilon 0.2 --random_seed 5");
  float without_save = ctr.back();
  BOOST_CHECK(without_save >= 0.7f);

  ctr = _test_helper_save_load("--cb_explore_adf -q UA --quiet --epsilon 0.2 --random_seed 5 --save_resume");
  float with_save = ctr.back();
  BOOST_CHECK(with_save >= 0.7f);

  BOOST_CHECK_CLOSE(without_save, with_save, FLOAT_TOL);
}