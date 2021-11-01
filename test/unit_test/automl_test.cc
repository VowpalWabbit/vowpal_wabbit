// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <boost/test/tools/old/interface.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "test_common.h"
#include "simulator.h"
#include "reductions_fwd.h"
#include "automl.h"
#include "metric_sink.h"

#include <functional>
#include <map>
#include <utility>

using simulator::callback_map;
using simulator::cb_sim;
using namespace VW::automl;

namespace aml_test
{

void check_interactions_match_exclusions(VW::automl::automl<interaction_config_manager>* aml)
{
  for (const auto& score : aml->cm->scores)
  {
    auto& exclusions = aml->cm->configs[score.config_index].exclusions;
    auto& interactions = score.live_interactions;
    // Check that no interaction can be found in exclusions
    for (const auto& interaction : interactions)
    {
      namespace_index ns1 = interaction[0];
      namespace_index ns2 = interaction[1];
      BOOST_CHECK(exclusions.find(ns1) == exclusions.end() || exclusions.at(ns1).find(ns2) == exclusions.at(ns1).end());
    }
    // Check that interaction count is equal to quadratic interaction size minus exclusion count
    size_t exclusion_count = 0;
    for (const auto& exclusion : exclusions) { exclusion_count += exclusion.second.size(); }
    size_t quad_inter_count = (aml->cm->ns_counter.size()) * (aml->cm->ns_counter.size() + 1) / 2;
    BOOST_CHECK_EQUAL(interactions.size(), quad_inter_count - exclusion_count);
  }
}

void check_config_states(VW::automl::automl<interaction_config_manager>* aml)
{
  // No configs in the index queue should be live
  auto index_queue = aml->cm->index_queue;
  while (!index_queue.empty())
  {
    auto& config_index = index_queue.top().second;
    index_queue.pop();
    BOOST_CHECK(aml->cm->configs[config_index].state != VW::automl::config_state::Live);
  }

  // All configs in the scores should be live
  for (const auto& score : aml->cm->scores)
  { BOOST_CHECK(aml->cm->configs[score.config_index].state == VW::automl::config_state::Live); }
}

VW::automl::automl<interaction_config_manager>* get_automl_data(vw& all)
{
  std::vector<std::string> e_r;
  all.l->get_enabled_reductions(e_r);
  if (std::find(e_r.begin(), e_r.end(), "automl") == e_r.end())
  { BOOST_FAIL("automl not found in enabled reductions"); }

  VW::LEARNER::multi_learner* automl_learner = as_multiline(all.l->get_learner_by_name_prefix("automl"));

  return (VW::automl::automl<interaction_config_manager>*)
      automl_learner->get_internal_type_erased_data_pointer_test_use_only();
}
}  // namespace aml_test

BOOST_AUTO_TEST_CASE(automl_first_champ_switch)
{
  const size_t num_iterations = 1331;
  const size_t seed = 10;
  const size_t deterministic_champ_switch = 735;
  callback_map test_hooks;

  test_hooks.emplace(deterministic_champ_switch - 1, [&](cb_sim&, vw& all, multi_ex&) {
    VW::automl::automl<interaction_config_manager>* aml = aml_test::get_automl_data(all);
    aml_test::check_interactions_match_exclusions(aml);
    aml_test::check_config_states(aml);
    BOOST_CHECK_EQUAL(aml->cm->current_champ, 2);
    BOOST_CHECK_EQUAL(deterministic_champ_switch - 1, aml->cm->total_learn_count);
    BOOST_CHECK(aml->current_state == VW::automl::automl_state::Experimenting);
    return true;
  });

  test_hooks.emplace(deterministic_champ_switch, [&deterministic_champ_switch](cb_sim&, vw& all, multi_ex&) {
    VW::automl::automl<interaction_config_manager>* aml = aml_test::get_automl_data(all);
    aml_test::check_interactions_match_exclusions(aml);
    aml_test::check_config_states(aml);
    BOOST_CHECK_EQUAL(aml->cm->current_champ, 2);
    BOOST_CHECK_EQUAL(deterministic_champ_switch, aml->cm->total_learn_count);
    BOOST_CHECK(aml->current_state == VW::automl::automl_state::Experimenting);
    return true;
  });

  // we initialize the reduction pointing to position 0 as champ, that config is hard-coded to empty
  auto ctr = simulator::_test_helper_hook(
      "--automl 3 --priority_type least_exclusion --cb_explore_adf --quiet --epsilon 0.2 --random_seed 5 --keep_configs", test_hooks,
      num_iterations, seed);

  BOOST_CHECK_GT(ctr.back(), 0.4f);
}

