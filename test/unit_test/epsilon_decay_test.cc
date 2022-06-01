#include "vw/core/reductions/epsilon_decay.h"

#include "simulator.h"
#include "test_common.h"
#include "vw/core/metric_sink.h"
#include "vw/core/reductions_fwd.h"
#include "vw/io/logger.h"

#include <boost/test/test_tools.hpp>
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

BOOST_AUTO_TEST_CASE(epsilon_decay_test_champ_change)
{
  const size_t num_iterations = 10000;
  const std::vector<uint64_t> swap_after = {5000};
  const size_t seed = 100;
  const size_t deterministic_champ_switch = 5781;
  callback_map test_hooks;

  test_hooks.emplace(deterministic_champ_switch - 1, [&](cb_sim&, VW::workspace& all, VW::multi_ex&) {
    epsilon_decay_data* epsilon_decay = epsilon_decay_test::get_epsilon_decay_data(all);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[0][0].update_count, 15);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[1][0].update_count, 15);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[2][0].update_count, 15);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[3][0].update_count, 15);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[1][1].update_count, 41);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[2][1].update_count, 41);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[3][1].update_count, 41);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[2][2].update_count, 459);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[3][2].update_count, 459);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[3][3].update_count, 5780);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[0][0].get_score_idx(), 0);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[1][0].get_score_idx(), 4);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[2][0].get_score_idx(), 3);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[3][0].get_score_idx(), 6);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[1][1].get_score_idx(), 2);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[2][1].get_score_idx(), 1);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[3][1].get_score_idx(), 7);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[2][2].get_score_idx(), 5);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[3][2].get_score_idx(), 8);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[3][3].get_score_idx(), 9);
    return true;
  });

  test_hooks.emplace(deterministic_champ_switch, [&](cb_sim&, VW::workspace& all, VW::multi_ex&) {
    epsilon_decay_data* epsilon_decay = epsilon_decay_test::get_epsilon_decay_data(all);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[0][0].update_count, 0);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[1][0].update_count, 0);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[2][0].update_count, 0);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[3][0].update_count, 0);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[1][1].update_count, 16);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[2][1].update_count, 16);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[3][1].update_count, 16);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[2][2].update_count, 42);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[3][2].update_count, 42);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[3][3].update_count, 460);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[0][0].get_score_idx(), 9);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[1][0].get_score_idx(), 8);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[2][0].get_score_idx(), 7);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[3][0].get_score_idx(), 6);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[1][1].get_score_idx(), 0);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[2][1].get_score_idx(), 4);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[3][1].get_score_idx(), 3);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[2][2].get_score_idx(), 2);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[3][2].get_score_idx(), 1);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[3][3].get_score_idx(), 5);
    return true;
  });

  // we initialize the reduction pointing to position 0 as champ, that config is hard-coded to empty
  auto ctr = simulator::_test_helper_hook(
      "--epsilon_decay --model_count 4 --cb_explore_adf --quiet  -q ::", test_hooks, num_iterations, seed, swap_after);

  BOOST_CHECK_GT(ctr.back(), 0.8f);
}

