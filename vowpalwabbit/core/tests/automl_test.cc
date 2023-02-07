// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/automl.h"

#include "simulator.h"
#include "vw/core/automl_impl.h"
#include "vw/core/estimators/confidence_sequence_robust.h"
#include "vw/core/metric_sink.h"
#include "vw/core/vw_fwd.h"

#include <gtest/gtest.h>

#include <functional>
#include <map>
#include <utility>

using simulator::callback_map;
using simulator::cb_sim;
using namespace VW::reductions::automl;

namespace std
{
template <typename T>
std::ostream& operator<<(std::ostream& o, std::vector<T> const& vec)
{
  o << '[';
  for (const auto& item : vec) { o << ' ' << item; }
  o << ']';
  return o;
}
}  // namespace std

namespace aml_test
{
template <typename T>
void check_interactions_match_exclusions(
    VW::reductions::automl::automl<interaction_config_manager<VW::reductions::automl::config_oracle<T>,
        VW::estimators::confidence_sequence_robust>>* aml)
{
  for (const auto& estimator : aml->cm->estimators)
  {
    EXPECT_EQ(aml->cm->_config_oracle.configs[estimator.first.config_index].conf_type, config_type::Exclusion);
    auto& exclusions = aml->cm->_config_oracle.configs[estimator.first.config_index].elements;
    auto& interactions = estimator.first.live_interactions;
    auto& interaction_type = aml->cm->_config_oracle._interaction_type;
    // Check that no interaction can be found in exclusions
    for (const auto& interaction : interactions)
    {
      if (interaction_type == "quadratic")
      {
        VW::namespace_index ns1 = interaction[0];
        VW::namespace_index ns2 = interaction[1];
        std::vector<VW::namespace_index> ns{ns1, ns2};
        EXPECT_EQ(exclusions.find(ns), exclusions.end());
      }
      else
      {
        VW::namespace_index ns1 = interaction[0];
        VW::namespace_index ns2 = interaction[1];
        VW::namespace_index ns3 = interaction[2];
        std::vector<VW::namespace_index> ns{ns1, ns2, ns3};
        EXPECT_EQ(exclusions.find(ns), exclusions.end());
      }
    }
    // Check that interaction count is equal to quadratic interaction size minus exclusion count
    size_t exclusion_count = exclusions.size();
    size_t quad_inter_count = (aml->cm->ns_counter.size()) * (aml->cm->ns_counter.size() + 1) / 2;
    EXPECT_EQ(interactions.size(), quad_inter_count - exclusion_count);
  }
}

template <typename T>
void check_config_states(
    VW::reductions::automl::automl<interaction_config_manager<VW::reductions::automl::config_oracle<T>,
        VW::estimators::confidence_sequence_robust>>* aml)
{
  // No configs in the index queue should be live
  auto index_queue = aml->cm->_config_oracle.index_queue;
  while (!index_queue.empty())
  {
    auto config_index = index_queue.top().second;
    index_queue.pop();
    EXPECT_NE(aml->cm->_config_oracle.configs[config_index].state, VW::reductions::automl::config_state::Live);
  }

  // All configs in the estimators should be live
  for (const auto& score : aml->cm->estimators)
  {
    EXPECT_EQ(
        aml->cm->_config_oracle.configs[score.first.config_index].state, VW::reductions::automl::config_state::Live);
  }
}

template <typename T>
VW::reductions::automl::automl<
    interaction_config_manager<VW::reductions::automl::config_oracle<T>, VW::estimators::confidence_sequence_robust>>*
get_automl_data(VW::workspace& all)
{
  std::vector<std::string> e_r;
  all.l->get_enabled_learners(e_r);
  if (std::find(e_r.begin(), e_r.end(), "automl") == e_r.end()) { THROW("automl not found in enabled learners"); }

  VW::LEARNER::learner* automl_learner = require_multiline(all.l->get_learner_by_name_prefix("automl"));

  return (VW::reductions::automl::automl<interaction_config_manager<VW::reductions::automl::config_oracle<T>,
          VW::estimators::confidence_sequence_robust>>*)
      automl_learner->get_internal_type_erased_data_pointer_test_use_only();
}
template VW::reductions::automl::automl<
    interaction_config_manager<VW::reductions::automl::config_oracle<VW::reductions::automl::oracle_rand_impl>,
        VW::estimators::confidence_sequence_robust>>*
get_automl_data(VW::workspace& all);
template VW::reductions::automl::automl<
    interaction_config_manager<VW::reductions::automl::config_oracle<VW::reductions::automl::one_diff_impl>,
        VW::estimators::confidence_sequence_robust>>*
get_automl_data(VW::workspace& all);
template VW::reductions::automl::automl<
    interaction_config_manager<VW::reductions::automl::config_oracle<VW::reductions::automl::champdupe_impl>,
        VW::estimators::confidence_sequence_robust>>*
get_automl_data(VW::workspace& all);
template VW::reductions::automl::automl<
    interaction_config_manager<VW::reductions::automl::config_oracle<VW::reductions::automl::one_diff_inclusion_impl>,
        VW::estimators::confidence_sequence_robust>>*
get_automl_data(VW::workspace& all);
template VW::reductions::automl::automl<
    interaction_config_manager<VW::reductions::automl::config_oracle<VW::reductions::automl::qbase_cubic>,
        VW::estimators::confidence_sequence_robust>>*
get_automl_data(VW::workspace& all);

using aml_rand = VW::reductions::automl::automl<
    interaction_config_manager<VW::reductions::automl::config_oracle<VW::reductions::automl::oracle_rand_impl>,
        VW::estimators::confidence_sequence_robust>>;
using aml_onediff = VW::reductions::automl::automl<
    interaction_config_manager<VW::reductions::automl::config_oracle<VW::reductions::automl::one_diff_impl>,
        VW::estimators::confidence_sequence_robust>>;
using aml_onediff_inclusion = VW::reductions::automl::automl<
    interaction_config_manager<VW::reductions::automl::config_oracle<VW::reductions::automl::one_diff_inclusion_impl>,
        VW::estimators::confidence_sequence_robust>>;
using aml_qbase_cubic = VW::reductions::automl::automl<
    interaction_config_manager<VW::reductions::automl::config_oracle<VW::reductions::automl::qbase_cubic>,
        VW::estimators::confidence_sequence_robust>>;
}  // namespace aml_test