// Need to add save_load functionality to multiple structs in automl reduction including
// config_manager and scored_config.
BOOST_AUTO_TEST_CASE(automl_save_load)
{
  callback_map empty_hooks;
  auto ctr = simulator::_test_helper_hook(
      "--automl 3 --priority_type least_exclusion --cb_explore_adf --quiet --epsilon 0.2 --random_seed 5 --keep_configs", empty_hooks);
  float without_save = ctr.back();
  BOOST_CHECK_GT(without_save, 0.7f);

  ctr = simulator::_test_helper_save_load(
      "--automl 3 --priority_type least_exclusion --cb_explore_adf --quiet --epsilon 0.2 --random_seed 5 --keep_configs "
      "--save_resume");
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
    VW::automl::automl<interaction_config_manager>* aml = aml_test::get_automl_data(all);
    BOOST_CHECK_EQUAL(aml->cm->total_learn_count, zero);
    BOOST_CHECK(aml->current_state == VW::automl::automl_state::Collecting);
    return true;
  });

  // test executes right after learn call of the 10th example
  test_hooks.emplace(num_iterations, [&num_iterations](cb_sim&, vw& all, multi_ex&) {
    VW::automl::automl<interaction_config_manager>* aml = aml_test::get_automl_data(all);
    BOOST_CHECK_EQUAL(aml->cm->total_learn_count, num_iterations);
    BOOST_CHECK(aml->current_state == VW::automl::automl_state::Experimenting);
    return true;
  });

  auto ctr = simulator::_test_helper_hook(
      "--automl 3 --priority_type least_exclusion --cb_explore_adf --quiet --epsilon 0.2 --random_seed 5 --keep_configs", test_hooks,
      num_iterations);

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

    BOOST_REQUIRE_EQUAL(metrics.get_uint(metric_name), zero);
    return true;
  });

  // test executes right after learn call of the 10th example
  test_hooks.emplace(num_iterations, [&metric_name, &num_iterations](cb_sim&, vw& all, multi_ex&) {
    VW::metric_sink metrics;
    all.l->persist_metrics(metrics);

    BOOST_REQUIRE_EQUAL(metrics.get_uint(metric_name), num_iterations);
    return true;
  });

  auto ctr = simulator::_test_helper_hook(
      "--extra_metrics ut_metrics.json --cb_explore_adf --quiet --epsilon 0.2 --random_seed 5", test_hooks,
      num_iterations);

  BOOST_CHECK_GT(ctr.back(), 0.1f);
}

BOOST_AUTO_TEST_CASE(assert_live_configs_and_lease)
{
  const size_t fifteen = 15;
  const size_t thirty_three = 33;
  const size_t num_iterations = 100;
  callback_map test_hooks;

  // Note this is after learning 14 examples (first iteration is Collecting)
  test_hooks.emplace(fifteen, [&fifteen](cb_sim&, vw& all, multi_ex&) {
    VW::automl::automl<interaction_config_manager>* aml = aml_test::get_automl_data(all);
    aml_test::check_interactions_match_exclusions(aml);
    aml_test::check_config_states(aml);
    BOOST_CHECK(aml->current_state == VW::automl::automl_state::Experimenting);
    BOOST_CHECK_EQUAL(aml->cm->total_learn_count, 15);
    BOOST_CHECK_EQUAL(aml->cm->current_champ, 0);
    BOOST_CHECK_EQUAL(aml->cm->scores[0].config_index, 0);
    BOOST_CHECK_EQUAL(aml->cm->scores[1].config_index, 5);
    BOOST_CHECK_EQUAL(aml->cm->scores[2].config_index, 3);
    BOOST_CHECK_EQUAL(aml->cm->configs.size(), 6);
    BOOST_CHECK_EQUAL(aml->cm->configs[0].lease, 20);
    BOOST_CHECK_EQUAL(aml->cm->configs[3].lease, 10);
    BOOST_CHECK_EQUAL(aml->cm->configs[2].lease, 10);
    BOOST_CHECK_EQUAL(aml->cm->configs[4].lease, 20);
    BOOST_CHECK_EQUAL(aml->cm->scores[0].update_count, 15);
    BOOST_CHECK_EQUAL(aml->cm->scores[1].update_count, 14);
    BOOST_CHECK_EQUAL(aml->cm->scores[2].update_count, 4);
    BOOST_CHECK_EQUAL(aml->cm->index_queue.size(), 2);
    return true;
  });

  auto ctr = simulator::_test_helper_hook(
      "--automl 3 --priority_type least_exclusion --cb_explore_adf --quiet --epsilon 0.2 --random_seed 5 --keep_configs", test_hooks,
      num_iterations);

  BOOST_CHECK_GT(ctr.back(), 0.1f);
}

