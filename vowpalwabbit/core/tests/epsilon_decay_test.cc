#include "vw/core/reductions/epsilon_decay.h"

#include "simulator.h"
#include "vw/core/learner.h"
#include "vw/core/metric_sink.h"
#include "vw/core/setup_base.h"
#include "vw/test_common/test_common.h"

#include <gtest/gtest.h>

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
  all.l->get_enabled_learners(e_r);
  if (std::find(e_r.begin(), e_r.end(), "epsilon_decay") == e_r.end())
  {
    THROW("Epsilon decay not found in enabled learners");
  }

  VW::LEARNER::learner* epsilon_decay_learner = require_multiline(all.l->get_learner_by_name_prefix("epsilon_decay"));

  return (epsilon_decay_data*)epsilon_decay_learner->get_internal_type_erased_data_pointer_test_use_only();
}
}  // namespace epsilon_decay_test

TEST(EpsilonDecay, ThrowIfNoExplore)
{
  EXPECT_THROW(
      {
        try
        {
          auto result = VW::initialize(vwtest::make_args("--epsilon_decay", "--cb_adf"));
        }
        catch (const VW::vw_exception& e)
        {
          EXPECT_STREQ(
              "Input prediction type: prediction_type_t::ACTION_PROBS of learner: epsilon_decay does not match "
              "output prediction type: prediction_type_t::ACTION_SCORES of base learner: cb_adf.",
              e.what());
          throw;
        }
      },
      VW::vw_exception);
}

TEST(EpsilonDecay, InitWIterations)
{
  // we initialize the reduction pointing to position 0 as champ, that config is hard-coded to empty
  auto ctr = simulator::_test_helper(std::vector<std::string>{
      "--epsilon_decay", "--model_count=3", "--cb_explore_adf", "--quiet", "--epsilon=0.2", "--random_seed=5"});
}

TEST(EpsilonDecay, ChampChangeWIterations)
{
  const size_t num_iterations = 610;
  const size_t seed = 36;
  const std::vector<uint64_t> swap_after = {500};
  const size_t deterministic_champ_switch = 600;
  callback_map test_hooks;

  test_hooks.emplace(deterministic_champ_switch - 1,
      [&](cb_sim&, VW::workspace& all, VW::multi_ex&)
      {
        epsilon_decay_data* epsilon_decay = epsilon_decay_test::get_epsilon_decay_data(all);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[0][0].update_count, 87);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[1][0].update_count, 87);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[2][0].update_count, 87);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[3][0].update_count, 87);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[1][1].update_count, 92);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[2][1].update_count, 92);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[3][1].update_count, 92);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[2][2].update_count, 93);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[3][2].update_count, 93);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[3][3].update_count, 599);
        return true;
      });

  test_hooks.emplace(deterministic_champ_switch,
      [&](cb_sim&, VW::workspace& all, VW::multi_ex&)
      {
        epsilon_decay_data* epsilon_decay = epsilon_decay_test::get_epsilon_decay_data(all);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[0][0].update_count, 0);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[1][0].update_count, 0);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[2][0].update_count, 0);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[3][0].update_count, 0);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[1][1].update_count, 88);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[2][1].update_count, 88);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[3][1].update_count, 88);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[2][2].update_count, 93);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[3][2].update_count, 93);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[3][3].update_count, 94);
        return true;
      });

  // we initialize the reduction pointing to position 0 as champ, that config is hard-coded to empty
  auto ctr = simulator::_test_helper_hook(
      std::vector<std::string>{"--epsilon_decay", "--model_count", "4", "--cb_explore_adf", "--quiet", "-q", "::"},
      test_hooks, num_iterations, seed, swap_after);

  EXPECT_GT(ctr.back(), 0.4f);
}