// Need to add save_load functionality to multiple structs in automl reduction including
// config_manager and confidence sequence estimator.
TEST(Automl, SaveLoadWIterations)
{
  const size_t num_iterations = 1000;
  const size_t split = 690;
  const size_t seed = 88;
  const std::vector<uint64_t> swap_after = {500};
  callback_map empty_hooks;
  auto ctr_no_save = simulator::_test_helper_hook(
      std::vector<std::string>{"--automl", "3", "--priority_type", "favor_popular_namespaces", "--cb_explore_adf",
          "--quiet", "--epsilon", "0.2", "--fixed_significance_level", "--random_seed", "5", "--default_lease", "10"},
      empty_hooks, num_iterations, seed, swap_after);
  EXPECT_GT(ctr_no_save.back(), 0.6f);

  auto ctr_with_save = simulator::_test_helper_save_load(
      std::vector<std::string>{"--automl", "3", "--priority_type", "favor_popular_namespaces", "--cb_explore_adf",
          "--quiet", "--epsilon", "0.2", "--fixed_significance_level", "--random_seed", "5", "--default_lease", "10"},
      num_iterations, seed, swap_after, split);
  EXPECT_GT(ctr_with_save.back(), 0.6f);

  EXPECT_EQ(ctr_no_save, ctr_with_save);
}

TEST(Automl, Assert0thEventAutomlWIterations)
{
  const size_t zero = 0;
  const size_t num_iterations = 10;
  callback_map test_hooks;

  // technically runs after the 0th example is learned
  test_hooks.emplace(zero,
      [&zero](cb_sim&, VW::workspace& all, VW::multi_ex&)
      {
        aml_test::aml_rand* aml = aml_test::get_automl_data<VW::reductions::automl::oracle_rand_impl>(all);
        EXPECT_EQ(aml->cm->total_learn_count, zero);
        EXPECT_EQ(aml->current_state, VW::reductions::automl::automl_state::Experimenting);
        return true;
      });

  // test executes right after learn call of the 10th example
  test_hooks.emplace(num_iterations,
      [&num_iterations](cb_sim&, VW::workspace& all, VW::multi_ex&)
      {
        aml_test::aml_rand* aml = aml_test::get_automl_data<VW::reductions::automl::oracle_rand_impl>(all);
        EXPECT_EQ(aml->cm->total_learn_count, num_iterations);
        EXPECT_EQ(aml->current_state, VW::reductions::automl::automl_state::Experimenting);
        return true;
      });

  auto ctr = simulator::_test_helper_hook(
      std::vector<std::string>{"--automl", "3", "--priority_type", "favor_popular_namespaces", "--cb_explore_adf",
          "--quiet", "--epsilon", "0.2", "--random_seed", "5", "--oracle_type", "rand", "--default_lease", "10"},
      test_hooks, num_iterations);

  EXPECT_GT(ctr.back(), 0.1f);
}

TEST(Automl, Assert0thEventMetricsWIterations)
{
  const auto metric_name = std::string("total_learn_calls");
  const size_t zero = 0;
  const size_t num_iterations = 10;
  callback_map test_hooks;

  // technically runs after the 0th example is learned
  test_hooks.emplace(zero,
      [&metric_name, &zero](cb_sim&, VW::workspace& all, VW::multi_ex&)
      {
        auto metrics = all.global_metrics.collect_metrics(all.l.get());

        EXPECT_EQ(metrics.get_uint(metric_name), zero);
        return true;
      });

  // test executes right after learn call of the 10th example
  test_hooks.emplace(num_iterations,
      [&metric_name, &num_iterations](cb_sim&, VW::workspace& all, VW::multi_ex&)
      {
        auto metrics = all.global_metrics.collect_metrics(all.l.get());

        EXPECT_EQ(metrics.get_uint(metric_name), num_iterations);
        return true;
      });

  auto ctr =
      simulator::_test_helper_hook(std::vector<std::string>{"--extra_metrics", "ut_metrics.json", "--cb_explore_adf",
                                       "--quiet", "--epsilon", "0.2", "--random_seed", "5", "--default_lease", "10"},
          test_hooks, num_iterations);

  EXPECT_GT(ctr.back(), 0.1f);
}

TEST(Automl, AssertLiveConfigsAndLeaseWIterations)
{
  const size_t fifteen = 15;
  const size_t num_iterations = 100;
  callback_map test_hooks;

  // Note this is after learning 14 examples (first iteration is Collecting)
  test_hooks.emplace(fifteen,
      [](cb_sim&, VW::workspace& all, VW::multi_ex&)
      {
        aml_test::aml_rand* aml = aml_test::get_automl_data<VW::reductions::automl::oracle_rand_impl>(all);
        aml_test::check_interactions_match_exclusions(aml);
        aml_test::check_config_states(aml);
        EXPECT_EQ(aml->current_state, VW::reductions::automl::automl_state::Experimenting);
        EXPECT_EQ(aml->cm->total_learn_count, 15);
        EXPECT_EQ(aml->cm->current_champ, 0);
        EXPECT_FLOAT_EQ(aml->cm->automl_significance_level, 0.05);
        EXPECT_EQ(aml->cm->estimators[0].first.config_index, 0);
        EXPECT_EQ(aml->cm->estimators[1].first.config_index, 3);
        EXPECT_EQ(aml->cm->estimators[2].first.config_index, 1);
        EXPECT_EQ(aml->cm->_config_oracle.configs.size(), 4);
        EXPECT_EQ(aml->cm->_config_oracle.configs[0].lease, 10);
        EXPECT_EQ(aml->cm->_config_oracle.configs[1].lease, 10);
        EXPECT_EQ(aml->cm->_config_oracle.configs[2].lease, 20);
        EXPECT_EQ(aml->cm->_config_oracle.configs[3].lease, 20);
        EXPECT_EQ(aml->cm->estimators[0].first._estimator.update_count, 0);
        EXPECT_EQ(aml->cm->estimators[1].first._estimator.update_count, 15);
        EXPECT_EQ(aml->cm->estimators[2].first._estimator.update_count, 5);
        EXPECT_EQ(aml->cm->estimators[0].second.update_count, 0);
        EXPECT_EQ(aml->cm->estimators[1].second.update_count, 15);
        EXPECT_EQ(aml->cm->estimators[2].second.update_count, 5);
        EXPECT_EQ(aml->cm->_config_oracle.index_queue.size(), 0);
        return true;
      });

  auto ctr =
      simulator::_test_helper_hook(std::vector<std::string>{"--automl=3", "--priority_type", "favor_popular_namespaces",
                                       "--cb_explore_adf", "--quiet", "--epsilon", "0.2", "--fixed_significance_level",
                                       "--random_seed", "5", "--oracle_type", "rand", "--default_lease", "10"},
          test_hooks, num_iterations);

  EXPECT_GT(ctr.back(), 0.1f);
}

