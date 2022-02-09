
#include <boost/test/tools/old/interface.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "test_common.h"
#include "simulator.h"
#include "reductions_fwd.h"
#include "ae.h"
#include "metric_sink.h"

#include <functional>
#include <map>
#include <utility>

using simulator::callback_map;
using simulator::cb_sim;
using namespace VW::ae;

namespace ae_test
{
VW::ae::ae_data* get_ae_data(VW::workspace& all)
{
  std::vector<std::string> e_r;
  all.l->get_enabled_reductions(e_r);
  if (std::find(e_r.begin(), e_r.end(), "agedexp") == e_r.end())
  { BOOST_FAIL("Epsilon decay not found in enabled reductions"); }

  VW::LEARNER::multi_learner* ae_learner = as_multiline(all.l->get_learner_by_name_prefix("agedexp"));

  return (VW::ae::ae_data*)ae_learner->get_internal_type_erased_data_pointer_test_use_only();
}
}  // namespace ae_test

BOOST_AUTO_TEST_CASE(ae_test_init)
{
  // we initialize the reduction pointing to position 0 as champ, that config is hard-coded to empty
  auto ctr =
      simulator::_test_helper("--agedexp --model_count 3 --cb_explore_adf --quiet --epsilon 0.2 --random_seed 5");
  float with_save = ctr.back();
}

BOOST_AUTO_TEST_CASE(ae_test_champ_switch)
{
  callback_map empty_hooks;
  std::vector<uint64_t> swap_after = {100, 200, 300, 400};
  size_t num_iterations = 3000;
  int seed = 10;
  auto ctr = simulator::_test_helper_hook(
      "--agedexp --model_count 5 --cb_explore_adf --quiet  -q ::", empty_hooks, num_iterations, seed, swap_after);
}

BOOST_AUTO_TEST_CASE(ae_test_change_dist)
{
  const size_t num_iterations = 10000;
  const std::vector<uint64_t> swap_after = {5000};
  const size_t seed = 10;
  const size_t deterministic_champ_switch = 6710;
  callback_map test_hooks;

  test_hooks.emplace(deterministic_champ_switch - 1, [&](cb_sim&, VW::workspace& all, multi_ex&) {
    VW::ae::ae_data* ae = ae_test::get_ae_data(all);
    BOOST_CHECK_EQUAL(ae->scored_configs[0].update_count, 29);
    BOOST_CHECK_EQUAL(ae->scored_configs[1].update_count, 35);
    BOOST_CHECK_EQUAL(ae->scored_configs[2].update_count, 100);
    BOOST_CHECK_EQUAL(ae->scored_configs[3].update_count, 6709);
    BOOST_CHECK_EQUAL(ae->scored_configs[0].get_model_idx(), 1);
    BOOST_CHECK_EQUAL(ae->scored_configs[1].get_model_idx(), 2);
    BOOST_CHECK_EQUAL(ae->scored_configs[2].get_model_idx(), 0);
    BOOST_CHECK_EQUAL(ae->scored_configs[3].get_model_idx(), 3);
    return true;
  });

  test_hooks.emplace(deterministic_champ_switch, [&](cb_sim&, VW::workspace& all, multi_ex&) {
    VW::ae::ae_data* ae = ae_test::get_ae_data(all);
    BOOST_CHECK_EQUAL(ae->scored_configs[0].update_count, 0);
    BOOST_CHECK_EQUAL(ae->scored_configs[1].update_count, 30);
    BOOST_CHECK_EQUAL(ae->scored_configs[2].update_count, 36);
    BOOST_CHECK_EQUAL(ae->scored_configs[3].update_count, 101);
    BOOST_CHECK_EQUAL(ae->scored_configs[0].get_model_idx(), 3);
    BOOST_CHECK_EQUAL(ae->scored_configs[1].get_model_idx(), 1);
    BOOST_CHECK_EQUAL(ae->scored_configs[2].get_model_idx(), 2);
    BOOST_CHECK_EQUAL(ae->scored_configs[3].get_model_idx(), 0);
    return true;
  });

  // we initialize the reduction pointing to position 0 as champ, that config is hard-coded to empty
  auto ctr = simulator::_test_helper_hook(
      "--agedexp --model_count 4 --cb_explore_adf --quiet  -q ::", test_hooks, num_iterations, seed, swap_after);

  BOOST_CHECK_GT(ctr.back(), 0.4f);
}

BOOST_AUTO_TEST_CASE(ae_save_load)
{
  callback_map empty_hooks;
  auto ctr = simulator::_test_helper_hook(
      "--agedexp --model_count 5 --cb_explore_adf --ae_alpha .01 --quiet  -q ::", empty_hooks);
  float without_save = ctr.back();
  BOOST_CHECK_GT(without_save, 0.8f);

  ctr = simulator::_test_helper_save_load("--agedexp --model_count 5 --cb_explore_adf --ae_alpha .01 --quiet  -q ::");

  float with_save = ctr.back();
  BOOST_CHECK_GT(with_save, 0.8f);

  BOOST_CHECK_CLOSE(without_save, with_save, FLOAT_TOL);
}
