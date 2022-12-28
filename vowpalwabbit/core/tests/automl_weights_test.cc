// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "simulator.h"
#include "vw/config/options.h"
#include "vw/core/array_parameters_dense.h"
#include "vw/core/constant.h"  // FNV_PRIME
#include "vw/core/learner.h"
#include "vw/core/multi_model_utils.h"
#include "vw/core/vw_math.h"

#include <gtest/gtest.h>

#include <functional>
#include <map>

using namespace VW::config;

using simulator::callback_map;
using simulator::cb_sim;

constexpr float AUTO_ML_FLOAT_TOL = 0.001f;

namespace vw_hash_helpers
{
// see parse_example.cc:maybeFeature(..) for other cases
size_t get_hash_for_feature(VW::workspace& all, const std::string& ns, const std::string& feature)
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
  // FNV_PRIME from constant.h
  return hash_to_index(weights, (VW::details::FNV_PRIME * one) ^ two);
}
}  // namespace vw_hash_helpers

using namespace vw_hash_helpers;

// 0) runs after 1331 learned examples from simulator.h
// 1) clears (set to zero) offset 1
// 2) copies offset 2 -> offset 1
// n) asserts weights before/after every operation
// NOTE: interactions are currently 0 for offset 0 since
// config 0 is hard-coded to be empty interactions for now.
bool weights_offset_test(cb_sim&, VW::workspace& all, VW::multi_ex&)
{
  const size_t offset_to_clear = 1;
  auto& weights = all.weights.dense_weights;

  std::vector<std::uint64_t> feature_indexes;
  // hardcoded features that correspond to simulator.h
  feature_indexes.emplace_back(hash_to_index(all.weights, get_hash_for_feature(all, "Action", "article=music")));
  feature_indexes.emplace_back(hash_to_index(all.weights, get_hash_for_feature(all, "Action", "article=politics")));
  feature_indexes.emplace_back(hash_to_index(all.weights, get_hash_for_feature(all, "User", "user=Anna")));

  const size_t interaction_index = interaction_to_index(all.weights,
      get_hash_for_feature(all, "Action", "article=sports"), get_hash_for_feature(all, "Action", "article=sports"));

  static const float EXPECTED_W0 = 0.0259284f;
  static const float EXPECTED_W1 = 0.00720719;
  static const float EXPECTED_W2 = -0.0374119f;
  static const float ZERO = 0.f;

  for (auto index : feature_indexes)
  {
    EXPECT_NE(ZERO, weights.strided_index(index));
    float w1 = weights.strided_index(index + offset_to_clear);
    float w2 = weights.strided_index(index + offset_to_clear + 1);
    EXPECT_NE(ZERO, w1);
    EXPECT_NE(ZERO, w2);
  }

  EXPECT_NEAR(EXPECTED_W0, weights.strided_index(interaction_index), AUTO_ML_FLOAT_TOL);
  EXPECT_NEAR(EXPECTED_W1, weights.strided_index(interaction_index + offset_to_clear), AUTO_ML_FLOAT_TOL);
  EXPECT_NEAR(EXPECTED_W2, weights.strided_index(interaction_index + offset_to_clear + 1), AUTO_ML_FLOAT_TOL);

  // all weights of offset 1 will be set to zero
  VW::reductions::multi_model::clear_offset(weights, offset_to_clear, all.wpp);

  for (auto index : feature_indexes)
  {
    EXPECT_NE(ZERO, weights.strided_index(index));
    float w1 = weights.strided_index(index + offset_to_clear);
    float w2 = weights.strided_index(index + offset_to_clear + 1);
    EXPECT_EQ(ZERO, w1);
    EXPECT_NE(ZERO, w2);
    EXPECT_NE(w1, w2);
  }

  EXPECT_NEAR(EXPECTED_W0, weights.strided_index(interaction_index), AUTO_ML_FLOAT_TOL);
  EXPECT_EQ(ZERO, weights.strided_index(interaction_index + offset_to_clear));
  EXPECT_NEAR(EXPECTED_W2, weights.strided_index(interaction_index + offset_to_clear + 1), AUTO_ML_FLOAT_TOL);

  // copy from offset 2 to offset 1
  weights.move_offsets(offset_to_clear + 1, offset_to_clear, all.wpp);

  for (auto index : feature_indexes)
  {
    EXPECT_NE(ZERO, weights.strided_index(index));
    float w1 = weights.strided_index(index + offset_to_clear);
    float w2 = weights.strided_index(index + offset_to_clear + 1);
    EXPECT_NE(ZERO, w1);
    EXPECT_NE(ZERO, w2);
    EXPECT_EQ(w1, w2);
  }

  EXPECT_NEAR(EXPECTED_W0, weights.strided_index(interaction_index), AUTO_ML_FLOAT_TOL);
  float actual_w1 = weights.strided_index(interaction_index + offset_to_clear);
  float actual_w2 = weights.strided_index(interaction_index + offset_to_clear + 1);
  EXPECT_NEAR(EXPECTED_W2, actual_w1, AUTO_ML_FLOAT_TOL);
  EXPECT_NEAR(EXPECTED_W2, actual_w2, AUTO_ML_FLOAT_TOL);
  EXPECT_EQ(actual_w1, actual_w2);

  // Ensure weights are non-zero for another live interaction
  const size_t interaction_index_other = interaction_to_index(all.weights,
      get_hash_for_feature(all, "Action", "article=sports"), get_hash_for_feature(all, "User", "user=Anna"));

  EXPECT_NE(ZERO, weights.strided_index(interaction_index_other));
  EXPECT_NE(ZERO, weights.strided_index(interaction_index_other + 1));
  EXPECT_NE(ZERO, weights.strided_index(interaction_index_other + 2));

  // Ensure weights are 0 for non-live interactions
  const size_t interaction_index_empty = interaction_to_index(all.weights,
      get_hash_for_feature(all, "User", "user=Anna"), get_hash_for_feature(all, "Action", "article=sports"));

  EXPECT_EQ(ZERO, weights.strided_index(interaction_index_empty));
  EXPECT_EQ(ZERO, weights.strided_index(interaction_index_empty + 1));
  EXPECT_EQ(ZERO, weights.strided_index(interaction_index_empty + 2));

  // VW::automl::helper::print_weights_nonzero(all, 0, all->weights.dense_weights);
  return true;
}