// Note higher ctr compared to cpp_simulator_without_interaction in tutorial_test.cc
TEST(Automl, CppSimulatorAutomlWIterations)
{
  auto ctr = simulator::_test_helper(
      std::vector<std::string>{"--cb_explore_adf", "--quiet", "--epsilon", "0.2", "--random_seed", "5", "--automl", "3",
          "--priority_type", "favor_popular_namespaces", "--oracle_type", "rand", "--default_lease", "10"});
  EXPECT_GT(ctr.back(), 0.6f);
}

TEST(Automl, NamespaceSwitchWIterations)
{
  const size_t num_iterations = 1000;
  callback_map test_hooks;
  const std::vector<uint64_t> swap_after = {500};
  const size_t seed = 88;

  test_hooks.emplace(100,
      [&](cb_sim& sim, VW::workspace& all, VW::multi_ex&)
      {
        aml_test::aml_onediff* aml = aml_test::get_automl_data<VW::reductions::automl::one_diff_impl>(all);
        auto count_ns_T = aml->cm->ns_counter.count('T');
        EXPECT_EQ(count_ns_T, 0);

        // change user namespace to start with letter T
        sim.user_ns = "Tser";
        return true;
      });

  test_hooks.emplace(101,
      [&](cb_sim& sim, VW::workspace& all, VW::multi_ex&)
      {
        aml_test::aml_onediff* aml = aml_test::get_automl_data<VW::reductions::automl::one_diff_impl>(all);
        size_t tser_count = aml->cm->ns_counter.at('T');
        EXPECT_GT(tser_count, 1);

        // reset user namespace to appropriate value
        sim.user_ns = "User";
        return true;
      });

  // TODO: Find champ change
  test_hooks.emplace(num_iterations,
      [&](cb_sim&, VW::workspace& all, VW::multi_ex&)
      {
        aml_test::aml_onediff* aml = aml_test::get_automl_data<VW::reductions::automl::one_diff_impl>(all);

        auto champ_exclusions =
            aml->cm->_config_oracle.configs[aml->cm->estimators[aml->cm->current_champ].first.config_index].elements;
        EXPECT_EQ(champ_exclusions.size(), 0);
        auto champ_interactions = aml->cm->estimators[aml->cm->current_champ].first.live_interactions;
        EXPECT_EQ(champ_interactions.size(), 6);
        return true;
      });

  auto ctr = simulator::_test_helper_hook(
      std::vector<std::string>{"--automl", "3", "--priority_type", "favor_popular_namespaces", "--cb_explore_adf",
          "--quiet", "--epsilon", "0.2", "--random_seed", "5", "--default_lease", "500", "--oracle_type", "one_diff",
          "--noconstant"},
      test_hooks, num_iterations, seed, swap_after);
  EXPECT_GT(ctr.back(), 0.65f);
}

TEST(Automl, ClearConfigsWIterations)
{
  const size_t seed = 85;
  const size_t num_iterations = 1000;
  const std::vector<uint64_t> swap_after = {200, 500};
  const size_t clear_champ_switch = 931;
  callback_map test_hooks;

  test_hooks.emplace(clear_champ_switch - 1,
      [&](cb_sim&, VW::workspace& all, VW::multi_ex&)
      {
        aml_test::aml_rand* aml = aml_test::get_automl_data<VW::reductions::automl::oracle_rand_impl>(all);
        aml_test::check_interactions_match_exclusions<VW::reductions::automl::oracle_rand_impl>(aml);
        aml_test::check_config_states(aml);
        EXPECT_EQ(aml->cm->current_champ, 0);
        EXPECT_EQ(aml->cm->_config_oracle.valid_config_size, 4);
        EXPECT_EQ(clear_champ_switch - 1, aml->cm->total_learn_count);
        EXPECT_EQ(aml->cm->estimators[0].first.live_interactions.size(), 3);
        EXPECT_EQ(aml->cm->estimators[1].first.live_interactions.size(), 2);
        EXPECT_EQ(aml->cm->estimators[2].first.live_interactions.size(), 2);
        EXPECT_EQ(aml->current_state, VW::reductions::automl::automl_state::Experimenting);
        return true;
      });

  test_hooks.emplace(clear_champ_switch,
      [](cb_sim&, VW::workspace& all, VW::multi_ex&)
      {
        aml_test::aml_rand* aml = aml_test::get_automl_data<VW::reductions::automl::oracle_rand_impl>(all);
        aml_test::check_interactions_match_exclusions(aml);
        aml_test::check_config_states(aml);
        EXPECT_EQ(aml->cm->current_champ, 0);
        // TODO: Find champ change
        // EXPECT_EQ(clear_champ_switch, aml->cm->total_learn_count);
        // EXPECT_EQ(aml->cm->estimators.size(), 2);
        // EXPECT_EQ(aml->cm->_config_oracle.valid_config_size, 4);
        // EXPECT_EQ(aml->cm->estimators[0].first.live_interactions.size(), 2);
        EXPECT_EQ(aml->current_state, VW::reductions::automl::automl_state::Experimenting);
        return true;
      });

  // we initialize the reduction pointing to position 0 as champ, that config is hard-coded to empty
  auto ctr = simulator::_test_helper_hook(
      std::vector<std::string>{"--automl", "3", "--priority_type", "favor_popular_namespaces", "--cb_explore_adf",
          "--quiet", "--epsilon", "0.2", "--fixed_significance_level", "--random_seed", "5", "--oracle_type", "rand",
          "--default_lease", "500", "--noconstant"},
      test_hooks, num_iterations, seed, swap_after);

  EXPECT_GT(ctr.back(), 0.4f);
}

