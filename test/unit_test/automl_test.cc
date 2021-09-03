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
#include "test_red.cc"
#include "metric_sink.h"
#include "constant.h"

#include <functional>
#include <map>

using simulator::callback_map;

namespace automl_test
{
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

VW::test_red::tr_data* get_automl_data(vw& all)
{
  std::vector<std::string> e_r;
  all.l->get_enabled_reductions(e_r);
  if (std::find(e_r.begin(), e_r.end(), "test_red") == e_r.end())
  { BOOST_FAIL("test_red not found in enabled reductions"); }

  VW::LEARNER::multi_learner* test_red_learner = as_multiline(all.l->get_learner_by_name_prefix("test_red"));

  return (VW::test_red::tr_data*)test_red_learner->learner_data.get();
}

// see parse_example.cc:maybeFeature(..) for other cases
size_t get_index_for_feature(vw& all, const std::string& ns, const std::string& feature)
{
  std::uint64_t hash_ft = VW::hash_feature(all, feature, VW::hash_space(all, ns));
  std::uint64_t ft = hash_ft & all.parse_mask;
  // apply multiplier like setup_example
  ft *= (static_cast<uint64_t>(all.wpp) << all.weights.stride_shift());

  return ft;
}

// see gd.cc:audit_feature(..)
size_t hash_to_index(parameters& weights, size_t hash)
{
  hash = hash & weights.mask();
  hash = hash >> weights.stride_shift();

  return hash;
}

// craft feature index for interaction
// see: interactions_predict.h:process_quadratic_interaction(..)
size_t interaction_to_index(parameters& weights, size_t one, size_t two)
{
  // FNV_prime from constant.h
  return hash_to_index(weights, (FNV_prime * one) ^ two);
}
}  // namespace ut_helper

using namespace ut_helper;

// 0) runs after 1331 learned examples from simulator.h
// 1) clears (set to zero) offset 1
// 2) copies offset 2 -> offset 1
// n) asserts weights before/after every operation
// NOTE: interactions are currently 0 for offset 0 since
// config 0 is hard-coded to be empty interactions for now.
bool weights_offset_test(vw& all, multi_ex&)
{
  const size_t offset_to_clear = 1;
  auto& weights = all.weights.dense_weights;

  std::vector<std::uint64_t> feature_indexes;
  // hardcoded features that correspond to simulator.h
  feature_indexes.emplace_back(hash_to_index(all.weights, get_index_for_feature(all, "Action", "article=music")));
  feature_indexes.emplace_back(hash_to_index(all.weights, get_index_for_feature(all, "Action", "article=politics")));
  feature_indexes.emplace_back(hash_to_index(all.weights, get_index_for_feature(all, "User", "user=Anna")));

  const size_t interaction_index = interaction_to_index(all.weights,
      get_index_for_feature(all, "Action", "article=sports"), get_index_for_feature(all, "User", "user=Tom"));

  const float expected_w1 = 0.588607371f;
  const float expected_w2 = 0.588550389f;
  const float ZERO = 0.f;

  for (auto index : feature_indexes)
  {
    BOOST_CHECK_NE(ZERO, weights.strided_index(index));
    float w1 = weights.strided_index(index + offset_to_clear);
    float w2 = weights.strided_index(index + offset_to_clear + 1);
    BOOST_CHECK_NE(ZERO, w1);
    BOOST_CHECK_NE(ZERO, w2);
  }
  // this will break once offset 0 stops being hard-coded to empty interactions
  BOOST_CHECK_EQUAL(ZERO, weights.strided_index(interaction_index));
  BOOST_CHECK_CLOSE(expected_w1, weights.strided_index(interaction_index + offset_to_clear), FLOAT_TOL);
  BOOST_CHECK_CLOSE(expected_w2, weights.strided_index(interaction_index + offset_to_clear + 1), FLOAT_TOL);

  // all weights of offset 1 will be set to zero
  weights.clear_offset(offset_to_clear, all.wpp);

  for (auto index : feature_indexes)
  {
    BOOST_CHECK_NE(ZERO, weights.strided_index(index));
    float w1 = weights.strided_index(index + offset_to_clear);
    float w2 = weights.strided_index(index + offset_to_clear + 1);
    BOOST_CHECK_EQUAL(ZERO, w1);
    BOOST_CHECK_NE(ZERO, w2);
    BOOST_CHECK_NE(w1, w2);
  }
  BOOST_CHECK_EQUAL(ZERO, weights.strided_index(interaction_index));
  BOOST_CHECK_EQUAL(ZERO, weights.strided_index(interaction_index + offset_to_clear));
  BOOST_CHECK_CLOSE(expected_w2, weights.strided_index(interaction_index + offset_to_clear + 1), FLOAT_TOL);

  // copy from offset 2 to offset 1
  weights.copy_offsets(offset_to_clear + 1, offset_to_clear, all.wpp);

  for (auto index : feature_indexes)
  {
    BOOST_CHECK_NE(ZERO, weights.strided_index(index));
    float w1 = weights.strided_index(index + offset_to_clear);
    float w2 = weights.strided_index(index + offset_to_clear + 1);
    BOOST_CHECK_NE(ZERO, w1);
    BOOST_CHECK_NE(ZERO, w2);
    BOOST_CHECK_EQUAL(w1, w2);
  }
  BOOST_CHECK_EQUAL(ZERO, weights.strided_index(interaction_index));
  float actual_w1 = weights.strided_index(interaction_index + offset_to_clear);
  float actual_w2 = weights.strided_index(interaction_index + offset_to_clear + 1);
  BOOST_CHECK_CLOSE(expected_w2, actual_w1, FLOAT_TOL);
  BOOST_CHECK_CLOSE(expected_w2, actual_w2, FLOAT_TOL);
  BOOST_CHECK_EQUAL(actual_w1, actual_w2);

  // VW::test_red::helper::print_weights_nonzero(all, 0, all->weights.dense_weights);
  return true;
}
}  // namespace automl_test

