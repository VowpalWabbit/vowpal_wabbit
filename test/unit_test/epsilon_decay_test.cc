
#include "reductions/epsilon_decay.h"

#include "metric_sink.h"
#include "reductions_fwd.h"
#include "simulator.h"
#include "test_common.h"

#include <boost/test/test_tools.hpp>
#include <boost/test/tools/old/interface.hpp>
#include <boost/test/unit_test.hpp>
#include <functional>
#include <map>
#include <utility>

using simulator::callback_map;
using simulator::cb_sim;
using namespace VW::reductions::epsilon_decay;

namespace epsilon_decay_test
{
epsilon_decay_data* get_epsilon_decay_data(VW::workspace& all)
{
  std::vector<std::string> e_r;
  all.l->get_enabled_reductions(e_r);
  if (std::find(e_r.begin(), e_r.end(), "epsilon_decay") == e_r.end())
  { BOOST_FAIL("Epsilon decay not found in enabled reductions"); }

  VW::LEARNER::multi_learner* epsilon_decay_learner = as_multiline(all.l->get_learner_by_name_prefix("epsilon_decay"));

  return (epsilon_decay_data*)epsilon_decay_learner->get_internal_type_erased_data_pointer_test_use_only();
}
}  // namespace epsilon_decay_test

BOOST_AUTO_TEST_CASE(epsilon_decay_test_init)
{
  // we initialize the reduction pointing to position 0 as champ, that config is hard-coded to empty
  auto ctr =
      simulator::_test_helper("--epsilon_decay --model_count 3 --cb_explore_adf --quiet --epsilon 0.2 --random_seed 5");
  float with_save = ctr.back();
}

BOOST_AUTO_TEST_CASE(epsilon_decay_test_change_dist)
{
  const size_t num_iterations = 10000;
  const std::vector<uint64_t> swap_after = {5000};
  const size_t seed = 10;
  const size_t deterministic_champ_switch = 6895;
  callback_map test_hooks;

  test_hooks.emplace(deterministic_champ_switch - 1, [&](cb_sim&, VW::workspace& all, multi_ex&) {
    epsilon_decay_data* epsilon_decay = epsilon_decay_test::get_epsilon_decay_data(all);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[0].update_count, 12);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[1].update_count, 18);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[2].update_count, 285);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[3].update_count, 6894);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[0].get_model_idx(), 1);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[1].get_model_idx(), 2);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[2].get_model_idx(), 0);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[3].get_model_idx(), 3);
    return true;
  });

  test_hooks.emplace(deterministic_champ_switch, [&](cb_sim&, VW::workspace& all, multi_ex&) {
    epsilon_decay_data* epsilon_decay = epsilon_decay_test::get_epsilon_decay_data(all);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[0].update_count, 0);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[1].update_count, 13);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[2].update_count, 19);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[3].update_count, 286);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[0].get_model_idx(), 3);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[1].get_model_idx(), 1);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[2].get_model_idx(), 2);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[3].get_model_idx(), 0);
    return true;
  });

  // we initialize the reduction pointing to position 0 as champ, that config is hard-coded to empty
  auto ctr = simulator::_test_helper_hook(
      "--epsilon_decay --model_count 4 --cb_explore_adf --quiet  -q ::", test_hooks, num_iterations, seed, swap_after);

  BOOST_CHECK_GT(ctr.back(), 0.4f);
}

BOOST_AUTO_TEST_CASE(epsilon_decay_save_load)
{
  callback_map empty_hooks;
  auto ctr = simulator::_test_helper_hook(
      "--epsilon_decay --model_count 5 --cb_explore_adf --epsilon_decay_alpha .01 --quiet  -q ::", empty_hooks);
  float without_save = ctr.back();
  BOOST_CHECK_GT(without_save, 0.8f);

  ctr = simulator::_test_helper_save_load(
      "--epsilon_decay --model_count 5 --cb_explore_adf --epsilon_decay_alpha .01 --quiet  -q ::");

  float with_save = ctr.back();
  BOOST_CHECK_GT(with_save, 0.8f);

  BOOST_CHECK_CLOSE(without_save, with_save, FLOAT_TOL);
}