TEST(Automl, ClearConfigsOneDiffWIterations)
{
  const size_t num_iterations = 1000;
  const std::vector<uint64_t> swap_after = {500};
  const size_t seed = 88;
  const size_t clear_champ_switch = 645;
  callback_map test_hooks;

  test_hooks.emplace(clear_champ_switch - 1,
      [&](cb_sim&, VW::workspace& all, VW::multi_ex&)
      {
        aml_test::aml_onediff* aml = aml_test::get_automl_data<VW::reductions::automl::one_diff_impl>(all);
        aml_test::check_interactions_match_exclusions(aml);
        aml_test::check_config_states(aml);
        EXPECT_EQ(aml->cm->current_champ, 0);
        EXPECT_EQ(aml->cm->_config_oracle.valid_config_size, 4);
        EXPECT_EQ(clear_champ_switch - 1, aml->cm->total_learn_count);
        EXPECT_EQ(aml->current_state, VW::reductions::automl::automl_state::Experimenting);
        return true;
      });

  test_hooks.emplace(clear_champ_switch,
      [&](cb_sim&, VW::workspace& all, VW::multi_ex&)
      {
        aml_test::aml_onediff* aml = aml_test::get_automl_data<VW::reductions::automl::one_diff_impl>(all);
        aml_test::check_interactions_match_exclusions(aml);
        aml_test::check_config_states(aml);
        EXPECT_EQ(aml->cm->current_champ, 0);
        EXPECT_EQ(clear_champ_switch, aml->cm->total_learn_count);
        EXPECT_EQ(aml->cm->estimators.size(), 3);
        EXPECT_EQ(aml->cm->_config_oracle.valid_config_size, 4);
        EXPECT_EQ(aml->current_state, VW::reductions::automl::automl_state::Experimenting);
        return true;
      });

  test_hooks.emplace(clear_champ_switch + 1,
      [](cb_sim&, VW::workspace& all, VW::multi_ex&)
      {
        aml_test::aml_onediff* aml = aml_test::get_automl_data<VW::reductions::automl::one_diff_impl>(all);
        EXPECT_EQ(aml->cm->estimators.size(), 3);
        EXPECT_EQ(aml->cm->estimators[0].first.live_interactions.size(), 3);
        EXPECT_EQ(aml->cm->estimators[1].first.live_interactions.size(), 2);
        EXPECT_EQ(aml->cm->estimators[2].first.live_interactions.size(), 2);
        return true;
      });

  // we initialize the reduction pointing to position 0 as champ, that config is hard-coded to empty
  auto ctr = simulator::_test_helper_hook(
      std::vector<std::string>{"--automl", "3", "--priority_type", "favor_popular_namespaces", "--cb_explore_adf",
          "--quiet", "--epsilon", "0.2", "--fixed_significance_level", "--random_seed", "5", "--noconstant",
          "--default_lease", "10"},
      test_hooks, num_iterations, seed, swap_after);

  EXPECT_GT(ctr.back(), 0.65f);
}

TEST(Automl, QColConsistencyWIterations)
{
  const size_t seed = 88;
  const size_t num_iterations = 1000;

  auto ctr_q_col = simulator::_test_helper(std::vector<std::string>{"--cb_explore_adf", "--quiet", "--epsilon", "0.2",
                                               "--random_seed", "5", "-q", "::", "--default_lease", "10"},
      num_iterations, seed);
  auto ctr_aml = simulator::_test_helper(std::vector<std::string>{"--cb_explore_adf", "--quiet", "--epsilon", "0.2",
                                             "--random_seed", "5", "--automl", "1", "--default_lease", "10"},
      num_iterations, seed);

  EXPECT_FLOAT_EQ(ctr_q_col.back(), ctr_aml.back());
}