TEST(EpsilonDecay, UpdateCountWIterations)
{
  const size_t num_iterations = 105;
  const size_t seed = 100;
  callback_map test_hooks;

  test_hooks.emplace(100,
      [&](cb_sim&, VW::workspace& all, VW::multi_ex&)
      {
        epsilon_decay_data* epsilon_decay = epsilon_decay_test::get_epsilon_decay_data(all);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[0][0].update_count, 100);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[1][0].update_count, 100);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[1][1].update_count, 100);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[2][0].update_count, 100);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[2][1].update_count, 100);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[2][2].update_count, 100);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[3][0].update_count, 100);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[3][1].update_count, 100);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[3][2].update_count, 100);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[3][3].update_count, 100);
        EXPECT_EQ(epsilon_decay->_weight_indices[0], 0);
        EXPECT_EQ(epsilon_decay->_weight_indices[1], 1);
        EXPECT_EQ(epsilon_decay->_weight_indices[2], 2);
        EXPECT_EQ(epsilon_decay->_weight_indices[3], 3);
        return true;
      });

  test_hooks.emplace(101,
      [&](cb_sim&, VW::workspace& all, VW::multi_ex&)
      {
        epsilon_decay_data* epsilon_decay = epsilon_decay_test::get_epsilon_decay_data(all);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[0][0].update_count, 0);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[1][0].update_count, 0);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[1][1].update_count, 101);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[2][0].update_count, 0);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[2][1].update_count, 101);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[2][2].update_count, 101);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[3][0].update_count, 0);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[3][1].update_count, 101);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[3][2].update_count, 101);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[3][3].update_count, 101);
        EXPECT_EQ(epsilon_decay->_weight_indices[0], 2);
        EXPECT_EQ(epsilon_decay->_weight_indices[1], 0);
        EXPECT_EQ(epsilon_decay->_weight_indices[2], 1);
        EXPECT_EQ(epsilon_decay->_weight_indices[3], 3);
        return true;
      });

  test_hooks.emplace(102,
      [&](cb_sim&, VW::workspace& all, VW::multi_ex&)
      {
        epsilon_decay_data* epsilon_decay = epsilon_decay_test::get_epsilon_decay_data(all);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[0][0].update_count, 0);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[1][0].update_count, 0);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[1][1].update_count, 1);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[2][0].update_count, 0);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[2][1].update_count, 1);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[2][2].update_count, 102);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[3][0].update_count, 0);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[3][1].update_count, 1);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[3][2].update_count, 102);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[3][3].update_count, 102);
        EXPECT_EQ(epsilon_decay->_weight_indices[0], 1);
        EXPECT_EQ(epsilon_decay->_weight_indices[1], 2);
        EXPECT_EQ(epsilon_decay->_weight_indices[2], 0);
        EXPECT_EQ(epsilon_decay->_weight_indices[3], 3);
        return true;
      });

  test_hooks.emplace(103,
      [&](cb_sim&, VW::workspace& all, VW::multi_ex&)
      {
        epsilon_decay_data* epsilon_decay = epsilon_decay_test::get_epsilon_decay_data(all);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[0][0].update_count, 0);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[1][0].update_count, 0);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[1][1].update_count, 1);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[2][0].update_count, 0);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[2][1].update_count, 1);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[2][2].update_count, 2);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[3][0].update_count, 0);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[3][1].update_count, 1);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[3][2].update_count, 2);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[3][3].update_count, 103);
        EXPECT_EQ(epsilon_decay->_weight_indices[0], 0);
        EXPECT_EQ(epsilon_decay->_weight_indices[1], 1);
        EXPECT_EQ(epsilon_decay->_weight_indices[2], 2);
        EXPECT_EQ(epsilon_decay->_weight_indices[3], 3);
        return true;
      });

  test_hooks.emplace(104,
      [&](cb_sim&, VW::workspace& all, VW::multi_ex&)
      {
        epsilon_decay_data* epsilon_decay = epsilon_decay_test::get_epsilon_decay_data(all);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[0][0].update_count, 1);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[1][0].update_count, 1);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[1][1].update_count, 2);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[2][0].update_count, 1);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[2][1].update_count, 2);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[2][2].update_count, 3);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[3][0].update_count, 1);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[3][1].update_count, 2);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[3][2].update_count, 3);
        EXPECT_EQ(epsilon_decay->conf_seq_estimators[3][3].update_count, 104);
        EXPECT_EQ(epsilon_decay->_weight_indices[0], 0);
        EXPECT_EQ(epsilon_decay->_weight_indices[1], 1);
        EXPECT_EQ(epsilon_decay->_weight_indices[2], 2);
        EXPECT_EQ(epsilon_decay->_weight_indices[3], 3);
        return true;
      });

  // we initialize the reduction pointing to position 0 as champ, that config is hard-coded to empty
  auto ctr = simulator::_test_helper_hook(
      std::vector<std::string>{"--epsilon_decay", "--model_count", "4", "--cb_explore_adf", "--quiet", "-q", "::"},
      test_hooks, num_iterations, seed);

  EXPECT_GT(ctr.back(), 0.5f);
}