TEST(automl_weights_tests, operations_w_iterations)
{
  const size_t seed = 10;
  const size_t num_iterations = 1331;
  callback_map test_hooks;

  // fn test/assert
  test_hooks.emplace(num_iterations, weights_offset_test);

  auto ctr = simulator::_test_helper_hook(
      "--automl 3 --priority_type favor_popular_namespaces --cb_explore_adf --quiet "
      "--epsilon 0.2 "
      "--random_seed 5 "
      "--oracle_type rand --default_lease 10",
      test_hooks, num_iterations, seed);

  EXPECT_GT(ctr.back(), 0.4f);
}

bool all_weights_equal_test(cb_sim&, VW::workspace& all, VW::multi_ex&)
{
  auto& weights = all.weights.dense_weights;
  uint32_t stride_size = 1 << weights.stride_shift();

  for (auto iter = weights.begin(); iter != weights.end(); ++iter)
  {
    size_t prestride_index = iter.index() >> weights.stride_shift();
    size_t current_offset = (iter.index() >> weights.stride_shift()) & (all.wpp - 1);
    if (current_offset == 0)
    {
      float* first_weight = &weights.first()[(prestride_index + 0) << weights.stride_shift()];
      uint32_t till = 1;  // instead of all.wpp, champdupe only uses 3 configs
      for (uint32_t i = 1; i <= till; ++i)
      {
        float* other = &weights.first()[(prestride_index + i) << weights.stride_shift()];
        for (uint32_t j = 0; j < stride_size; ++j)
        {
          EXPECT_NEAR((&(*first_weight))[j], (&(*other))[j], AUTO_ML_FLOAT_TOL);
        }
      }
    }
  }

  return true;
}