TEST(Automl, OneDiffImplUnittestWIterations)
{
  using namespace VW::reductions::automl;

  const size_t num_iterations = 2;
  callback_map test_hooks;
  // const std::vector<uint64_t> swap_after = {500};
  const size_t seed = 88;

  test_hooks.emplace(1,
      [&](cb_sim&, VW::workspace& all, VW::multi_ex&)
      {
        const size_t CHAMP = 0;
        auto* aml = aml_test::get_automl_data<one_diff_impl>(all);

        config_oracle<one_diff_impl>& co = aml->cm->_config_oracle;
        auto rand_state = all.get_random_state();

        std::map<VW::namespace_index, uint64_t> ns_counter;
        std::vector<std::pair<aml_estimator<VW::estimators::confidence_sequence_robust>,
            VW::estimators::confidence_sequence_robust>>
            estimators;

        config_oracle<one_diff_impl> oracle(aml->cm->default_lease, co.calc_priority, co._interaction_type,
            co._oracle_type, rand_state, config_type::Exclusion);

        auto& configs = oracle.configs;
        auto& prio_queue = oracle.index_queue;

        ns_counter['A'] = 1;
        ns_counter['B'] = 1;

        EXPECT_EQ(configs.size(), 0);
        EXPECT_EQ(estimators.size(), 0);
        EXPECT_EQ(prio_queue.size(), 0);
        interaction_config_manager<config_oracle<one_diff_impl>,
            VW::estimators::confidence_sequence_robust>::insert_starting_configuration(estimators, oracle,
            aml->cm->automl_significance_level, 1e-6, false);
        EXPECT_EQ(configs.size(), 1);
        EXPECT_EQ(estimators.size(), 1);
        EXPECT_EQ(prio_queue.size(), 0);
        auto& champ_interactions = estimators[CHAMP].first.live_interactions;

        EXPECT_EQ(champ_interactions.size(), 0);
        auto& exclusions = oracle.configs[estimators[0].first.config_index];
        auto& interactions = estimators[0].first.live_interactions;
        ns_based_config::apply_config_to_interactions(
            false, ns_counter, oracle._interaction_type, exclusions, interactions);
        EXPECT_EQ(champ_interactions.size(), 3);

        const interaction_vec_t expected = {
            {'A', 'A'},
            {'A', 'B'},
            {'B', 'B'},
        };
        EXPECT_EQ(champ_interactions, expected);

        EXPECT_EQ(configs.size(), 1);
        oracle.gen_configs(estimators[CHAMP].first.live_interactions, ns_counter);
        EXPECT_EQ(configs.size(), 4);
        EXPECT_EQ(prio_queue.size(), 3);

        const set_ns_list_t excl_0{};
        EXPECT_EQ(configs[0].elements, excl_0);
        const set_ns_list_t excl_1{{'A', 'A'}};
        EXPECT_EQ(configs[1].elements, excl_1);
        const set_ns_list_t excl_2{{'A', 'B'}};
        EXPECT_EQ(configs[2].elements, excl_2);
        const set_ns_list_t excl_3{{'B', 'B'}};
        EXPECT_EQ(configs[3].elements, excl_3);

        EXPECT_EQ(estimators.size(), 1);
        // add dummy evaluators to simulate that all configs are in play
        for (size_t i = 1; i < configs.size(); ++i)
        {
          interaction_config_manager<config_oracle<one_diff_impl>,
              VW::estimators::confidence_sequence_robust>::apply_config_at_slot(estimators, oracle.configs, i,
              config_oracle<one_diff_impl>::choose(oracle.index_queue), aml->cm->automl_significance_level,
              aml->cm->tol_x, aml->cm->is_brentq, 1);
          auto& temp_exclusions = oracle.configs[estimators[i].first.config_index];
          auto& temp_interactions = estimators[i].first.live_interactions;
          ns_based_config::apply_config_to_interactions(
              false, ns_counter, oracle._interaction_type, temp_exclusions, temp_interactions);
        }
        EXPECT_EQ(prio_queue.size(), 0);
        EXPECT_EQ(estimators.size(), 4);

        // excl_2 is now champ
        interaction_config_manager<config_oracle<one_diff_impl>,
            VW::estimators::confidence_sequence_robust>::apply_new_champ(oracle, 2, estimators, 0, ns_counter);

        EXPECT_EQ(configs[0].elements, excl_2);
        EXPECT_EQ(configs[1].elements, excl_0);

        EXPECT_EQ(oracle.valid_config_size, 4);
        EXPECT_EQ(configs.size(), 4);

        const set_ns_list_t excl_4{{'A', 'A'}, {'A', 'B'}};
        EXPECT_EQ(configs[2].elements, excl_4);
        const set_ns_list_t excl_5{{'A', 'B'}, {'B', 'B'}};
        EXPECT_EQ(configs[3].elements, excl_5);

        // the previous two exclusion configs are now inside the priority queue
        EXPECT_EQ(prio_queue.size(), 2);

        // add dummy evaluators to simulate that all configs are in play
        for (size_t i = 2; i < 4; ++i)
        {
          interaction_config_manager<config_oracle<one_diff_impl>,
              VW::estimators::confidence_sequence_robust>::apply_config_at_slot(estimators, oracle.configs, i,
              config_oracle<one_diff_impl>::choose(oracle.index_queue), aml->cm->automl_significance_level,
              aml->cm->tol_x, aml->cm->is_brentq, 1);
          auto& temp_config = oracle.configs[estimators[i].first.config_index];
          auto& temp_interactions = estimators[i].first.live_interactions;
          ns_based_config::apply_config_to_interactions(
              false, ns_counter, oracle._interaction_type, temp_config, temp_interactions);
        }
        EXPECT_EQ(prio_queue.size(), 0);

        // excl_4 is now champ
        interaction_config_manager<config_oracle<one_diff_impl>,
            VW::estimators::confidence_sequence_robust>::apply_new_champ(oracle, 3, estimators, 0, ns_counter);

        EXPECT_EQ(configs[0].elements, excl_4);
        EXPECT_EQ(configs[1].elements, excl_2);
        EXPECT_EQ(configs[3].elements, excl_1);
        const set_ns_list_t excl_6{{'A', 'A'}, {'A', 'B'}, {'B', 'B'}};
        EXPECT_EQ(configs[2].elements, excl_6);

        EXPECT_EQ(prio_queue.size(), 2);

        return true;
      });

  auto ctr = simulator::_test_helper_hook(
      std::vector<std::string>{"--automl", "3", "--priority_type", "favor_popular_namespaces", "--cb_explore_adf",
          "--quiet", "--epsilon", "0.2", "--random_seed", "5", "--default_lease", "500", "--oracle_type", "one_diff",
          "--noconstant"},
      test_hooks, num_iterations, seed);
}