TEST(EpsilonDecay, SaveLoadWIterations)
{
  callback_map empty_hooks;
  auto ctr =
      simulator::_test_helper_hook(std::vector<std::string>{"--epsilon_decay", "--model_count", "5", "--cb_explore_adf",
                                       "--epsilon_decay_significance_level", ".01", "--quiet", "-q", "::"},
          empty_hooks);
  float without_save = ctr.back();
  EXPECT_GT(without_save, 0.9f);

  ctr = simulator::_test_helper_save_load(std::vector<std::string>{"--epsilon_decay", "--model_count", "5",
      "--cb_explore_adf", "--epsilon_decay_significance_level", ".01", "--quiet", "-q", "::"});

  float with_save = ctr.back();
  EXPECT_GT(with_save, 0.9f);

  EXPECT_FLOAT_EQ(without_save, with_save);
}

TEST(EpsilonDecay, ScoreBoundsUnit)
{
  // Initialize epsilon_decay_data class with 5 models
  uint64_t num_models = 5;
  uint32_t wpp = 8;
  VW::dense_parameters dense_weights(num_models);
  epsilon_decay_data ep_data(
      num_models, 100, .05, .1, dense_weights, "", false, wpp, 0, 1.f, 0, false, 1e-6, "bisect", false);

  // Set update counts to fixed values with expected horizon bound violation
  size_t score_idx = 0;
  for (uint64_t model_ind = 0; model_ind < num_models; ++model_ind)
  {
    for (uint64_t score_ind = 0; score_ind <= model_ind; ++score_ind)
    {
      ep_data.conf_seq_estimators[model_ind][score_ind].update_count = score_idx;
      ++score_idx;
    }
  }

  /*
   * Check that score and weight indices are in expected start positions:
   * W_i denotes weight index i, and other indices denote update_count
   *
   * W0: 0
   * W1: 1  2
   * W2: 3  4  5
   * W3: 6  7  8  9
   * W4: 10 11 12 13 14
   *
   */
  EXPECT_EQ(ep_data.conf_seq_estimators[0][0].update_count, 0);
  EXPECT_EQ(ep_data.conf_seq_estimators[1][0].update_count, 1);
  EXPECT_EQ(ep_data.conf_seq_estimators[1][1].update_count, 2);
  EXPECT_EQ(ep_data.conf_seq_estimators[2][0].update_count, 3);
  EXPECT_EQ(ep_data.conf_seq_estimators[2][1].update_count, 4);
  EXPECT_EQ(ep_data.conf_seq_estimators[2][2].update_count, 5);
  EXPECT_EQ(ep_data.conf_seq_estimators[3][0].update_count, 6);
  EXPECT_EQ(ep_data.conf_seq_estimators[3][1].update_count, 7);
  EXPECT_EQ(ep_data.conf_seq_estimators[3][2].update_count, 8);
  EXPECT_EQ(ep_data.conf_seq_estimators[3][3].update_count, 9);
  EXPECT_EQ(ep_data.conf_seq_estimators[4][0].update_count, 10);
  EXPECT_EQ(ep_data.conf_seq_estimators[4][1].update_count, 11);
  EXPECT_EQ(ep_data.conf_seq_estimators[4][2].update_count, 12);
  EXPECT_EQ(ep_data.conf_seq_estimators[4][3].update_count, 13);
  EXPECT_EQ(ep_data.conf_seq_estimators[4][4].update_count, 14);
  EXPECT_EQ(ep_data._weight_indices[0], 0);
  EXPECT_EQ(ep_data._weight_indices[1], 1);
  EXPECT_EQ(ep_data._weight_indices[2], 2);
  EXPECT_EQ(ep_data._weight_indices[3], 3);
  EXPECT_EQ(ep_data._weight_indices[4], 4);

  // Set lower_bound of model 2 to beat upper_bound of current champ and run score check
  uint64_t new_champ = 2;
  for (auto i = 0; i < 100; ++i) { ep_data.conf_seq_estimators[new_champ][new_champ].update(i, 1); };
  for (auto i = 0; i < 100; ++i) { ep_data.conf_seq_estimators[num_models - 1][new_champ].update(i, 0); };

  EXPECT_GT(ep_data.conf_seq_estimators[new_champ][new_champ].lower_bound(),
      ep_data.conf_seq_estimators[num_models - 1][new_champ].upper_bound());
  ep_data.check_estimator_bounds();

  /*
   * Check that weight and score indices shift as expected when model 2 overtakes champion:
   * W_i denotes weight index i, and other indices denote update_count. Scores
   * marked with X have been removed and have garbage values.
   *
   * W4: X
   * W3: X  X
   * W0: X  X  0
   * W1: X  X  1  2
   * W2: X  X  3  4  5
   *
   */
  EXPECT_EQ(ep_data.conf_seq_estimators[2][2].update_count, 0);
  EXPECT_EQ(ep_data.conf_seq_estimators[3][2].update_count, 1);
  EXPECT_EQ(ep_data.conf_seq_estimators[3][3].update_count, 2);
  EXPECT_EQ(ep_data.conf_seq_estimators[4][2].update_count, 3);
  EXPECT_EQ(ep_data.conf_seq_estimators[4][3].update_count, 4);
  EXPECT_EQ(ep_data.conf_seq_estimators[4][4].update_count, 105);
  EXPECT_EQ(ep_data._weight_indices[0], 4);
  EXPECT_EQ(ep_data._weight_indices[1], 3);
  EXPECT_EQ(ep_data._weight_indices[2], 0);
  EXPECT_EQ(ep_data._weight_indices[3], 1);
  EXPECT_EQ(ep_data._weight_indices[4], 2);
}