BOOST_AUTO_TEST_CASE(epsilon_decay_test_update_count)
{
  const size_t num_iterations = 105;
  const size_t seed = 100;
  callback_map test_hooks;

  test_hooks.emplace(100, [&](cb_sim&, VW::workspace& all, VW::multi_ex&) {
    epsilon_decay_data* epsilon_decay = epsilon_decay_test::get_epsilon_decay_data(all);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[0][0].get_score_idx(), 0);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[1][0].get_score_idx(), 1);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[1][1].get_score_idx(), 2);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[2][0].get_score_idx(), 3);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[2][1].get_score_idx(), 4);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[2][2].get_score_idx(), 5);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[3][0].get_score_idx(), 6);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[3][1].get_score_idx(), 7);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[3][2].get_score_idx(), 8);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[3][3].get_score_idx(), 9);
    BOOST_CHECK_EQUAL(epsilon_decay->_weight_indices[0], 0);
    BOOST_CHECK_EQUAL(epsilon_decay->_weight_indices[1], 1);
    BOOST_CHECK_EQUAL(epsilon_decay->_weight_indices[2], 2);
    BOOST_CHECK_EQUAL(epsilon_decay->_weight_indices[3], 3);
    return true;
  });

  test_hooks.emplace(101, [&](cb_sim&, VW::workspace& all, VW::multi_ex&) {
    epsilon_decay_data* epsilon_decay = epsilon_decay_test::get_epsilon_decay_data(all);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[0][0].get_score_idx(), 0);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[1][0].get_score_idx(), 1);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[1][1].get_score_idx(), 2);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[2][0].get_score_idx(), 3);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[2][1].get_score_idx(), 4);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[2][2].get_score_idx(), 5);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[3][0].get_score_idx(), 6);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[3][1].get_score_idx(), 7);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[3][2].get_score_idx(), 8);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[3][3].get_score_idx(), 9);
    BOOST_CHECK_EQUAL(epsilon_decay->_weight_indices[0], 0);
    BOOST_CHECK_EQUAL(epsilon_decay->_weight_indices[1], 1);
    BOOST_CHECK_EQUAL(epsilon_decay->_weight_indices[2], 2);
    BOOST_CHECK_EQUAL(epsilon_decay->_weight_indices[3], 3);
    return true;
  });

  test_hooks.emplace(102, [&](cb_sim&, VW::workspace& all, VW::multi_ex&) {
    epsilon_decay_data* epsilon_decay = epsilon_decay_test::get_epsilon_decay_data(all);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[0][0].get_score_idx(), 2);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[1][0].get_score_idx(), 1);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[1][1].get_score_idx(), 0);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[2][0].get_score_idx(), 4);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[2][1].get_score_idx(), 3);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[2][2].get_score_idx(), 5);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[3][0].get_score_idx(), 7);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[3][1].get_score_idx(), 6);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[3][2].get_score_idx(), 8);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[3][3].get_score_idx(), 9);
    BOOST_CHECK_EQUAL(epsilon_decay->_weight_indices[0], 1);
    BOOST_CHECK_EQUAL(epsilon_decay->_weight_indices[1], 0);
    BOOST_CHECK_EQUAL(epsilon_decay->_weight_indices[2], 2);
    BOOST_CHECK_EQUAL(epsilon_decay->_weight_indices[3], 3);
    return true;
  });

  test_hooks.emplace(103, [&](cb_sim&, VW::workspace& all, VW::multi_ex&) {
    epsilon_decay_data* epsilon_decay = epsilon_decay_test::get_epsilon_decay_data(all);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[0][0].get_score_idx(), 5);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[1][0].get_score_idx(), 3);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[1][1].get_score_idx(), 2);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[2][0].get_score_idx(), 4);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[2][1].get_score_idx(), 1);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[2][2].get_score_idx(), 0);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[3][0].get_score_idx(), 8);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[3][1].get_score_idx(), 7);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[3][2].get_score_idx(), 6);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[3][3].get_score_idx(), 9);
    BOOST_CHECK_EQUAL(epsilon_decay->_weight_indices[0], 2);
    BOOST_CHECK_EQUAL(epsilon_decay->_weight_indices[1], 1);
    BOOST_CHECK_EQUAL(epsilon_decay->_weight_indices[2], 0);
    BOOST_CHECK_EQUAL(epsilon_decay->_weight_indices[3], 3);
    return true;
  });

  test_hooks.emplace(104, [&](cb_sim&, VW::workspace& all, VW::multi_ex&) {
    epsilon_decay_data* epsilon_decay = epsilon_decay_test::get_epsilon_decay_data(all);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[0][0].get_score_idx(), 5);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[1][0].get_score_idx(), 3);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[1][1].get_score_idx(), 2);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[2][0].get_score_idx(), 4);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[2][1].get_score_idx(), 1);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[2][2].get_score_idx(), 0);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[3][0].get_score_idx(), 8);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[3][1].get_score_idx(), 7);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[3][2].get_score_idx(), 6);
    BOOST_CHECK_EQUAL(epsilon_decay->_scored_configs[3][3].get_score_idx(), 9);
    BOOST_CHECK_EQUAL(epsilon_decay->_weight_indices[0], 2);
    BOOST_CHECK_EQUAL(epsilon_decay->_weight_indices[1], 1);
    BOOST_CHECK_EQUAL(epsilon_decay->_weight_indices[2], 0);
    BOOST_CHECK_EQUAL(epsilon_decay->_weight_indices[3], 3);
    return true;
  });

  // we initialize the reduction pointing to position 0 as champ, that config is hard-coded to empty
  auto ctr = simulator::_test_helper_hook(
      "--epsilon_decay --model_count 4 --cb_explore_adf --quiet  -q ::", test_hooks, num_iterations, seed);

  BOOST_CHECK_GT(ctr.back(), 0.5f);
}