TEST(Automl, QbaseUnittestWIterations)
{
  using namespace VW::reductions::automl;

  const size_t num_iterations = 2;
  callback_map test_hooks;
  // const std::vector<uint64_t> swap_after = {500};
  const size_t seed = 88;

  test_hooks.emplace(1,
      [&](cb_sim&, VW::workspace& all, VW::multi_ex&)
      {
        const size_t CHAMP = 0;
        auto* aml = aml_test::get_automl_data<qbase_cubic>(all);

        config_oracle<qbase_cubic>& co = aml->cm->_config_oracle;
        auto rand_state = all.get_random_state();

        std::map<VW::namespace_index, uint64_t> ns_counter;
        std::vector<std::pair<aml_estimator<VW::estimators::confidence_sequence_robust>,
            VW::estimators::confidence_sequence_robust>>
            estimators;

        config_oracle<qbase_cubic> oracle(aml->cm->default_lease, co.calc_priority, co._interaction_type,
            co._oracle_type, rand_state, config_type::Interaction);

        auto& configs = oracle.configs;
        auto& prio_queue = oracle.index_queue;

        ns_counter['A'] = 1;
        ns_counter['B'] = 1;
        ns_counter['C'] = 1;

        EXPECT_EQ(configs.size(), 0);
        EXPECT_EQ(estimators.size(), 0);
        EXPECT_EQ(prio_queue.size(), 0);
        interaction_config_manager<config_oracle<qbase_cubic>,
            VW::estimators::confidence_sequence_robust>::insert_starting_configuration(estimators, oracle,
            aml->cm->automl_significance_level, 1e-6, false);
        EXPECT_EQ(configs.size(), 1);
        EXPECT_EQ(estimators.size(), 1);
        EXPECT_EQ(prio_queue.size(), 0);
        auto& champ_interactions = estimators[CHAMP].first.live_interactions;

        EXPECT_EQ(champ_interactions.size(), 0);
        auto& exclusions = oracle.configs[estimators[0].first.config_index];
        auto& interactions = estimators[0].first.live_interactions;
        ns_based_config::apply_config_to_interactions(
            false, ns_counter, oracle._interaction_type, exclusions, interactions);
        EXPECT_EQ(champ_interactions.size(), 6);

        const interaction_vec_t expected = {
            {'A', 'A'},
            {'A', 'B'},
            {'A', 'C'},
            {'B', 'B'},
            {'B', 'C'},
            {'C', 'C'},
        };
        EXPECT_EQ(champ_interactions, expected);

        EXPECT_EQ(configs.size(), 1);
        oracle.gen_configs(estimators[CHAMP].first.live_interactions, ns_counter);
        EXPECT_EQ(configs.size(), 11);
        EXPECT_EQ(prio_queue.size(), 10);

        const set_ns_list_t excl_0{};
        EXPECT_EQ(configs[0].elements, excl_0);
        const set_ns_list_t excl_1{{'A', 'A', 'A'}};
        EXPECT_EQ(configs[1].elements, excl_1);
        const set_ns_list_t excl_2{{'A', 'B', 'C'}};
        EXPECT_EQ(configs[2].elements, excl_2);
        const set_ns_list_t excl_3{{'A', 'A', 'C'}};
        EXPECT_EQ(configs[3].elements, excl_3);
        const set_ns_list_t excl_9{{'B', 'C', 'C'}};
        EXPECT_EQ(configs[9].elements, excl_9);

        EXPECT_EQ(estimators.size(), 1);
        // add dummy evaluators to simulate that all configs are in play
        for (size_t i = 1; i < configs.size(); ++i)
        {
          interaction_config_manager<config_oracle<qbase_cubic>,
              VW::estimators::confidence_sequence_robust>::apply_config_at_slot(estimators, oracle.configs, i,
              config_oracle<qbase_cubic>::choose(oracle.index_queue), aml->cm->automl_significance_level,
              aml->cm->tol_x, aml->cm->is_brentq, 1);
          auto& temp_exclusions = oracle.configs[estimators[i].first.config_index];
          auto& temp_interactions = estimators[i].first.live_interactions;
          ns_based_config::apply_config_to_interactions(
              false, ns_counter, oracle._interaction_type, temp_exclusions, temp_interactions);
        }
        EXPECT_EQ(prio_queue.size(), 0);
        EXPECT_EQ(estimators.size(), 11);
        const interaction_vec_t expected2 = {
            {'B', 'C', 'C'},
            {'A', 'A'},
            {'A', 'B'},
            {'A', 'C'},
            {'B', 'B'},
            {'B', 'C'},
            {'C', 'C'},
        };
        EXPECT_EQ(estimators[2].first.config_index, 9);
        EXPECT_EQ(estimators[2].first.live_interactions, expected2);

        // excl_9 - config 2 is now champ
        interaction_config_manager<config_oracle<qbase_cubic>,
            VW::estimators::confidence_sequence_robust>::apply_new_champ(oracle, 2, estimators, 0, ns_counter);

        EXPECT_EQ(configs[0].elements, excl_9);
        EXPECT_EQ(configs[1].elements, excl_0);

        EXPECT_EQ(oracle.valid_config_size, 11);
        EXPECT_EQ(configs.size(), 11);

        const set_ns_list_t excl_4{{'A', 'A', 'A'}, {'B', 'C', 'C'}};
        EXPECT_EQ(configs[2].elements, excl_4);
        const set_ns_list_t excl_5{{'B', 'B', 'C'}, {'B', 'C', 'C'}};
        EXPECT_EQ(configs[3].elements, excl_5);

        // the previous two exclusion configs are now inside the priority queue
        EXPECT_EQ(prio_queue.size(), 9);

        // add dummy evaluators to simulate that all configs are in play
        for (size_t i = 2; i < 4; ++i)
        {
          interaction_config_manager<config_oracle<qbase_cubic>,
              VW::estimators::confidence_sequence_robust>::apply_config_at_slot(estimators, oracle.configs, i,
              config_oracle<qbase_cubic>::choose(oracle.index_queue), aml->cm->automl_significance_level,
              aml->cm->tol_x, aml->cm->is_brentq, 1);
          auto& temp_config = oracle.configs[estimators[i].first.config_index];
          auto& temp_interactions = estimators[i].first.live_interactions;
          ns_based_config::apply_config_to_interactions(
              false, ns_counter, oracle._interaction_type, temp_config, temp_interactions);
        }
        EXPECT_EQ(prio_queue.size(), 7);

        // excl_7 is now champ
        const set_ns_list_t excl_7{{'A', 'B', 'C'}, {'B', 'C', 'C'}};
        interaction_config_manager<config_oracle<qbase_cubic>,
            VW::estimators::confidence_sequence_robust>::apply_new_champ(oracle, 3, estimators, 0, ns_counter);

        EXPECT_EQ(configs[0].elements, excl_7);
        EXPECT_EQ(configs[1].elements, excl_9);
        const set_ns_list_t excl_6{{'A', 'B', 'C'}, {'B', 'C', 'C'}, {'C', 'C', 'C'}};
        EXPECT_EQ(configs[2].elements, excl_6);

        EXPECT_EQ(prio_queue.size(), 9);

        return true;
      });

  auto ctr = simulator::_test_helper_hook(
      std::vector<std::string>{"--automl", "3", "--priority_type", "favor_popular_namespaces", "--cb_explore_adf",
          "--quiet", "--epsilon", "0.2", "--random_seed", "5", "--default_lease", "500", "--oracle_type", "qbase_cubic",
          "--noconstant"},
      test_hooks, num_iterations, seed);
}