using namespace automl_test;

BOOST_AUTO_TEST_CASE(automl_weight_operations)
{
  const size_t seed = 10;
  const size_t num_iterations = 1331;
  callback_map test_hooks;

  // lambda test/assert
  test_hooks.emplace(num_iterations - 1, [&num_iterations](vw& all, multi_ex&) {
    VW::test_red::tr_data* tr = ut_helper::get_automl_data(all);
    BOOST_CHECK_EQUAL(tr->cm.county, num_iterations - 1);
    BOOST_CHECK_GT(tr->cm.max_live_configs, 2);
    BOOST_CHECK_EQUAL(tr->cm.current_state, VW::test_red::config_state::Experimenting);
    return true;
  });

  // fn test/assert
  test_hooks.emplace(num_iterations, weights_offset_test);

  auto ctr = simulator::_test_helper_hook(
      "--test_red 0 --cb_explore_adf --quiet --epsilon 0.2 --random_seed 5", test_hooks, num_iterations, seed);

  BOOST_CHECK_GT(ctr.back(), 0.6f);
}

BOOST_AUTO_TEST_CASE(automl_first_champ_switch)
{
  const size_t num_iterations = 1331;
  const size_t seed = 10;
  const size_t deterministic_champ_switch = 735;
  callback_map test_hooks;

  test_hooks.emplace(deterministic_champ_switch - 1, [&](vw& all, multi_ex&) {
    VW::test_red::tr_data* tr = ut_helper::get_automl_data(all);
    BOOST_CHECK_EQUAL(tr->cm.current_champ, 0);
    BOOST_CHECK_EQUAL(deterministic_champ_switch - 1, tr->cm.county);
    BOOST_CHECK_EQUAL(tr->cm.current_state, VW::test_red::config_state::Experimenting);
    return true;
  });

  test_hooks.emplace(deterministic_champ_switch, [&deterministic_champ_switch](vw& all, multi_ex&) {
    VW::test_red::tr_data* tr = ut_helper::get_automl_data(all);
    BOOST_CHECK_EQUAL(tr->cm.current_champ, 1);
    BOOST_CHECK_EQUAL(deterministic_champ_switch, tr->cm.county);
    BOOST_CHECK_EQUAL(tr->cm.current_state, VW::test_red::config_state::Experimenting);
    return true;
  });

  // we initialize the reduction pointing to position 0 as champ, that config is hard-coded to empty
  auto ctr = simulator::_test_helper_hook(
      "--test_red 0 --cb_explore_adf --quiet --epsilon 0.2 --random_seed 5", test_hooks, num_iterations, seed);

  BOOST_CHECK_GT(ctr.back(), 0.6f);
}

BOOST_AUTO_TEST_CASE(automl_save_load)
{
  callback_map empty_hooks;
  auto ctr =
      simulator::_test_helper_hook("--test_red 0 --cb_explore_adf --quiet --epsilon 0.2 --random_seed 5", empty_hooks);
  float without_save = ctr.back();
  BOOST_CHECK_GT(without_save, 0.7f);

  ctr = simulator::_test_helper_save_load(
      "--test_red 0 --cb_explore_adf --quiet --epsilon 0.2 --random_seed 5 --save_resume");
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
  test_hooks.emplace(zero, [&zero](vw& all, multi_ex&) {
    VW::test_red::tr_data* tr = ut_helper::get_automl_data(all);
    BOOST_CHECK_EQUAL(tr->cm.county, zero);
    BOOST_CHECK_EQUAL(tr->cm.current_state, VW::test_red::config_state::Idle);
    return true;
  });

  // test executes right after learn call of the 10th example
  test_hooks.emplace(num_iterations, [&num_iterations](vw& all, multi_ex&) {
    VW::test_red::tr_data* tr = ut_helper::get_automl_data(all);
    BOOST_CHECK_EQUAL(tr->cm.county, num_iterations);
    BOOST_CHECK_EQUAL(tr->cm.current_state, VW::test_red::config_state::Experimenting);
    return true;
  });

  auto ctr = simulator::_test_helper_hook(
      "--test_red 0 --cb_explore_adf --quiet --epsilon 0.2 --random_seed 5", test_hooks, num_iterations);

  BOOST_CHECK_GT(ctr.back(), 0.1f);
}

BOOST_AUTO_TEST_CASE(assert_0th_event_metrics)
{
  const auto metric_name = std::string("total_learn_calls");
  const size_t zero = 0;
  const size_t num_iterations = 10;
  callback_map test_hooks;

  // technically runs after the 0th example is learned
  test_hooks.emplace(zero, [&metric_name, &zero](vw& all, multi_ex&) {
    VW::metric_sink metrics;
    all.l->persist_metrics(metrics);

    ut_helper::assert_metric(metric_name, zero, metrics);
    return true;
  });

  // test executes right after learn call of the 10th example
  test_hooks.emplace(num_iterations, [&metric_name, &num_iterations](vw& all, multi_ex&) {
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