TEST(automl_weights_tests, noop_samechampconfig_w_iterations)
{
  const size_t seed = 10;
  const size_t num_iterations = 1742;
  callback_map test_hooks;

  test_hooks.emplace(500, all_weights_equal_test);
  test_hooks.emplace(num_iterations, all_weights_equal_test);

  auto ctr = simulator::_test_helper_hook(
      "--automl 4 --priority_type favor_popular_namespaces --cb_explore_adf --quiet "
      "--epsilon 0.2 "
      "--random_seed 5 "
      "--oracle_type champdupe -b 8 --default_lease 10 --extra_metrics champdupe.json --verbose_metrics",
      test_hooks, num_iterations, seed);

  EXPECT_GT(ctr.back(), 0.4f);
}

TEST(automl_weights_tests, learn_order_w_iterations)
{
  callback_map test_hooks;

  std::string vw_arg =
      "--automl 4 --priority_type favor_popular_namespaces --cb_explore_adf --quiet "
      "--epsilon 0.2 "
      "--random_seed 5 -b 18 "
      "--oracle_type one_diff --default_lease 10 ";
  int seed = 10;
  size_t num_iterations = 2000;

  auto* vw_increasing = VW::initialize(vw_arg + "--invert_hash learnorder1.vw");
  auto* vw_decreasing = VW::initialize(vw_arg + "--invert_hash learnorder2.vw --debug_reversed_learn");
  simulator::cb_sim sim1(seed);
  simulator::cb_sim sim2(seed);
  auto ctr1 = sim1.run_simulation_hook(vw_increasing, num_iterations, test_hooks);
  auto ctr2 = sim2.run_simulation_hook(vw_decreasing, num_iterations, test_hooks);

  auto& weights_1 = vw_increasing->weights.dense_weights;
  auto& weights_2 = vw_decreasing->weights.dense_weights;
  auto iter_1 = weights_1.begin();
  auto iter_2 = weights_2.begin();

  bool at_least_one_diff = false;

  while (iter_1 != weights_1.end() && iter_2 != weights_2.end())
  {
    EXPECT_EQ(*iter_1, *iter_2);

    if (*iter_1 != *iter_2)
    {
      at_least_one_diff = true;
      break;
    }

    ++iter_1;
    ++iter_2;
  }

  EXPECT_FALSE(at_least_one_diff);

  VW::finish(*vw_increasing);
  VW::finish(*vw_decreasing);

  EXPECT_EQ(ctr1, ctr2);
}

TEST(automl_weights_tests, equal_no_automl_w_iterations)
{
  callback_map test_hooks;

  std::string vw_arg =
      "--cb_explore_adf --quiet --epsilon 0.2 "
      "--random_seed 5 ";
  std::string vw_automl_arg =
      "--automl 4 --priority_type favor_popular_namespaces "
      "--oracle_type one_diff --default_lease 10 ";
  int seed = 10;
  // a switch happens around ~1756
  size_t num_iterations = 1700;
  // this has to match with --automl 4 above
  static const size_t AUTOML_MODELS = 4;

  auto* vw_qcolcol = VW::initialize(vw_arg + "-b 18 --invert_hash without_automl.vw -q ::");
  auto* vw_automl = VW::initialize(
      vw_arg + vw_automl_arg + "-b 20 --invert_hash with_automl.vw --extra_metrics equaltest.json --verbose_metrics");
  simulator::cb_sim sim1(seed);
  simulator::cb_sim sim2(seed);
  auto ctr1 = sim1.run_simulation_hook(vw_qcolcol, num_iterations, test_hooks);
  auto ctr2 = sim2.run_simulation_hook(vw_automl, num_iterations, test_hooks);

  auto& weights_qcolcol = vw_qcolcol->weights.dense_weights;
  auto& weights_automl = vw_automl->weights.dense_weights;
  auto q_col_col_offset = 0;  // q:: is runner-up and not champ (offset 0) if num_iterations > ~1750
  auto iter_2 = weights_automl.begin() + q_col_col_offset;

  std::vector<std::tuple<float, float, float, float>> qcolcol_weights_vector;
  std::vector<std::tuple<float, float, float, float>> automl_champ_weights_vector;

  dense_iterator<float> qcolcol_it = weights_qcolcol.begin();
  auto end = weights_qcolcol.end();

  if (*qcolcol_it != 0.0f)
  {
    qcolcol_weights_vector.emplace_back(*qcolcol_it[0], *qcolcol_it[1], *qcolcol_it[2], *qcolcol_it[3]);
  }

  while (qcolcol_it.next_non_zero(end) < end)
  {
    qcolcol_weights_vector.emplace_back(*qcolcol_it[0], *qcolcol_it[1], *qcolcol_it[2], *qcolcol_it[3]);
  }

  std::sort(qcolcol_weights_vector.begin(), qcolcol_weights_vector.end());

  while (iter_2 < weights_automl.end())
  {
    size_t prestride_index = iter_2.index() >> 2;
    size_t current_offset = prestride_index & (AUTOML_MODELS - 1);
    EXPECT_EQ(current_offset, q_col_col_offset);
    EXPECT_EQ(iter_2.index_without_stride() & (AUTOML_MODELS - 1), current_offset);

    if (*iter_2 != 0.0f) { automl_champ_weights_vector.emplace_back(*iter_2[0], *iter_2[1], *iter_2[2], *iter_2[3]); }

    iter_2 += AUTOML_MODELS;
  }

  VW::finish(*vw_qcolcol);
  VW::finish(*vw_automl);

  std::sort(automl_champ_weights_vector.begin(), automl_champ_weights_vector.end());
  EXPECT_EQ(qcolcol_weights_vector.size(), 31);
  EXPECT_EQ(automl_champ_weights_vector.size(), 31);
  EXPECT_EQ(qcolcol_weights_vector, automl_champ_weights_vector);
  EXPECT_EQ(ctr1, ctr2);
}