BOOST_AUTO_TEST_CASE(epsilon_decay_test_save_load)
{
  callback_map empty_hooks;
  auto ctr = simulator::_test_helper_hook(
      "--epsilon_decay --model_count 5 --cb_explore_adf --epsilon_decay_alpha .01 --quiet  -q ::", empty_hooks);
  float without_save = ctr.back();
  BOOST_CHECK_GT(without_save, 0.9f);

  ctr = simulator::_test_helper_save_load(
      "--epsilon_decay --model_count 5 --cb_explore_adf --epsilon_decay_alpha .01 --quiet  -q ::");

  float with_save = ctr.back();
  BOOST_CHECK_GT(with_save, 0.9f);

  BOOST_CHECK_CLOSE(without_save, with_save, FLOAT_TOL);
}

BOOST_AUTO_TEST_CASE(epsilon_decay_test_score_bounds_unit)
{
  // Initialize epsilon_decay_data struct with 5 models
  uint64_t K = 5;
  dense_parameters dense_weights(K);
  VW::io::logger logger = VW::io::create_default_logger();
  epsilon_decay_data ep_data(K, 100, .05, .1, dense_weights, logger, false, false);

  /*
   * Check that score and weight indices are in expected start positions:
   * WX denotes weight index X, and other indices denote score index
   *
   * W0: 0
   * W1: 1  2
   * W2: 3  4  5
   * W3: 6  7  8  9
   * W4: 10 11 12 13 14
   *
   */
  BOOST_CHECK_EQUAL(ep_data._scored_configs[0][0].get_score_idx(), 0);
  BOOST_CHECK_EQUAL(ep_data._scored_configs[1][0].get_score_idx(), 1);
  BOOST_CHECK_EQUAL(ep_data._scored_configs[1][1].get_score_idx(), 2);
  BOOST_CHECK_EQUAL(ep_data._scored_configs[2][0].get_score_idx(), 3);
  BOOST_CHECK_EQUAL(ep_data._scored_configs[2][1].get_score_idx(), 4);
  BOOST_CHECK_EQUAL(ep_data._scored_configs[2][2].get_score_idx(), 5);
  BOOST_CHECK_EQUAL(ep_data._scored_configs[3][0].get_score_idx(), 6);
  BOOST_CHECK_EQUAL(ep_data._scored_configs[3][1].get_score_idx(), 7);
  BOOST_CHECK_EQUAL(ep_data._scored_configs[3][2].get_score_idx(), 8);
  BOOST_CHECK_EQUAL(ep_data._scored_configs[3][3].get_score_idx(), 9);
  BOOST_CHECK_EQUAL(ep_data._scored_configs[4][0].get_score_idx(), 10);
  BOOST_CHECK_EQUAL(ep_data._scored_configs[4][1].get_score_idx(), 11);
  BOOST_CHECK_EQUAL(ep_data._scored_configs[4][2].get_score_idx(), 12);
  BOOST_CHECK_EQUAL(ep_data._scored_configs[4][3].get_score_idx(), 13);
  BOOST_CHECK_EQUAL(ep_data._scored_configs[4][4].get_score_idx(), 14);
  BOOST_CHECK_EQUAL(ep_data._weight_indices[0], 0);
  BOOST_CHECK_EQUAL(ep_data._weight_indices[1], 1);
  BOOST_CHECK_EQUAL(ep_data._weight_indices[2], 2);
  BOOST_CHECK_EQUAL(ep_data._weight_indices[3], 3);
  BOOST_CHECK_EQUAL(ep_data._weight_indices[4], 4);

  // Set lower_bound of model 2 to beat upper_bound of current champ and run score check
  uint64_t new_champ = 2;
  ep_data._scored_configs[new_champ][new_champ]._lower_bound = 1;
  BOOST_CHECK_GT(ep_data._scored_configs[new_champ][new_champ].get_lower_bound(),
      ep_data._scored_configs[K - 1][new_champ].get_upper_bound());
  ep_data.check_score_bounds();

  /*
   * Check that weight and score indices shift as expected when model 2 overtakes champion:
   * WX denotes weight index X, and other indices denote score index
   *
   * W4: 14
   * W3: 8  9
   * W0: 12 13 0
   * W1: 6  7  1  2
   * W2: 10 11 3  4  5
   *
   */
  BOOST_CHECK_EQUAL(ep_data._scored_configs[0][0].get_score_idx(), 14);
  BOOST_CHECK_EQUAL(ep_data._scored_configs[1][0].get_score_idx(), 8);
  BOOST_CHECK_EQUAL(ep_data._scored_configs[1][1].get_score_idx(), 9);
  BOOST_CHECK_EQUAL(ep_data._scored_configs[2][0].get_score_idx(), 12);
  BOOST_CHECK_EQUAL(ep_data._scored_configs[2][1].get_score_idx(), 13);
  BOOST_CHECK_EQUAL(ep_data._scored_configs[2][2].get_score_idx(), 0);
  BOOST_CHECK_EQUAL(ep_data._scored_configs[3][0].get_score_idx(), 6);
  BOOST_CHECK_EQUAL(ep_data._scored_configs[3][1].get_score_idx(), 7);
  BOOST_CHECK_EQUAL(ep_data._scored_configs[3][2].get_score_idx(), 1);
  BOOST_CHECK_EQUAL(ep_data._scored_configs[3][3].get_score_idx(), 2);
  BOOST_CHECK_EQUAL(ep_data._scored_configs[4][0].get_score_idx(), 10);
  BOOST_CHECK_EQUAL(ep_data._scored_configs[4][1].get_score_idx(), 11);
  BOOST_CHECK_EQUAL(ep_data._scored_configs[4][2].get_score_idx(), 3);
  BOOST_CHECK_EQUAL(ep_data._scored_configs[4][3].get_score_idx(), 4);
  BOOST_CHECK_EQUAL(ep_data._scored_configs[4][4].get_score_idx(), 5);
  BOOST_CHECK_EQUAL(ep_data._weight_indices[0], 4);
  BOOST_CHECK_EQUAL(ep_data._weight_indices[1], 3);
  BOOST_CHECK_EQUAL(ep_data._weight_indices[2], 0);
  BOOST_CHECK_EQUAL(ep_data._weight_indices[3], 1);
  BOOST_CHECK_EQUAL(ep_data._weight_indices[4], 2);
}