// Note higher ctr compared to cpp_simulator_without_interaction in tutorial_test.cc
BOOST_AUTO_TEST_CASE(cpp_simulator_automl)
{
  auto ctr = simulator::_test_helper(
      "--cb_explore_adf --quiet --epsilon 0.2 --random_seed 5 --extra_metrics --automl 3 --priority_type "
      "least_exclusion --keep_configs");
  BOOST_CHECK_GT(ctr.back(), 0.6f);
}

BOOST_AUTO_TEST_CASE(namespace_switch)
{
  const size_t num_iterations = 3000;
  callback_map test_hooks;

  test_hooks.emplace(100, [&](cb_sim& sim, vw& all, multi_ex&) {
    VW::automl::automl<interaction_config_manager>* aml = aml_test::get_automl_data(all);
    auto count_ns_T = aml->cm->ns_counter.count('T');
    BOOST_CHECK_EQUAL(count_ns_T, 0);

    // change user namespace to start with letter T
    sim.user_ns = "Tser";
    return true;
  });

  test_hooks.emplace(101, [&](cb_sim& sim, vw& all, multi_ex&) {
    VW::automl::automl<interaction_config_manager>* aml = aml_test::get_automl_data(all);
    size_t tser_count = aml->cm->ns_counter.at('T');
    BOOST_CHECK_GT(tser_count, 1);

    // reset user namespace to appropriate value
    sim.user_ns = "User";
    return true;
  });

  test_hooks.emplace(num_iterations, [&](cb_sim&, vw& all, multi_ex&) {
    VW::automl::automl<interaction_config_manager>* aml = aml_test::get_automl_data(all);

    auto champ_exclusions = aml->cm->configs[aml->cm->scores[aml->cm->current_champ].config_index].exclusions;
    BOOST_CHECK_EQUAL(champ_exclusions.size(), 1);

    BOOST_CHECK(champ_exclusions.find('U') != champ_exclusions.end());

    auto champ_interactions = aml->cm->scores[aml->cm->current_champ].live_interactions;
    BOOST_CHECK_EQUAL(champ_interactions.size(), 9);

    return true;
  });

  auto ctr = simulator::_test_helper_hook(
      "--automl 3 --priority_type least_exclusion --cb_explore_adf --quiet --epsilon 0.2 --random_seed 5 "
      "--global_lease 500 --keep_configs",
      test_hooks, num_iterations);
  BOOST_CHECK_GT(ctr.back(), 0.8f);
}

BOOST_AUTO_TEST_CASE(clear_configs)
{
  const size_t num_iterations = 3000;
  const size_t seed = 10;
  const size_t clear_champ_switch = 150;
  callback_map test_hooks;

  test_hooks.emplace(clear_champ_switch - 1, [&](cb_sim&, vw& all, multi_ex&) {
    VW::automl::automl<interaction_config_manager>* aml = aml_test::get_automl_data(all);
    aml_test::check_interactions_match_exclusions(aml);
    aml_test::check_config_states(aml);
    BOOST_CHECK_EQUAL(aml->cm->current_champ, 0);
    BOOST_CHECK_EQUAL(clear_champ_switch - 1, aml->cm->total_learn_count);
    BOOST_CHECK(aml->current_state == VW::automl::automl_state::Experimenting);
    return true;
  });

  test_hooks.emplace(clear_champ_switch, [&clear_champ_switch](cb_sim&, vw& all, multi_ex&) {
    VW::automl::automl<interaction_config_manager>* aml = aml_test::get_automl_data(all);
    aml_test::check_interactions_match_exclusions(aml);
    aml_test::check_config_states(aml);
    BOOST_CHECK_EQUAL(aml->cm->current_champ, 0);
    BOOST_CHECK_EQUAL(clear_champ_switch, aml->cm->total_learn_count);
    BOOST_CHECK_EQUAL(aml->cm->scores.size(), 1);
    BOOST_CHECK_EQUAL(aml->cm->config_size, 6);
    BOOST_CHECK(aml->current_state == VW::automl::automl_state::Experimenting);
    return true;
  });

  // we initialize the reduction pointing to position 0 as champ, that config is hard-coded to empty
  auto ctr = simulator::_test_helper_hook(
      "--automl 3 --priority_type least_exclusion --cb_explore_adf --quiet --epsilon 0.2 --random_seed 5", test_hooks,
      num_iterations, seed);

  BOOST_CHECK_GT(ctr.back(), 0.4f);
}