TEST(automl_weights_tests, equal_spin_off_model_w_iterations)
{
  callback_map test_hooks;

  std::string vw_arg =
      "--cb_explore_adf --quiet --epsilon 0.2 "
      "--random_seed 5 --predict_only_model ";
  std::string vw_automl_arg =
      "--automl 4 --priority_type favor_popular_namespaces "
      "--oracle_type one_diff --default_lease 10 ";
  int seed = 10;
  // a switch happens around ~1756
  size_t num_iterations = 1700;

  auto* vw_qcolcol = VW::initialize(vw_arg + "-b 17 --interactions \\x20\\x20 --interactions \\x20U --interactions UU");
  auto* vw_automl = VW::initialize(vw_arg + vw_automl_arg + "-b 18");
  simulator::cb_sim sim1(seed, true);
  simulator::cb_sim sim2(seed, true);
  auto ctr1 = sim1.run_simulation_hook(vw_qcolcol, num_iterations, test_hooks);
  auto ctr2 = sim2.run_simulation_hook(vw_automl, num_iterations, test_hooks);
  vw_automl->l->pre_save_load(*vw_automl);

  std::vector<std::string> automl_inters =
      vw_automl->options->get_typed_option<std::vector<std::string>>("interactions").value();
  std::vector<std::string> qcolcol_inters =
      vw_qcolcol->options->get_typed_option<std::vector<std::string>>("interactions").value();
  EXPECT_EQ(automl_inters, qcolcol_inters);

  auto& weights_qcolcol = vw_qcolcol->weights.dense_weights;
  auto& weights_automl = vw_automl->weights.dense_weights;

  std::vector<std::tuple<float, float, float, float>> qcolcol_weights_vector;
  std::vector<std::tuple<float, float, float, float>> automl_weights_vector;

  dense_iterator<float> qcolcol_it = weights_qcolcol.begin();
  auto qcolcol_end = weights_qcolcol.end();

  if (*qcolcol_it != 0.0f)
  {
    qcolcol_weights_vector.emplace_back(*qcolcol_it[0], *qcolcol_it[1], *qcolcol_it[2], *qcolcol_it[3]);
  }

  while (qcolcol_it.next_non_zero(qcolcol_end) < qcolcol_end)
  {
    qcolcol_weights_vector.emplace_back(*qcolcol_it[0], *qcolcol_it[1], *qcolcol_it[2], *qcolcol_it[3]);
  }

  std::sort(qcolcol_weights_vector.begin(), qcolcol_weights_vector.end());

  dense_iterator<float> automl_it = weights_automl.begin();
  auto automl_end = weights_automl.end();

  if (*automl_it != 0.0f)
  {
    automl_weights_vector.emplace_back(*automl_it[0], *automl_it[1], *automl_it[2], *automl_it[3]);
  }

  while (automl_it.next_non_zero(automl_end) < automl_end)
  {
    automl_weights_vector.emplace_back(*automl_it[0], *automl_it[1], *automl_it[2], *automl_it[3]);
  }

  std::sort(automl_weights_vector.begin(), automl_weights_vector.end());

  VW::finish(*vw_qcolcol);
  VW::finish(*vw_automl);

  EXPECT_EQ(qcolcol_weights_vector.size(), 31);
  EXPECT_EQ(automl_weights_vector.size(), 31);
  EXPECT_EQ(qcolcol_weights_vector, automl_weights_vector);
  EXPECT_EQ(ctr1, ctr2);
}