BOOST_AUTO_TEST_CASE(epsilon_decay_test_horizon_bounds_unit)
{
  // Initialize epsilon_decay_data struct with 5 models
  uint64_t K = 5;
  dense_parameters dense_weights(K);
  VW::io::logger logger = VW::io::create_default_logger();
  epsilon_decay_data ep_data(K, 100, .05, .1, dense_weights, logger, false, false);

  /*
   * Check that score and weight indices are in expected start positions:
   * WX denotes weight index X, and other indices denote score index
   *
   * W0: 0
   * W1: 1  2
   * W2: 3  4  5
   * W3: 6  7  8  9
   * W4: 10 11 12 13 14
   *
   */
  BOOST_CHECK_EQUAL(ep_data._scored_configs[0][0].get_score_idx(), 0);
  BOOST_CHECK_EQUAL(ep_data._scored_configs[1][0].get_score_idx(), 1);
  BOOST_CHECK_EQUAL(ep_data._scored_configs[1][1].get_score_idx(), 2);
  BOOST_CHECK_EQUAL(ep_data._scored_configs[2][0].get_score_idx(), 3);
  BOOST_CHECK_EQUAL(ep_data._scored_configs[2][1].get_score_idx(), 4);
  BOOST_CHECK_EQUAL(ep_data._scored_configs[2][2].get_score_idx(), 5);
  BOOST_CHECK_EQUAL(ep_data._scored_configs[3][0].get_score_idx(), 6);
  BOOST_CHECK_EQUAL(ep_data._scored_configs[3][1].get_score_idx(), 7);
  BOOST_CHECK_EQUAL(ep_data._scored_configs[3][2].get_score_idx(), 8);
  BOOST_CHECK_EQUAL(ep_data._scored_configs[3][3].get_score_idx(), 9);
  BOOST_CHECK_EQUAL(ep_data._scored_configs[4][0].get_score_idx(), 10);
  BOOST_CHECK_EQUAL(ep_data._scored_configs[4][1].get_score_idx(), 11);
  BOOST_CHECK_EQUAL(ep_data._scored_configs[4][2].get_score_idx(), 12);
  BOOST_CHECK_EQUAL(ep_data._scored_configs[4][3].get_score_idx(), 13);
  BOOST_CHECK_EQUAL(ep_data._scored_configs[4][4].get_score_idx(), 14);
  BOOST_CHECK_EQUAL(ep_data._weight_indices[0], 0);
  BOOST_CHECK_EQUAL(ep_data._weight_indices[1], 1);
  BOOST_CHECK_EQUAL(ep_data._weight_indices[2], 2);
  BOOST_CHECK_EQUAL(ep_data._weight_indices[3], 3);
  BOOST_CHECK_EQUAL(ep_data._weight_indices[4], 4);

  // Set update_count of model 2 to be above threshold horizon based on champion
  uint64_t over_horizon = 2;
  ep_data._scored_configs[over_horizon][over_horizon].update_count = 500;
  ep_data._scored_configs[K - 1][K - 1].update_count = 1000;
  BOOST_CHECK_GT(ep_data._scored_configs[over_horizon][over_horizon].update_count,
      std::pow(ep_data._scored_configs[K - 1][K - 1].update_count, static_cast<float>(over_horizon + 1) / K));
  ep_data.check_horizon_bounds();

  /*
   * Check that weight and score indices shift as expected when model 2 overtakes champion:
   * WX denotes weight index X, and other indices denote score index
   *
   * W2: 5
   * W0: 4  0
   * W1: 3  1  2
   * W3: 8  6  7  9
   * W4: 12 10 11 13 14
   *
   */
  BOOST_CHECK_EQUAL(ep_data._scored_configs[0][0].get_score_idx(), 5);
  BOOST_CHECK_EQUAL(ep_data._scored_configs[1][0].get_score_idx(), 4);
  BOOST_CHECK_EQUAL(ep_data._scored_configs[1][1].get_score_idx(), 0);
  BOOST_CHECK_EQUAL(ep_data._scored_configs[2][0].get_score_idx(), 3);
  BOOST_CHECK_EQUAL(ep_data._scored_configs[2][1].get_score_idx(), 1);
  BOOST_CHECK_EQUAL(ep_data._scored_configs[2][2].get_score_idx(), 2);
  BOOST_CHECK_EQUAL(ep_data._scored_configs[3][0].get_score_idx(), 8);
  BOOST_CHECK_EQUAL(ep_data._scored_configs[3][1].get_score_idx(), 6);
  BOOST_CHECK_EQUAL(ep_data._scored_configs[3][2].get_score_idx(), 7);
  BOOST_CHECK_EQUAL(ep_data._scored_configs[3][3].get_score_idx(), 9);
  BOOST_CHECK_EQUAL(ep_data._scored_configs[4][0].get_score_idx(), 12);
  BOOST_CHECK_EQUAL(ep_data._scored_configs[4][1].get_score_idx(), 10);
  BOOST_CHECK_EQUAL(ep_data._scored_configs[4][2].get_score_idx(), 11);
  BOOST_CHECK_EQUAL(ep_data._scored_configs[4][3].get_score_idx(), 13);
  BOOST_CHECK_EQUAL(ep_data._scored_configs[4][4].get_score_idx(), 14);
  BOOST_CHECK_EQUAL(ep_data._weight_indices[0], 2);
  BOOST_CHECK_EQUAL(ep_data._weight_indices[1], 0);
  BOOST_CHECK_EQUAL(ep_data._weight_indices[2], 1);
  BOOST_CHECK_EQUAL(ep_data._weight_indices[3], 3);
  BOOST_CHECK_EQUAL(ep_data._weight_indices[4], 4);
}
