// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "simulator.h"
#include "test_common.h"
#include "vw/core/constant.h"  // FNV_prime
#include "vw/core/vw_math.h"

#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>
#include <functional>
#include <map>

#define ARE_SAME(X, Y, Z) \
  BOOST_CHECK_MESSAGE(VW::math::are_same(X, Y, Z), "check ARE_SAME: expected: " << X << " not equal to " << Y);

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
  // FNV_prime from constant.h
  return hash_to_index(weights, (FNV_prime * one) ^ two);
}
}  // namespace vw_hash_helpers

using namespace vw_hash_helpers;

// 0) runs after 1331 learned examples from simulator.h
// 1) clears (set to zero) offset 1
// 2) copies offset 2 -> offset 1
// n) asserts weights before/after every operation
// NOTE: interactions are currently 0 for offset 0 since
// config 0 is hard-coded to be empty interactions for now.
bool weights_offset_test(cb_sim&, VW::workspace& all, VW::multi_ex& ec)
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

  const float expected_w0 = 0.0284346f;
  const float expected_w1 = -0.0268783f;
  const float expected_w2 = -0.0279688f;
  const float ZERO = 0.f;

  for (auto index : feature_indexes)
  {
    BOOST_CHECK_NE(ZERO, weights.strided_index(index));
    float w1 = weights.strided_index(index + offset_to_clear);
    float w2 = weights.strided_index(index + offset_to_clear + 1);
    BOOST_CHECK_NE(ZERO, w1);
    BOOST_CHECK_NE(ZERO, w2);
  }

  ARE_SAME(expected_w0, weights.strided_index(interaction_index), AUTO_ML_FLOAT_TOL);
  ARE_SAME(expected_w1, weights.strided_index(interaction_index + offset_to_clear), AUTO_ML_FLOAT_TOL);
  ARE_SAME(expected_w2, weights.strided_index(interaction_index + offset_to_clear + 1), AUTO_ML_FLOAT_TOL);

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

  ARE_SAME(expected_w0, weights.strided_index(interaction_index), AUTO_ML_FLOAT_TOL);
  BOOST_CHECK_EQUAL(ZERO, weights.strided_index(interaction_index + offset_to_clear));
  ARE_SAME(expected_w2, weights.strided_index(interaction_index + offset_to_clear + 1), AUTO_ML_FLOAT_TOL);

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

  ARE_SAME(expected_w0, weights.strided_index(interaction_index), AUTO_ML_FLOAT_TOL);
  float actual_w1 = weights.strided_index(interaction_index + offset_to_clear);
  float actual_w2 = weights.strided_index(interaction_index + offset_to_clear + 1);
  ARE_SAME(expected_w2, actual_w1, AUTO_ML_FLOAT_TOL);
  ARE_SAME(expected_w2, actual_w2, AUTO_ML_FLOAT_TOL);
  BOOST_CHECK_EQUAL(actual_w1, actual_w2);

  // Ensure weights are non-zero for another live interaction
  const size_t interaction_index_other = interaction_to_index(all.weights,
      get_hash_for_feature(all, "Action", "article=sports"), get_hash_for_feature(all, "User", "user=Anna"));

  BOOST_CHECK_NE(ZERO, weights.strided_index(interaction_index_other));
  BOOST_CHECK_NE(ZERO, weights.strided_index(interaction_index_other + 1));
  BOOST_CHECK_NE(ZERO, weights.strided_index(interaction_index_other + 2));

  // Ensure weights are 0 for non-live interactions
  const size_t interaction_index_empty = interaction_to_index(all.weights,
      get_hash_for_feature(all, "User", "user=Anna"), get_hash_for_feature(all, "Action", "article=sports"));

  BOOST_CHECK_EQUAL(ZERO, weights.strided_index(interaction_index_empty));
  BOOST_CHECK_EQUAL(ZERO, weights.strided_index(interaction_index_empty + 1));
  BOOST_CHECK_EQUAL(ZERO, weights.strided_index(interaction_index_empty + 2));

  // VW::automl::helper::print_weights_nonzero(all, 0, all->weights.dense_weights);
  return true;
}

BOOST_AUTO_TEST_CASE(automl_weight_operations)
{
  const size_t seed = 10;
  const size_t num_iterations = 1331;
  callback_map test_hooks;

  // fn test/assert
  test_hooks.emplace(num_iterations, weights_offset_test);

  auto ctr = simulator::_test_helper_hook(
      "--automl 3 --automl_estimator_decay .999 --priority_type favor_popular_namespaces --cb_explore_adf --quiet "
      "--epsilon 0.2 "
      "--random_seed 5 "
      "--keep_configs --oracle_type rand",
      test_hooks, num_iterations, seed);

  BOOST_CHECK_GT(ctr.back(), 0.4f);
}

bool all_weights_equal_test(cb_sim&, VW::workspace& all, VW::multi_ex& ec)
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
      for (uint32_t i = 1; i < all.wpp; ++i)
      {
        float* other = &weights.first()[(prestride_index + i) << weights.stride_shift()];
        for (uint32_t j = 0; j < stride_size; ++j)
        { ARE_SAME((&(*first_weight))[j], (&(*other))[j], AUTO_ML_FLOAT_TOL); }
      }
    }
  }

  return true;
}

BOOST_AUTO_TEST_CASE(automl_noop_samechampconfig)
{
  const size_t seed = 10;
  const size_t num_iterations = 2000;
  callback_map test_hooks;

  test_hooks.emplace(500, all_weights_equal_test);
  test_hooks.emplace(num_iterations, all_weights_equal_test);

  auto ctr = simulator::_test_helper_hook(
      "--automl 4 --automl_estimator_decay .999 --priority_type favor_popular_namespaces --cb_explore_adf --quiet --epsilon 0.2 "
      "--random_seed 5 "
      "--keep_configs --oracle_type champdupe -b 8",
      test_hooks, num_iterations, seed);

  BOOST_CHECK_GT(ctr.back(), 0.4f);
}