TEST(automl_weights_tests, equal_spin_off_model_cubic)
{
  callback_map test_hooks;

  std::string vw_arg =
      "--cb_explore_adf --quiet --epsilon 0.2 "
      "--random_seed 5 --predict_only_model ";
  std::string vw_automl_arg =
      "--automl 4 --priority_type favor_popular_namespaces "
      "--oracle_type one_diff --default_lease 10 --interaction_type cubic ";
  int seed = 10;
  // a switch happens around ~1756
  size_t num_iterations = 10;

  auto* vw_qcolcol = VW::initialize(vw_arg +
      "-b 17 --interactions \\x20\\x20\\x20 --interactions \\x20\\x20U --interactions \\x20UU --interactions UUU");
  auto* vw_automl = VW::initialize(vw_arg + vw_automl_arg + "-b 18");
  simulator::cb_sim sim1(seed, true);
  simulator::cb_sim sim2(seed, true);
  auto ctr1 = sim1.run_simulation_hook(vw_qcolcol, num_iterations, test_hooks);
  auto ctr2 = sim2.run_simulation_hook(vw_automl, num_iterations, test_hooks);
  vw_automl->l->pre_save_load(*vw_automl);

  std::vector<std::string> automl_inters =
      vw_automl->options->get_typed_option<std::vector<std::string>>("interactions").value();
  std::vector<std::string> qcolcol_inters =
      vw_qcolcol->options->get_typed_option<std::vector<std::string>>("interactions").value();
  EXPECT_EQ(automl_inters, qcolcol_inters);

  auto& weights_qcolcol = vw_qcolcol->weights.dense_weights;
  auto& weights_automl = vw_automl->weights.dense_weights;

  std::vector<std::tuple<float, float, float, float>> qcolcol_weights_vector;
  std::vector<std::tuple<float, float, float, float>> automl_weights_vector;

  dense_iterator<float> qcolcol_it = weights_qcolcol.begin();
  auto qcolcol_end = weights_qcolcol.end();

  if (*qcolcol_it != 0.0f)
  {
    qcolcol_weights_vector.emplace_back(*qcolcol_it[0], *qcolcol_it[1], *qcolcol_it[2], *qcolcol_it[3]);
  }

  while (qcolcol_it.next_non_zero(qcolcol_end) < qcolcol_end)
  {
    qcolcol_weights_vector.emplace_back(*qcolcol_it[0], *qcolcol_it[1], *qcolcol_it[2], *qcolcol_it[3]);
  }

  std::sort(qcolcol_weights_vector.begin(), qcolcol_weights_vector.end());

  dense_iterator<float> automl_it = weights_automl.begin();
  auto automl_end = weights_automl.end();

  if (*automl_it != 0.0f)
  {
    automl_weights_vector.emplace_back(*automl_it[0], *automl_it[1], *automl_it[2], *automl_it[3]);
  }

  while (automl_it.next_non_zero(automl_end) < automl_end)
  {
    automl_weights_vector.emplace_back(*automl_it[0], *automl_it[1], *automl_it[2], *automl_it[3]);
  }

  std::sort(automl_weights_vector.begin(), automl_weights_vector.end());

  VW::finish(*vw_qcolcol);
  VW::finish(*vw_automl);

  EXPECT_EQ(qcolcol_weights_vector.size(), 38);
  EXPECT_EQ(automl_weights_vector.size(), 38);
  EXPECT_EQ(qcolcol_weights_vector, automl_weights_vector);
  EXPECT_EQ(ctr1, ctr2);
}