TEST(Automl, ExcInclUnitTest)
{
  using namespace VW::reductions::automl;

  std::map<VW::namespace_index, uint64_t> ns_counter{{'A', 5}, {'B', 4}, {'C', 3}};

  interaction_vec_t interactions;

  ns_based_config test_config_interaction(set_ns_list_t{{'A', 'A'}, {'A', 'B'}}, 4000, config_type::Interaction);
  ns_based_config::apply_config_to_interactions(false, ns_counter, "", test_config_interaction, interactions);

  const interaction_vec_t expected{{'A', 'A'}, {'A', 'B'}};
  EXPECT_EQ(interactions.size(), 2);
  EXPECT_EQ(interactions, expected);

  ns_based_config test_config_exclusion(set_ns_list_t{{'A', 'A'}, {'A', 'B'}}, 4000, config_type::Exclusion);
  ns_based_config::apply_config_to_interactions(false, ns_counter, "quadratic", test_config_exclusion, interactions);

  const interaction_vec_t expected2{{'A', 'C'}, {'B', 'B'}, {'B', 'C'}, {'C', 'C'}};
  EXPECT_EQ(interactions.size(), 4);
  EXPECT_EQ(interactions, expected2);
}

TEST(Automl, QuadcubicUnitTest)
{
  using namespace VW::reductions::automl;

  std::map<VW::namespace_index, uint64_t> ns_counter{{'A', 5}, {'B', 4}, {'C', 3}};

  interaction_vec_t interactions;

  ns_based_config test_config_exclusion(
      set_ns_list_t{{'A', 'A', 'A'}, {'B', 'B', 'B'}, {'C', 'C', 'C'}}, 4000, config_type::Interaction);
  ns_based_config::apply_config_to_interactions(false, ns_counter, "both", test_config_exclusion, interactions);

  const interaction_vec_t expected2{{'A', 'A', 'A'}, {'B', 'B', 'B'}, {'C', 'C', 'C'}, {'A', 'A'}, {'A', 'B'},
      {'A', 'C'}, {'B', 'B'}, {'B', 'C'}, {'C', 'C'}};

  EXPECT_EQ(interactions, expected2);
}

TEST(Automl, InsertionChampChangeWIterations)
{
  const size_t seed = 85;
  const size_t num_iterations = 4136;
  const std::vector<uint64_t> swap_after = {200, 500};
  const size_t clear_champ_switch = 4131;
  callback_map test_hooks;

  test_hooks.emplace(clear_champ_switch - 1,
      [&](cb_sim&, VW::workspace& all, VW::multi_ex&)
      {
        auto* aml = aml_test::get_automl_data<VW::reductions::automl::one_diff_inclusion_impl>(all);
        aml_test::check_config_states(aml);
        EXPECT_EQ(aml->cm->current_champ, 0);
        EXPECT_EQ(aml->cm->_config_oracle.valid_config_size, 4);
        EXPECT_EQ(clear_champ_switch - 1, aml->cm->total_learn_count);
        EXPECT_EQ(aml->cm->estimators[0].first.live_interactions.size(), 1);
        EXPECT_EQ(aml->cm->estimators[1].first.live_interactions.size(), 0);
        EXPECT_EQ(aml->cm->estimators[2].first.live_interactions.size(), 2);
        EXPECT_EQ(aml->cm->_config_oracle.configs.size(), 4);
        EXPECT_EQ(aml->cm->_config_oracle.configs[0].elements.size(), 1);
        EXPECT_EQ(aml->cm->_config_oracle.configs[1].elements.size(), 0);
        EXPECT_EQ(aml->cm->_config_oracle.configs[2].elements.size(), 2);
        EXPECT_EQ(aml->cm->_config_oracle.configs[3].elements.size(), 2);
        EXPECT_EQ(aml->cm->total_champ_switches, 1);
        EXPECT_EQ(aml->current_state, VW::reductions::automl::automl_state::Experimenting);
        return true;
      });

  test_hooks.emplace(clear_champ_switch,
      [&](cb_sim&, VW::workspace& all, VW::multi_ex&)
      {
        auto* aml = aml_test::get_automl_data<VW::reductions::automl::one_diff_inclusion_impl>(all);
        aml_test::check_config_states(aml);
        EXPECT_EQ(aml->cm->current_champ, 0);
        EXPECT_EQ(clear_champ_switch, aml->cm->total_learn_count);
        EXPECT_EQ(aml->cm->estimators.size(), 3);
        EXPECT_EQ(aml->cm->_config_oracle.valid_config_size, 4);
        EXPECT_EQ(aml->cm->estimators[0].first.live_interactions.size(), 1);
        EXPECT_EQ(aml->cm->estimators[1].first.live_interactions.size(), 0);
        EXPECT_EQ(aml->cm->_config_oracle.configs.size(), 4);
        EXPECT_EQ(aml->cm->_config_oracle.configs[0].elements.size(), 1);
        EXPECT_EQ(aml->cm->_config_oracle.configs[1].elements.size(), 0);
        EXPECT_EQ(aml->cm->_config_oracle.configs[2].elements.size(), 2);
        EXPECT_EQ(aml->cm->_config_oracle.configs[3].elements.size(), 2);
        EXPECT_EQ(aml->cm->total_champ_switches, 1);
        EXPECT_EQ(aml->current_state, VW::reductions::automl::automl_state::Experimenting);
        return true;
      });

  test_hooks.emplace(clear_champ_switch + 1,
      [](cb_sim&, VW::workspace& all, VW::multi_ex&)
      {
        auto* aml = aml_test::get_automl_data<VW::reductions::automl::one_diff_inclusion_impl>(all);
        aml_test::check_config_states(aml);
        EXPECT_EQ(aml->cm->estimators.size(), 3);
        EXPECT_EQ(aml->cm->estimators[0].first.live_interactions.size(), 1);
        EXPECT_EQ(aml->cm->estimators[1].first.live_interactions.size(), 0);
        EXPECT_EQ(aml->cm->estimators[2].first.live_interactions.size(), 2);
        return true;
      });

  // we initialize the reduction pointing to position 0 as champ, that config is hard-coded to empty
  auto ctr = simulator::_test_helper_hook(
      std::vector<std::string>{"--automl", "3", "--priority_type", "favor_popular_namespaces", "--cb_explore_adf",
          "--quiet", "--epsilon", "0.2", "--fixed_significance_level", "--random_seed", "5", "--oracle_type",
          "one_diff_inclusion", "--default_lease", "500", "--noconstant"},
      test_hooks, num_iterations, seed, swap_after);

  EXPECT_GT(ctr.back(), 0.4f);
}