TEST(EpsilonDecay, HorizonBoundsUnit)
{
  // Initialize epsilon_decay_data class with 5 models
  uint64_t num_models = 5;
  uint32_t wpp = 8;
  VW::dense_parameters dense_weights(num_models);
  epsilon_decay_data ep_data(
      num_models, 100, .05, .1, dense_weights, "", false, wpp, 0, 1.f, 0, false, 1e-6, "bisect", false);

  // Set update counts to fixed values with expected horizon bound violation
  size_t score_idx = 0;
  uint64_t over_horizon = 2;
  for (uint64_t model_ind = 0; model_ind < num_models; ++model_ind)
  {
    for (uint64_t score_ind = 0; score_ind <= model_ind; ++score_ind)
    {
      ep_data.conf_seq_estimators[model_ind][score_ind].update_count = score_idx;
      ++score_idx;
    }
  }
  // Set specific update_counts so model 2 in removed
  ep_data.conf_seq_estimators[over_horizon][over_horizon].update_count = 500;
  ep_data.conf_seq_estimators[num_models - 1][num_models - 1].update_count = 1000;

  /*
   * Check that score and weight indices are in expected start positions:
   * W_i denotes weight index i, and other indices denote update_count
   *
   * W0: 0
   * W1: 1  2
   * W2: 3  4  500
   * W3: 6  7  8  9
   * W4: 10 11 12 13 1000
   *
   */
  EXPECT_EQ(ep_data.conf_seq_estimators[0][0].update_count, 0);
  EXPECT_EQ(ep_data.conf_seq_estimators[1][0].update_count, 1);
  EXPECT_EQ(ep_data.conf_seq_estimators[1][1].update_count, 2);
  EXPECT_EQ(ep_data.conf_seq_estimators[2][0].update_count, 3);
  EXPECT_EQ(ep_data.conf_seq_estimators[2][1].update_count, 4);
  EXPECT_EQ(ep_data.conf_seq_estimators[2][2].update_count, 500);
  EXPECT_EQ(ep_data.conf_seq_estimators[3][0].update_count, 6);
  EXPECT_EQ(ep_data.conf_seq_estimators[3][1].update_count, 7);
  EXPECT_EQ(ep_data.conf_seq_estimators[3][2].update_count, 8);
  EXPECT_EQ(ep_data.conf_seq_estimators[3][3].update_count, 9);
  EXPECT_EQ(ep_data.conf_seq_estimators[4][0].update_count, 10);
  EXPECT_EQ(ep_data.conf_seq_estimators[4][1].update_count, 11);
  EXPECT_EQ(ep_data.conf_seq_estimators[4][2].update_count, 12);
  EXPECT_EQ(ep_data.conf_seq_estimators[4][3].update_count, 13);
  EXPECT_EQ(ep_data.conf_seq_estimators[4][4].update_count, 1000);
  EXPECT_EQ(ep_data._weight_indices[0], 0);
  EXPECT_EQ(ep_data._weight_indices[1], 1);
  EXPECT_EQ(ep_data._weight_indices[2], 2);
  EXPECT_EQ(ep_data._weight_indices[3], 3);
  EXPECT_EQ(ep_data._weight_indices[4], 4);

  // Set update_count of model 2 to be above threshold horizon based on champion
  EXPECT_GT(ep_data.conf_seq_estimators[over_horizon][over_horizon].update_count,
      std::pow(ep_data.conf_seq_estimators[num_models - 1][num_models - 1].update_count,
          static_cast<float>(over_horizon + 1) / num_models));
  ep_data.check_horizon_bounds();

  /*
   * Check that weight and score indices shift as expected when model 2 overtakes champion:
   * W_i denotes weight index i, and other indices denote update_count. Scores
   * marked with X have been removed and have garbage values.
   *
   * W2: X
   * W0: X  0
   * W1: X  1  2
   * W3: X  6  7  9
   * W4: X  10 11 13 1000
   *
   */
  EXPECT_EQ(ep_data.conf_seq_estimators[1][1].update_count, 0);
  EXPECT_EQ(ep_data.conf_seq_estimators[2][1].update_count, 1);
  EXPECT_EQ(ep_data.conf_seq_estimators[2][2].update_count, 2);
  EXPECT_EQ(ep_data.conf_seq_estimators[3][1].update_count, 6);
  EXPECT_EQ(ep_data.conf_seq_estimators[3][2].update_count, 7);
  EXPECT_EQ(ep_data.conf_seq_estimators[3][3].update_count, 9);
  EXPECT_EQ(ep_data.conf_seq_estimators[4][1].update_count, 10);
  EXPECT_EQ(ep_data.conf_seq_estimators[4][2].update_count, 11);
  EXPECT_EQ(ep_data.conf_seq_estimators[4][3].update_count, 13);
  EXPECT_EQ(ep_data.conf_seq_estimators[4][4].update_count, 1000);
  EXPECT_EQ(ep_data._weight_indices[0], 2);
  EXPECT_EQ(ep_data._weight_indices[1], 0);
  EXPECT_EQ(ep_data._weight_indices[2], 1);
  EXPECT_EQ(ep_data._weight_indices[3], 3);
  EXPECT_EQ(ep_data._weight_indices[4], 4);
}
