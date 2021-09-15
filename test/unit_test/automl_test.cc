// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#ifndef STATIC_LINK_VW
#  define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "test_common.h"
#include "simulator.h"
#include "reductions_fwd.h"
#include "automl.h"
#include "metric_sink.h"

#include <functional>
#include <map>

using simulator::callback_map;
using simulator::cb_sim;

constexpr float AUTO_ML_FLOAT_TOL = 0.01f;

namespace ut_helper
{
void assert_metric(const std::string& metric_name, const size_t value, const VW::metric_sink& metrics)
{
  auto it = std::find_if(metrics.int_metrics_list.begin(), metrics.int_metrics_list.end(),
      [&metric_name](const std::pair<std::string, size_t>& element) { return element.first == metric_name; });

  if (it == metrics.int_metrics_list.end()) { BOOST_FAIL("could not find metric. fatal."); }
  else
  {
    BOOST_CHECK_EQUAL(it->second, value);
  }
}

VW::automl::automl* get_automl_data(vw& all)
{
  std::vector<std::string> e_r;
  all.l->get_enabled_reductions(e_r);
  if (std::find(e_r.begin(), e_r.end(), "automl") == e_r.end())
  { BOOST_FAIL("automl not found in enabled reductions"); }

  VW::LEARNER::multi_learner* automl_learner = as_multiline(all.l->get_learner_by_name_prefix("automl"));

  return (VW::automl::automl*)automl_learner->learner_data.get();
}
}  // namespace ut_helper

BOOST_AUTO_TEST_CASE(automl_first_champ_switch)
{
  const size_t num_iterations = 1331;
  const size_t seed = 10;
  const size_t deterministic_champ_switch = 735;
  callback_map test_hooks;

  test_hooks.emplace(deterministic_champ_switch - 1, [&](cb_sim&, vw& all, multi_ex&) {
    VW::automl::automl* aml = ut_helper::get_automl_data(all);
    BOOST_CHECK_EQUAL(aml->cm.current_champ, 0);
    BOOST_CHECK_EQUAL(deterministic_champ_switch - 1, aml->cm.county);
    BOOST_CHECK_EQUAL(aml->cm.current_state, VW::automl::config_state::Experimenting);
    return true;
  });

  test_hooks.emplace(deterministic_champ_switch, [&deterministic_champ_switch](cb_sim&, vw& all, multi_ex&) {
    VW::automl::automl* aml = ut_helper::get_automl_data(all);
    BOOST_CHECK_EQUAL(aml->cm.current_champ, 0);
    BOOST_CHECK_EQUAL(deterministic_champ_switch, aml->cm.county);
    BOOST_CHECK_EQUAL(aml->cm.current_state, VW::automl::config_state::Experimenting);
    return true;
  });

  // we initialize the reduction pointing to position 0 as champ, that config is hard-coded to empty
  auto ctr = simulator::_test_helper_hook(
      "--automl 3 --cb_explore_adf --quiet --epsilon 0.2 --random_seed 5", test_hooks, num_iterations, seed);

  BOOST_CHECK_GT(ctr.back(), 0.4f);
}

// Need to add save_load functionality to multiple structs in automl reduction including
// config_manager and scored_config.
BOOST_AUTO_TEST_CASE(automl_save_load)
{
  callback_map empty_hooks;
  auto ctr =
      simulator::_test_helper_hook("--automl 3 --cb_explore_adf --quiet --epsilon 0.2 --random_seed 5", empty_hooks);
  float without_save = ctr.back();
  BOOST_CHECK_GT(without_save, 0.7f);

  ctr = simulator::_test_helper_save_load(
      "--automl 3 --cb_explore_adf --quiet --epsilon 0.2 --random_seed 5 --save_resume");
  float with_save = ctr.back();
  BOOST_CHECK_GT(with_save, 0.7f);

  BOOST_CHECK_CLOSE(without_save, with_save, FLOAT_TOL);
}