TEST(Automl, InsertionChampChangeBagWIterations)
{
  const size_t seed = 85;
  const size_t num_iterations = 940;
  const std::vector<uint64_t> swap_after = {200, 500};
  const size_t clear_champ_switch = 936;
  callback_map test_hooks;

  test_hooks.emplace(clear_champ_switch - 1,
      [&](cb_sim&, VW::workspace& all, VW::multi_ex&)
      {
        auto* aml = aml_test::get_automl_data<VW::reductions::automl::one_diff_inclusion_impl>(all);
        aml_test::check_config_states(aml);
        EXPECT_EQ(aml->cm->current_champ, 0);
        EXPECT_EQ(aml->cm->_config_oracle.valid_config_size, 4);
        EXPECT_EQ(aml->cm->estimators[0].first.live_interactions.size(), 1);
        EXPECT_EQ(aml->cm->estimators[1].first.live_interactions.size(), 0);
        EXPECT_EQ(aml->cm->estimators[2].first.live_interactions.size(), 2);
        EXPECT_EQ(aml->cm->estimators[3].first.live_interactions.size(), 2);
        EXPECT_EQ(aml->cm->_config_oracle.configs.size(), 4);
        EXPECT_EQ(aml->cm->_config_oracle.configs[0].elements.size(), 1);
        EXPECT_EQ(aml->cm->_config_oracle.configs[1].elements.size(), 0);
        EXPECT_EQ(aml->cm->_config_oracle.configs[2].elements.size(), 2);
        EXPECT_EQ(aml->cm->_config_oracle.configs[3].elements.size(), 2);
        EXPECT_EQ(aml->cm->total_champ_switches, 1);
        EXPECT_EQ(aml->current_state, VW::reductions::automl::automl_state::Experimenting);
        return true;
      });

  test_hooks.emplace(clear_champ_switch,
      [&](cb_sim&, VW::workspace& all, VW::multi_ex&)
      {
        auto* aml = aml_test::get_automl_data<VW::reductions::automl::one_diff_inclusion_impl>(all);
        aml_test::check_config_states(aml);
        EXPECT_EQ(aml->cm->current_champ, 0);
        EXPECT_EQ(aml->cm->estimators.size(), 4);
        EXPECT_EQ(aml->cm->_config_oracle.valid_config_size, 4);
        EXPECT_EQ(aml->cm->estimators[0].first.live_interactions.size(), 1);
        EXPECT_EQ(aml->cm->estimators[1].first.live_interactions.size(), 0);
        EXPECT_EQ(aml->cm->_config_oracle.configs.size(), 4);
        EXPECT_EQ(aml->cm->_config_oracle.configs[0].elements.size(), 1);
        EXPECT_EQ(aml->cm->_config_oracle.configs[1].elements.size(), 0);
        EXPECT_EQ(aml->cm->_config_oracle.configs[2].elements.size(), 2);
        EXPECT_EQ(aml->cm->_config_oracle.configs[3].elements.size(), 2);
        EXPECT_EQ(aml->cm->total_champ_switches, 1);
        EXPECT_EQ(aml->current_state, VW::reductions::automl::automl_state::Experimenting);
        return true;
      });

  test_hooks.emplace(clear_champ_switch + 1,
      [](cb_sim&, VW::workspace& all, VW::multi_ex&)
      {
        auto* aml = aml_test::get_automl_data<VW::reductions::automl::one_diff_inclusion_impl>(all);
        aml_test::check_config_states(aml);
        EXPECT_EQ(aml->cm->estimators.size(), 4);
        EXPECT_EQ(aml->cm->estimators[0].first.live_interactions.size(), 1);
        EXPECT_EQ(aml->cm->estimators[1].first.live_interactions.size(), 0);
        EXPECT_EQ(aml->cm->estimators[2].first.live_interactions.size(), 2);
        EXPECT_EQ(aml->cm->estimators[3].first.live_interactions.size(), 2);
        return true;
      });

  // we initialize the reduction pointing to position 0 as champ, that config is hard-coded to empty
  auto ctr = simulator::_test_helper_hook(
      std::vector<std::string>{"--automl", "4", "--priority_type", "favor_popular_namespaces", "--cb_explore_adf",
          "--quiet", "--epsilon", "0.2", "--fixed_significance_level", "--random_seed", "5", "--oracle_type",
          "one_diff_inclusion", "--default_lease", "500", "--noconstant", "--bag", "2"},
      test_hooks, num_iterations, seed, swap_after);

  EXPECT_GT(ctr.back(), 0.4f);
}