BOOST_AUTO_TEST_CASE(assert_0th_event_automl)
{
  const size_t zero = 0;
  const size_t num_iterations = 10;
  callback_map test_hooks;

  // technically runs after the 0th example is learned
  test_hooks.emplace(zero, [&zero](cb_sim&, vw& all, multi_ex&) {
    VW::automl::automl* aml = ut_helper::get_automl_data(all);
    BOOST_CHECK_EQUAL(aml->cm.county, zero);
    BOOST_CHECK_EQUAL(aml->cm.current_state, VW::automl::config_state::Idle);
    return true;
  });

  // test executes right after learn call of the 10th example
  test_hooks.emplace(num_iterations, [&num_iterations](cb_sim&, vw& all, multi_ex&) {
    VW::automl::automl* aml = ut_helper::get_automl_data(all);
    BOOST_CHECK_EQUAL(aml->cm.county, num_iterations);
    BOOST_CHECK_EQUAL(aml->cm.current_state, VW::automl::config_state::Experimenting);
    return true;
  });

  auto ctr = simulator::_test_helper_hook(
      "--automl 3 --cb_explore_adf --quiet --epsilon 0.2 --random_seed 5", test_hooks, num_iterations);

  BOOST_CHECK_GT(ctr.back(), 0.1f);
}

BOOST_AUTO_TEST_CASE(assert_0th_event_metrics)
{
  const auto metric_name = std::string("total_learn_calls");
  const size_t zero = 0;
  const size_t num_iterations = 10;
  callback_map test_hooks;

  // technically runs after the 0th example is learned
  test_hooks.emplace(zero, [&metric_name, &zero](cb_sim&, vw& all, multi_ex&) {
    VW::metric_sink metrics;
    all.l->persist_metrics(metrics);

    ut_helper::assert_metric(metric_name, zero, metrics);
    return true;
  });

  // test executes right after learn call of the 10th example
  test_hooks.emplace(num_iterations, [&metric_name, &num_iterations](cb_sim&, vw& all, multi_ex&) {
    VW::metric_sink metrics;
    all.l->persist_metrics(metrics);

    ut_helper::assert_metric(metric_name, num_iterations, metrics);
    return true;
  });

  auto ctr = simulator::_test_helper_hook(
      "--extra_metrics ut_metrics.json --cb_explore_adf --quiet --epsilon 0.2 --random_seed 5", test_hooks,
      num_iterations);

  BOOST_CHECK_GT(ctr.back(), 0.1f);
}

BOOST_AUTO_TEST_CASE(assert_live_configs_and_budget)
{
  const size_t fifteen = 15;
  const size_t thirty_three = 33;
  const size_t num_iterations = 100;
  callback_map test_hooks;

  // Note this is after learning 14 examples (first iteration is Idle)
  test_hooks.emplace(fifteen, [&fifteen](cb_sim&, vw& all, multi_ex&) {
    VW::automl::automl* aml = ut_helper::get_automl_data(all);
    BOOST_CHECK_EQUAL(aml->cm.current_state, VW::automl::config_state::Experimenting);
    BOOST_CHECK_EQUAL(aml->cm.county, 15);
    BOOST_CHECK_EQUAL(aml->cm.current_champ, 0);
    BOOST_CHECK_EQUAL(aml->cm.scores[0].config_index, 0);
    BOOST_CHECK_EQUAL(aml->cm.scores[1].config_index, 1);
    BOOST_CHECK_EQUAL(aml->cm.scores[2].config_index, 6);
    BOOST_CHECK_EQUAL(aml->cm.configs.size(), 8);
    BOOST_CHECK_EQUAL(aml->cm.configs[0].budget, 10);
    BOOST_CHECK_EQUAL(aml->cm.configs[7].budget, 20);
    BOOST_CHECK_EQUAL(aml->cm.configs[1].budget, 10);
    BOOST_CHECK_EQUAL(aml->cm.configs[6].budget, 10);
    BOOST_CHECK_EQUAL(aml->cm.scores[0].update_count, 15);
    BOOST_CHECK_EQUAL(aml->cm.scores[1].update_count, 4);
    BOOST_CHECK_EQUAL(aml->cm.scores[2].update_count, 4);
    BOOST_CHECK_EQUAL(aml->cm.index_queue.size(), 3);
    BOOST_CHECK_EQUAL(aml->cm.configs[0].exclusions.size(), 0);
    BOOST_CHECK_EQUAL(aml->cm.configs[7].exclusions.size(), 1);
    BOOST_CHECK_EQUAL(aml->cm.configs[3].exclusions.size(), 1);
    BOOST_CHECK_EQUAL(aml->cm.scores[0].live_interactions.size(), 6);
    BOOST_CHECK_EQUAL(aml->cm.scores[1].live_interactions.size(), 3);
    BOOST_CHECK_EQUAL(aml->cm.scores[2].live_interactions.size(), 1);
    BOOST_CHECK_EQUAL(aml->cm.configs[6].exclusions.size(), 2);
    BOOST_CHECK_EQUAL(aml->cm.configs[6].exclusions['U'].size(), 1);
    BOOST_CHECK_EQUAL(aml->cm.configs[6].exclusions[128].size(), 1);
    BOOST_CHECK_EQUAL(aml->cm.scores[2].live_interactions.size(), 1);
    BOOST_CHECK_EQUAL(aml->cm.scores[2].live_interactions[0][0], 'A');
    BOOST_CHECK_EQUAL(aml->cm.scores[2].live_interactions[0][1], 'A');
    return true;
  });

  auto ctr = simulator::_test_helper_hook(
      "--automl 3 --cb_explore_adf --quiet --epsilon 0.2 --random_seed 5", test_hooks, num_iterations);

  BOOST_CHECK_GT(ctr.back(), 0.1f);
}

// Note higher ctr compared to cpp_simulator_without_interaction in tutorial_test.cc
BOOST_AUTO_TEST_CASE(cpp_simulator_automl)
{
  auto ctr =
      simulator::_test_helper("--cb_explore_adf --quiet --epsilon 0.2 --random_seed 5 --extra_metrics --automl 3");
  BOOST_CHECK_GT(ctr.back(), 0.6f);
}

BOOST_AUTO_TEST_CASE(learner_copy_clear_test)
{
  callback_map test_hooks;

  test_hooks.emplace(10, [&](cb_sim&, vw& all, multi_ex&) {
    all.l->copy_offset_based(0, 1);
    all.l->clear_offset_based(0);

    // no-op for now

    return true;
  });

  auto ctr =
      simulator::_test_helper_hook("--automl 3 --cb_explore_adf --quiet --epsilon 0.2 --random_seed 5", test_hooks, 10);
}

BOOST_AUTO_TEST_CASE(namespace_switch)
{
  const size_t num_iterations = 3000;
  callback_map test_hooks;

  test_hooks.emplace(100, [&](cb_sim& sim, vw& all, multi_ex&) {
    VW::automl::automl* aml = ut_helper::get_automl_data(all);
    auto count_ns_T = aml->cm.ns_counter.count('T');
    BOOST_CHECK_EQUAL(count_ns_T, 0);

    // change user namespace to start with letter T
    sim.user_ns = "Tser";
    return true;
  });

  test_hooks.emplace(101, [&](cb_sim& sim, vw& all, multi_ex&) {
    VW::automl::automl* aml = ut_helper::get_automl_data(all);
    size_t tser_count = aml->cm.ns_counter.at('T');
    BOOST_CHECK_GT(tser_count, 1);

    // reset user namespace to appropriate value
    sim.user_ns = "User";
    return true;
  });

  test_hooks.emplace(num_iterations, [&](cb_sim&, vw& all, multi_ex&) {
    VW::automl::automl* aml = ut_helper::get_automl_data(all);

    auto champ_exclusions = aml->cm.configs[aml->cm.scores[aml->cm.current_champ].config_index].exclusions;
    BOOST_CHECK_EQUAL(champ_exclusions.size(), 1);

    BOOST_CHECK(champ_exclusions.find(static_cast<namespace_index>(128)) != champ_exclusions.end());

    auto champ_interactions = aml->cm.scores[aml->cm.current_champ].live_interactions;
    BOOST_CHECK_EQUAL(champ_interactions.size(), 6);

    return true;
  });

  auto ctr = simulator::_test_helper_hook(
      "--automl 3 --cb_explore_adf --quiet --epsilon 0.2 --random_seed 5 --budget 500", test_hooks, num_iterations);
  BOOST_CHECK_GT(ctr.back(), 0.8f);
}
