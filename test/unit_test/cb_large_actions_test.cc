// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "reductions/cb/details/large_action_space.h"
#include "test_common.h"
#include "vw/core/qr_decomposition.h"
#include "vw/core/rand48.h"
#include "vw/core/rand_state.h"
#include "vw/core/reductions/cb/cb_explore_adf_common.h"
#include "vw/core/reductions/cb/cb_explore_adf_large_action_space.h"
#include "vw/core/vw.h"

#include <boost/test/unit_test.hpp>

using internal_action_space = VW::cb_explore_adf::cb_explore_adf_base<
    VW::cb_explore_adf::cb_explore_adf_large_action_space<VW::cb_explore_adf::vanilla_rand_svd_impl>>;
using internal_action_space_mw = VW::cb_explore_adf::cb_explore_adf_base<
    VW::cb_explore_adf::cb_explore_adf_large_action_space<VW::cb_explore_adf::model_weight_rand_svd_impl>>;
using internal_action_space_op = VW::cb_explore_adf::cb_explore_adf_base<
    VW::cb_explore_adf::cb_explore_adf_large_action_space<VW::cb_explore_adf::one_pass_svd_impl>>;

BOOST_AUTO_TEST_SUITE(test_suite_las)

BOOST_AUTO_TEST_CASE(creation_of_the_og_A_matrix)
{
  auto d = 2;
  auto& vw = *VW::initialize("--cb_explore_adf --large_action_space --full_predictions --max_actions " +
          std::to_string(d) + " --quiet --random_seed 5",
      nullptr, false, nullptr, nullptr);

  std::vector<std::string> e_r;
  vw.l->get_enabled_reductions(e_r);
  if (std::find(e_r.begin(), e_r.end(), "cb_explore_adf_large_action_space") == e_r.end())
  { BOOST_FAIL("cb_explore_adf_large_action_space not found in enabled reductions"); }

  VW::LEARNER::multi_learner* learner =
      as_multiline(vw.l->get_learner_by_name_prefix("cb_explore_adf_large_action_space"));

  auto action_space = (internal_action_space*)learner->get_internal_type_erased_data_pointer_test_use_only();

  BOOST_CHECK_EQUAL(action_space != nullptr, true);

  std::vector<Eigen::Triplet<float>> _triplets;

  {
    VW::multi_ex examples;

    examples.push_back(VW::read_example(vw, "0:1.0:0.5 | 1:0.1 2:0.2 3:0.3"));

    std::vector<float> ft_values = {0.1f, 0.2f, 0.3f};

    vw.predict(examples);

    VW::cb_explore_adf::_generate_A(&vw, examples, _triplets, action_space->explore._A);

    auto num_actions = examples.size();
    BOOST_CHECK_EQUAL(num_actions, 1);

    uint64_t action_index = 0;
    auto* ex = examples[action_index];
    // test sanity - test assumes no shared features
    BOOST_CHECK_EQUAL(!CB::ec_is_example_header(*ex), true);
    for (auto ns : ex->indices)
    {
      for (size_t i = 0; i < ex->feature_space[ns].indices.size(); i++)
      {
        auto ft_index = ex->feature_space[ns].indices[i];
        auto ft_value = ex->feature_space[ns].values[i];

        if (ns == default_namespace) { BOOST_CHECK_CLOSE(ft_value, ft_values[i], FLOAT_TOL); }
        else if (ns == constant_namespace)
        {
          BOOST_CHECK_CLOSE(ft_value, 1.f, FLOAT_TOL);
        }

        BOOST_CHECK_EQUAL(
            action_space->explore._A.coeffRef(action_index, (ft_index & vw.weights.dense_weights.mask())), ft_value);
      }
    }

    vw.finish_example(examples);
  }
  VW::finish(vw);
}

BOOST_AUTO_TEST_CASE(creation_of_AAtop)
{
  auto d = 2;
  auto& vw = *VW::initialize("--cb_explore_adf --large_action_space --squarecb --full_predictions --max_actions " +
          std::to_string(d) + " --quiet --random_seed 5",
      nullptr, false, nullptr, nullptr);

  std::vector<std::string> e_r;
  vw.l->get_enabled_reductions(e_r);
  if (std::find(e_r.begin(), e_r.end(), "cb_explore_adf_large_action_space") == e_r.end())
  { BOOST_FAIL("cb_explore_adf_large_action_space not found in enabled reductions"); }

  VW::LEARNER::multi_learner* learner =
      as_multiline(vw.l->get_learner_by_name_prefix("cb_explore_adf_large_action_space"));

  auto action_space = (internal_action_space*)learner->get_internal_type_erased_data_pointer_test_use_only();

  BOOST_CHECK_EQUAL(action_space != nullptr, true);

  VW::cb_explore_adf::aatop_impl _aatop_impl(&vw);

  {
    VW::multi_ex examples;

    examples.push_back(VW::read_example(vw, "0:1.0:0.5 | 1:0.1 2:0.2 3:0.3"));
    examples.push_back(VW::read_example(vw, " | 2:0.1 3:0.2 4:0.3"));

    vw.predict(examples);
    auto& preds = examples[0]->pred.a_s;
    action_space->explore._shrink_factor_config.calculate_shrink_factor(
        0, d, examples[0]->pred.a_s, action_space->explore.shrink_factors);

    VW::cb_explore_adf::_generate_A(&vw, examples, _aatop_impl._triplets, action_space->explore._A);
    _aatop_impl.run(examples, action_space->explore.shrink_factors);

    Eigen::SparseMatrix<float> diag_M(examples.size(), examples.size());

    for (Eigen::Index i = 0; i < action_space->explore.shrink_factors.size(); i++)
    { diag_M.coeffRef(i, i) = action_space->explore.shrink_factors[i]; }

    Eigen::MatrixXf AAtop = diag_M * action_space->explore._A * action_space->explore._A.transpose() * diag_M;

    BOOST_CHECK_EQUAL(AAtop.isApprox(_aatop_impl.AAtop), true);

    vw.finish_example(examples);
  }
  VW::finish(vw);
}

BOOST_AUTO_TEST_CASE(check_AO_same_actions_same_representation)
{
  auto d = 3;
  std::vector<VW::workspace*> vws;
  auto& vw = *VW::initialize("--cb_explore_adf --large_action_space --full_predictions --max_actions " +
          std::to_string(d) + " --quiet --random_seed 5 --one_pass",
      nullptr, false, nullptr, nullptr);

  std::vector<std::string> e_r;
  vw.l->get_enabled_reductions(e_r);
  if (std::find(e_r.begin(), e_r.end(), "cb_explore_adf_large_action_space") == e_r.end())
  { BOOST_FAIL("cb_explore_adf_large_action_space not found in enabled reductions"); }

  VW::LEARNER::multi_learner* learner =
      as_multiline(vw.l->get_learner_by_name_prefix("cb_explore_adf_large_action_space"));

  auto action_space = (internal_action_space_op*)learner->get_internal_type_erased_data_pointer_test_use_only();

  BOOST_CHECK_EQUAL(action_space != nullptr, true);

  action_space->explore._populate_all_testing_components();

  {
    VW::multi_ex examples;

    examples.push_back(VW::read_example(vw, "| 1:0.1 2:0.12 3:0.13 b200:2 c500:9"));
    // duplicates start
    examples.push_back(VW::read_example(vw, "| a_1:0.5 a_2:0.65 a_3:0.12 a100:4 a200:33"));
    examples.push_back(VW::read_example(vw, "| a_1:0.5 a_2:0.65 a_3:0.12 a100:4 a200:33"));
    // duplicates end
    examples.push_back(VW::read_example(vw, "| a_4:0.8 a_5:0.32 a_6:0.15 d1:0.2 d10:0.2"));
    examples.push_back(VW::read_example(vw, "| a_7 a_8 a_9 v1:0.99"));
    examples.push_back(VW::read_example(vw, "| a_10 a_11 a_12"));
    examples.push_back(VW::read_example(vw, "| a_13 a_14 a_15"));
    examples.push_back(VW::read_example(vw, "| a_16 a_17 a_18:0.2"));

    vw.predict(examples);

    // representation of actions 2 and 3 (duplicates) should be the same in U
    BOOST_CHECK_EQUAL(action_space->explore.U.row(1).isApprox(action_space->explore.U.row(2)), true);

    vw.finish_example(examples);
  }
  VW::finish(vw);
}

BOOST_AUTO_TEST_CASE(check_AO_linear_combination_of_actions)
{
  auto d = 3;
  std::vector<VW::workspace*> vws;
  auto& vw = *VW::initialize("--cb_explore_adf --large_action_space --full_predictions --max_actions " +
          std::to_string(d) + " --quiet --random_seed 5 --one_pass --noconstant",
      nullptr, false, nullptr, nullptr);

  std::vector<std::string> e_r;
  vw.l->get_enabled_reductions(e_r);
  if (std::find(e_r.begin(), e_r.end(), "cb_explore_adf_large_action_space") == e_r.end())
  { BOOST_FAIL("cb_explore_adf_large_action_space not found in enabled reductions"); }

  VW::LEARNER::multi_learner* learner =
      as_multiline(vw.l->get_learner_by_name_prefix("cb_explore_adf_large_action_space"));

  auto action_space = (internal_action_space_op*)learner->get_internal_type_erased_data_pointer_test_use_only();

  BOOST_CHECK_EQUAL(action_space != nullptr, true);

  action_space->explore._populate_all_testing_components();

  {
    VW::multi_ex examples;

    examples.push_back(VW::read_example(vw, "| 1:0.1 2:0.12 3:0.13 b200:2 c500:9"));

    examples.push_back(VW::read_example(vw, "| a_1:0.5 a_2:0.65 a_3:0.12 a100:4 a200:33"));
    examples.push_back(VW::read_example(vw, "| a_1:0.8 a_2:0.32 a_3:0.15 a100:0.2 a200:0.2"));
    // linear combination of the above two actions
    // action_4 = action_2 + 2 * action_3
    examples.push_back(VW::read_example(vw, "| a_1:2.1 a_2:1.29 a_3:0.42 a100:4.4 a200:33.4"));

    examples.push_back(VW::read_example(vw, "| a_4:0.8 a_5:0.32 a_6:0.15 d1:0.2 d10: 0.2"));
    examples.push_back(VW::read_example(vw, "| a_7 a_8 a_9 v1:0.99"));
    examples.push_back(VW::read_example(vw, "| a_10 a_11 a_12"));
    examples.push_back(VW::read_example(vw, "| a_13 a_14 a_15"));
    examples.push_back(VW::read_example(vw, "| a_16 a_17 a_18:0.2"));

    vw.predict(examples);

    // check that the representation of the fourth action is the same linear combination of the representation of the
    // 2nd and 3rd actions
    Eigen::VectorXf action_2 = action_space->explore.U.row(1);
    Eigen::VectorXf action_3 = action_space->explore.U.row(2);
    Eigen::VectorXf action_4 = action_space->explore.U.row(3);

    Eigen::VectorXf action_lin_rep = action_2 + 2.f * action_3;

    BOOST_CHECK_EQUAL(action_lin_rep.isApprox(action_4), true);

    vw.finish_example(examples);
  }
  VW::finish(vw);
}

BOOST_AUTO_TEST_CASE(test_two_Ys_are_equal)
{
  auto d = 2;
  auto& vw = *VW::initialize("--cb_explore_adf --large_action_space --full_predictions --max_actions " +
          std::to_string(d) + " --quiet --random_seed 5 -q ::",
      nullptr, false, nullptr, nullptr);

  std::vector<std::string> e_r;
  vw.l->get_enabled_reductions(e_r);
  if (std::find(e_r.begin(), e_r.end(), "cb_explore_adf_large_action_space") == e_r.end())
  { BOOST_FAIL("cb_explore_adf_large_action_space not found in enabled reductions"); }

  VW::LEARNER::multi_learner* learner =
      as_multiline(vw.l->get_learner_by_name_prefix("cb_explore_adf_large_action_space"));

  auto action_space = (internal_action_space*)learner->get_internal_type_erased_data_pointer_test_use_only();

  BOOST_CHECK_EQUAL(action_space != nullptr, true);

  VW::cb_explore_adf::model_weight_rand_svd_impl _model_weight_rand_svd_impl(&vw, d, 50, 1 << vw.num_bits);

  {
    VW::multi_ex examples;

    examples.push_back(VW::read_example(vw, "|f 1:0.1 2:0.12 3:0.13"));
    examples.push_back(VW::read_example(vw, "|f a_1:0.5 a_2:0.65 a_3:0.12"));
    examples.push_back(VW::read_example(vw, "|f a_4:0.8 a_5:0.32 a_6:0.15"));

    vw.predict(examples);

    action_space->explore._impl.generate_Y(examples, action_space->explore.shrink_factors);
    Eigen::SparseMatrix<float> Y_vanilla = action_space->explore._impl.Y;

    uint64_t max_existing_column = 0;
    _model_weight_rand_svd_impl.generate_model_weight_Y(
        examples, max_existing_column, action_space->explore.shrink_factors);
    _model_weight_rand_svd_impl._populate_from_model_weight_Y(examples);

    BOOST_CHECK_EQUAL(_model_weight_rand_svd_impl.Y.rows() > 0, true);
    BOOST_CHECK_EQUAL(_model_weight_rand_svd_impl.Y.cols(), d);

    BOOST_CHECK_EQUAL(Y_vanilla.isApprox(_model_weight_rand_svd_impl.Y), true);

    vw.finish_example(examples);

    VW::finish(vw);
  }
}

BOOST_AUTO_TEST_CASE(test_two_Bs_are_equal)
{
  auto d = 2;
  auto& vw = *VW::initialize("--cb_explore_adf --large_action_space --full_predictions --max_actions " +
          std::to_string(d) + " --quiet --random_seed 5 -q ::",
      nullptr, false, nullptr, nullptr);

  std::vector<std::string> e_r;
  vw.l->get_enabled_reductions(e_r);
  if (std::find(e_r.begin(), e_r.end(), "cb_explore_adf_large_action_space") == e_r.end())
  { BOOST_FAIL("cb_explore_adf_large_action_space not found in enabled reductions"); }

  VW::LEARNER::multi_learner* learner =
      as_multiline(vw.l->get_learner_by_name_prefix("cb_explore_adf_large_action_space"));

  auto action_space = (internal_action_space*)learner->get_internal_type_erased_data_pointer_test_use_only();

  BOOST_CHECK_EQUAL(action_space != nullptr, true);

  VW::cb_explore_adf::model_weight_rand_svd_impl _model_weight_rand_svd_impl(&vw, d, 50, 1 << vw.num_bits);

  {
    VW::multi_ex examples;

    examples.push_back(VW::read_example(vw, "|f 1:0.1 2:0.12 3:0.13"));
    examples.push_back(VW::read_example(vw, "|f a_1:0.5 a_2:0.65 a_3:0.12"));
    examples.push_back(VW::read_example(vw, "|f a_4:0.8 a_5:0.32 a_6:0.15"));

    vw.predict(examples);

    action_space->explore._impl.generate_Y(examples, action_space->explore.shrink_factors);
    action_space->explore._impl.generate_B(examples, action_space->explore.shrink_factors);
    Eigen::MatrixXf B_vanilla = action_space->explore._impl.B;

    uint64_t max_existing_column = 0;
    _model_weight_rand_svd_impl.generate_model_weight_Y(
        examples, max_existing_column, action_space->explore.shrink_factors);
    _model_weight_rand_svd_impl.generate_B_model_weight(
        examples, max_existing_column, action_space->explore.shrink_factors);

    BOOST_CHECK_EQUAL(B_vanilla.isApprox(_model_weight_rand_svd_impl.B), true);

    vw.finish_example(examples);

    VW::finish(vw);
  }
}

BOOST_AUTO_TEST_CASE(check_interactions_on_Y)
{
  auto d = 2;
  std::vector<std::pair<VW::workspace*, bool>> vws;
  auto* vw_no_interactions = VW::initialize("--cb_explore_adf --large_action_space --full_predictions --max_actions " +
          std::to_string(d) + " --quiet --random_seed 5",
      nullptr, false, nullptr, nullptr);

  vws.push_back({vw_no_interactions, false});

  auto* vw_yes_interactions = VW::initialize("--cb_explore_adf --large_action_space --full_predictions --max_actions " +
          std::to_string(d) + " --quiet --random_seed 5 -q ::",
      nullptr, false, nullptr, nullptr);

  vws.push_back({vw_yes_interactions, true});

  size_t interactions_rows = 0;
  size_t non_interactions_rows = 0;

  for (auto& vw_pair : vws)
  {
    auto& vw = *std::get<0>(vw_pair);
    bool interactions = std::get<1>(vw_pair);

    std::vector<std::string> e_r;
    vw.l->get_enabled_reductions(e_r);
    if (std::find(e_r.begin(), e_r.end(), "cb_explore_adf_large_action_space") == e_r.end())
    { BOOST_FAIL("cb_explore_adf_large_action_space not found in enabled reductions"); }

    VW::LEARNER::multi_learner* learner =
        as_multiline(vw.l->get_learner_by_name_prefix("cb_explore_adf_large_action_space"));

    auto action_space = (internal_action_space*)learner->get_internal_type_erased_data_pointer_test_use_only();

    BOOST_CHECK_EQUAL(action_space != nullptr, true);

    {
      VW::multi_ex examples;

      examples.push_back(VW::read_example(vw, "|f 1:0.1 2:0.12 3:0.13"));
      examples.push_back(VW::read_example(vw, "|f a_1:0.5 a_2:0.65 a_3:0.12"));
      examples.push_back(VW::read_example(vw, "|f a_4:0.8 a_5:0.32 a_6:0.15"));

      vw.predict(examples);

      std::set<uint64_t> non_zero_rows;
      for (int k = 0; k < action_space->explore._impl.Y.outerSize(); ++k)
      {
        for (Eigen::SparseMatrix<float>::InnerIterator it(action_space->explore._impl.Y, k); it; ++it)
        { non_zero_rows.emplace(it.row()); }
      }

      if (!interactions) { non_interactions_rows = non_zero_rows.size(); }
      if (interactions) { interactions_rows = non_zero_rows.size(); }
      vw.finish_example(examples);
    }
    VW::finish(vw);
  }
  BOOST_CHECK_GT(interactions_rows, non_interactions_rows);
}

BOOST_AUTO_TEST_CASE(check_interactions_on_B)
{
  auto d = 2;
  std::vector<std::pair<VW::workspace*, bool>> vws;
  auto* vw_no_interactions = VW::initialize("--cb_explore_adf --large_action_space --full_predictions --max_actions " +
          std::to_string(d) + " --quiet --random_seed 5",
      nullptr, false, nullptr, nullptr);

  vws.push_back({vw_no_interactions, false});

  auto* vw_yes_interactions = VW::initialize("--cb_explore_adf --large_action_space --full_predictions --max_actions " +
          std::to_string(d) + " --quiet --random_seed 5 -q ::",
      nullptr, false, nullptr, nullptr);

  vws.push_back({vw_yes_interactions, true});

  Eigen::MatrixXf B_non_interactions;
  Eigen::MatrixXf B_interactions;

  for (auto& vw_pair : vws)
  {
    auto& vw = *std::get<0>(vw_pair);
    bool interactions = std::get<1>(vw_pair);

    std::vector<std::string> e_r;
    vw.l->get_enabled_reductions(e_r);
    if (std::find(e_r.begin(), e_r.end(), "cb_explore_adf_large_action_space") == e_r.end())
    { BOOST_FAIL("cb_explore_adf_large_action_space not found in enabled reductions"); }

    VW::LEARNER::multi_learner* learner =
        as_multiline(vw.l->get_learner_by_name_prefix("cb_explore_adf_large_action_space"));

    auto action_space = (internal_action_space*)learner->get_internal_type_erased_data_pointer_test_use_only();

    BOOST_CHECK_EQUAL(action_space != nullptr, true);

    {
      VW::multi_ex examples;

      examples.push_back(VW::read_example(vw, "|f 1:0.1 2:0.12 3:0.13"));
      examples.push_back(VW::read_example(vw, "|f a_1:0.5 a_2:0.65 a_3:0.12"));
      examples.push_back(VW::read_example(vw, "|f a_4:0.8 a_5:0.32 a_6:0.15"));

      vw.predict(examples);

      if (!interactions) { B_non_interactions = action_space->explore._impl.B; }
      if (interactions) { B_interactions = action_space->explore._impl.B; }
      vw.finish_example(examples);
    }
    VW::finish(vw);
  }
  BOOST_CHECK_EQUAL(B_interactions.isApprox(B_non_interactions), false);
}

BOOST_AUTO_TEST_CASE(check_At_times_Omega_is_Y)
{
  auto d = 2;
  std::vector<std::pair<VW::workspace*, bool>> vws;
  auto* vw_epsilon = VW::initialize("--cb_explore_adf --large_action_space --full_predictions --max_actions " +
          std::to_string(d) + " --quiet --random_seed 5",
      nullptr, false, nullptr, nullptr);

  vws.push_back({vw_epsilon, false});

  auto* vw_squarecb =
      VW::initialize("--cb_explore_adf --squarecb --large_action_space --full_predictions --max_actions " +
              std::to_string(d) + " --quiet --random_seed 5",
          nullptr, false, nullptr, nullptr);

  vws.push_back({vw_squarecb, true});

  for (auto& vw_pair : vws)
  {
    auto& vw = *std::get<0>(vw_pair);
    auto apply_diag_M = std::get<1>(vw_pair);

    std::vector<std::string> e_r;
    vw.l->get_enabled_reductions(e_r);
    if (std::find(e_r.begin(), e_r.end(), "cb_explore_adf_large_action_space") == e_r.end())
    { BOOST_FAIL("cb_explore_adf_large_action_space not found in enabled reductions"); }

    VW::LEARNER::multi_learner* learner =
        as_multiline(vw.l->get_learner_by_name_prefix("cb_explore_adf_large_action_space"));

    auto action_space = (internal_action_space*)learner->get_internal_type_erased_data_pointer_test_use_only();

    BOOST_CHECK_EQUAL(action_space != nullptr, true);

    std::vector<Eigen::Triplet<float>> _triplets;

    {
      VW::multi_ex examples;

      examples.push_back(VW::read_example(vw, "| 1:0.1 2:0.12 3:0.13"));
      examples.push_back(VW::read_example(vw, "| a_1:0.5 a_2:0.65 a_3:0.12"));
      examples.push_back(VW::read_example(vw, "| a_4:0.8 a_5:0.32 a_6:0.15"));
      examples.push_back(VW::read_example(vw, "| a_7 a_8 a_9"));
      examples.push_back(VW::read_example(vw, "| a_10 a_11 a_12"));
      examples.push_back(VW::read_example(vw, "| a_13 a_14 a_15"));
      examples.push_back(VW::read_example(vw, "| a_16 a_17 a_18"));

      vw.predict(examples);

      action_space->explore._shrink_factor_config.calculate_shrink_factor(
          0, d, examples[0]->pred.a_s, action_space->explore.shrink_factors);

      VW::cb_explore_adf::_generate_A(&vw, examples, _triplets, action_space->explore._A);
      action_space->explore._impl.generate_Y(examples, action_space->explore.shrink_factors);

      uint64_t num_actions = examples[0]->pred.a_s.size();

      // Generate Omega
      std::vector<Eigen::Triplet<float>> omega_triplets;
      uint64_t max_ft_index = 0;
      uint64_t seed = vw.get_random_state()->get_current_state() * 10.f;

      for (uint64_t action_index = 0; action_index < num_actions; action_index++)
      {
        auto* ex = examples[action_index];
        // test sanity - test assumes no shared features
        BOOST_CHECK_EQUAL(!CB::ec_is_example_header(*ex), true);
        for (auto ns : ex->indices)
        {
          for (uint64_t col = 0; col < d; col++)
          {
            auto combined_index = action_index + col + seed;
            auto mm = merand48_boxmuller(combined_index);
            omega_triplets.push_back(Eigen::Triplet<float>(action_index, col, mm));
          }
        }
      }

      Eigen::SparseMatrix<float> Omega(num_actions, d);
      Omega.setFromTriplets(omega_triplets.begin(), omega_triplets.end(), [](const float& a, const float& b) {
        assert(a == b);
        return b;
      });

      Eigen::SparseMatrix<float> diag_M(num_actions, num_actions);

      if (apply_diag_M)
      {
        for (Eigen::Index i = 0; i < action_space->explore.shrink_factors.size(); i++)
        { diag_M.coeffRef(i, i) = action_space->explore.shrink_factors[i]; }
      }
      else
      {
        diag_M.setIdentity();
      }

      Eigen::SparseMatrix<float> Yd(action_space->explore._impl.Y.rows(), d);

      Yd = action_space->explore._A.transpose() * diag_M * Omega;
      // Orthonormalize Yd
      VW::gram_schmidt(Yd);
      BOOST_CHECK_EQUAL(Yd.isApprox(action_space->explore._impl.Y), true);

      vw.finish_example(examples);
    }
    VW::finish(vw);
  }
}

BOOST_AUTO_TEST_CASE(check_A_times_Y_is_B)
{
  auto d = 2;
  std::vector<std::pair<VW::workspace*, bool>> vws;
  auto* vw_epsilon = VW::initialize("--cb_explore_adf --large_action_space --full_predictions --max_actions " +
          std::to_string(d) + " --quiet --random_seed 5",
      nullptr, false, nullptr, nullptr);

  vws.push_back({vw_epsilon, false});

  auto* vw_squarecb =
      VW::initialize("--cb_explore_adf --squarecb --large_action_space --full_predictions --max_actions " +
              std::to_string(d) + " --quiet --random_seed 5",
          nullptr, false, nullptr, nullptr);

  vws.push_back({vw_squarecb, true});

  for (auto& vw_pair : vws)
  {
    auto& vw = *std::get<0>(vw_pair);
    std::vector<Eigen::Triplet<float>> _triplets;
    auto apply_diag_M = std::get<1>(vw_pair);

    std::vector<std::string> e_r;
    vw.l->get_enabled_reductions(e_r);
    if (std::find(e_r.begin(), e_r.end(), "cb_explore_adf_large_action_space") == e_r.end())
    { BOOST_FAIL("cb_explore_adf_large_action_space not found in enabled reductions"); }

    VW::LEARNER::multi_learner* learner =
        as_multiline(vw.l->get_learner_by_name_prefix("cb_explore_adf_large_action_space"));

    auto action_space = (internal_action_space*)learner->get_internal_type_erased_data_pointer_test_use_only();

    BOOST_CHECK_EQUAL(action_space != nullptr, true);

    {
      VW::multi_ex examples;

      examples.push_back(VW::read_example(vw, "0:1.0:0.5 | 1:0.1 2:0.12 3:0.13"));
      examples.push_back(VW::read_example(vw, "| a_1:0.5 a_2:0.65 a_3:0.12"));

      vw.predict(examples);

      VW::cb_explore_adf::_generate_A(&vw, examples, _triplets, action_space->explore._A);
      action_space->explore._shrink_factor_config.calculate_shrink_factor(
          0, d, examples[0]->pred.a_s, action_space->explore.shrink_factors);

      action_space->explore._impl.generate_Y(examples, action_space->explore.shrink_factors);
      action_space->explore._impl.generate_B(examples, action_space->explore.shrink_factors);

      auto num_actions = examples[0]->pred.a_s.size();
      Eigen::SparseMatrix<float> diag_M(num_actions, num_actions);

      if (apply_diag_M)
      {
        for (Eigen::Index i = 0; i < action_space->explore.shrink_factors.size(); i++)
        { diag_M.coeffRef(i, i) = action_space->explore.shrink_factors[i]; }
      }
      else
      {
        diag_M.setIdentity();
      }

      Eigen::MatrixXf B = diag_M * action_space->explore._A * action_space->explore._impl.Y;
      BOOST_CHECK_EQUAL(B.isApprox(action_space->explore._impl.B), true);

      vw.finish_example(examples);
    }
    VW::finish(vw);
  }
}

BOOST_AUTO_TEST_CASE(check_B_times_P_is_Z)
{
  auto d = 2;

  std::vector<std::pair<VW::workspace*, bool>> vws;
  auto* vw_epsilon = VW::initialize("--cb_explore_adf --large_action_space --full_predictions --max_actions " +
          std::to_string(d) + " --quiet --random_seed 5",
      nullptr, false, nullptr, nullptr);

  vws.push_back({vw_epsilon, false});

  auto* vw_squarecb =
      VW::initialize("--cb_explore_adf --squarecb --large_action_space --full_predictions --max_actions " +
              std::to_string(d) + " --quiet --random_seed 5",
          nullptr, false, nullptr, nullptr);

  vws.push_back({vw_squarecb, true});

  for (auto& vw_pair : vws)
  {
    auto& vw = *std::get<0>(vw_pair);
    std::vector<Eigen::Triplet<float>> _triplets;

    std::vector<std::string> e_r;
    vw.l->get_enabled_reductions(e_r);
    if (std::find(e_r.begin(), e_r.end(), "cb_explore_adf_large_action_space") == e_r.end())
    { BOOST_FAIL("cb_explore_adf_large_action_space not found in enabled reductions"); }

    VW::LEARNER::multi_learner* learner =
        as_multiline(vw.l->get_learner_by_name_prefix("cb_explore_adf_large_action_space"));

    auto action_space = (internal_action_space*)learner->get_internal_type_erased_data_pointer_test_use_only();

    BOOST_CHECK_EQUAL(action_space != nullptr, true);

    {
      VW::multi_ex examples;

      examples.push_back(VW::read_example(vw, "0:1.0:0.5 | 1:0.1 2:0.12 3:0.13"));
      examples.push_back(VW::read_example(vw, "| a_1:0.5 a_2:0.65 a_3:0.12"));

      vw.predict(examples);

      action_space->explore._shrink_factor_config.calculate_shrink_factor(
          0, d, examples[0]->pred.a_s, action_space->explore.shrink_factors);
      VW::cb_explore_adf::_generate_A(&vw, examples, _triplets, action_space->explore._A);
      action_space->explore._impl.generate_Y(examples, action_space->explore.shrink_factors);
      action_space->explore._impl.generate_B(examples, action_space->explore.shrink_factors);
      VW::cb_explore_adf::generate_Z(examples, action_space->explore._impl.Z, action_space->explore._impl.B, d, 50);

      Eigen::MatrixXf P(d, d);

      uint64_t seed = vw.get_random_state()->get_current_state() * 10.f;

      for (size_t row = 0; row < d; row++)
      {
        for (size_t col = 0; col < d; col++)
        {
          auto combined_index = row + col + seed;
          auto mm = merand48_boxmuller(combined_index);
          P(row, col) = mm;
        }
      }

      Eigen::MatrixXf Zp = action_space->explore._impl.B * P;
      VW::gram_schmidt(Zp);
      BOOST_CHECK_EQUAL(Zp.isApprox(action_space->explore._impl.Z), true);
      vw.finish_example(examples);
    }

    VW::finish(vw);
  }
}

template <typename T>
void check_final_truncated_SVD_validity_impl(VW::workspace& vw,
    VW::cb_explore_adf::cb_explore_adf_base<VW::cb_explore_adf::cb_explore_adf_large_action_space<T>>* action_space,
    bool apply_diag_M, std::vector<Eigen::Triplet<float>> _triplets, uint64_t d)
{
  BOOST_CHECK_EQUAL(action_space != nullptr, true);

  {
    VW::multi_ex examples;

    examples.push_back(VW::read_example(vw, "|f 1:0.1 2:0.12 3:0.13"));
    examples.push_back(VW::read_example(vw, "|f a_1:0.5 a_2:0.65 a_3:0.12"));
    examples.push_back(VW::read_example(vw, "|f a_4:0.8 a_5:0.32 a_6:0.15"));
    action_space->explore._populate_all_testing_components();

    vw.predict(examples);

    action_space->explore._shrink_factor_config.calculate_shrink_factor(
        0, d, examples[0]->pred.a_s, action_space->explore.shrink_factors);
    action_space->explore.randomized_SVD(examples);

    VW::cb_explore_adf::_generate_A(&vw, examples, _triplets, action_space->explore._A);
    {
      Eigen::FullPivLU<Eigen::MatrixXf> lu_decomp(action_space->explore._A);
      auto rank = lu_decomp.rank();
      // for test set actual rank of A
      action_space->explore._set_rank(rank);
      // should have a rank larger than 1 for the test
      BOOST_CHECK_GT(rank, 1);
    }

    vw.predict(examples);

    action_space->explore._shrink_factor_config.calculate_shrink_factor(
        0, d, examples[0]->pred.a_s, action_space->explore.shrink_factors);
    action_space->explore.randomized_SVD(examples);

    auto num_actions = examples.size();

    // U dimensions should be K x d
    BOOST_CHECK_EQUAL(action_space->explore.U.rows(), num_actions);
    BOOST_CHECK_EQUAL(action_space->explore.U.cols(), d);

    // truncated randomized SVD reconstruction
    for (int i = 0; i < action_space->explore.U.cols(); ++i)
    {
      BOOST_CHECK_SMALL(1.f - action_space->explore.U.col(i).norm(), FLOAT_TOL);
      for (int j = 0; j < i; ++j)
      { BOOST_CHECK_SMALL(action_space->explore.U.col(i).dot(action_space->explore.U.col(j)), FLOAT_TOL); }
    }

    for (int i = 0; i < action_space->explore._V.cols(); ++i)
    {
      BOOST_CHECK_SMALL(1.f - action_space->explore._V.col(i).norm(), FLOAT_TOL);
      for (int j = 0; j < i; ++j)
      { BOOST_CHECK_SMALL(action_space->explore._V.col(i).dot(action_space->explore._V.col(j)), FLOAT_TOL); }
    }

    Eigen::SparseMatrix<float> diag_M(num_actions, num_actions);

    if (apply_diag_M)
    {
      for (Eigen::Index i = 0; i < action_space->explore.shrink_factors.size(); i++)
      { diag_M.coeffRef(i, i) = action_space->explore.shrink_factors[i]; }
    }
    else
    {
      diag_M.setIdentity();
    }

    BOOST_CHECK_SMALL(
        ((diag_M * action_space->explore._A) -
            action_space->explore.U * action_space->explore._S.asDiagonal() * action_space->explore._V.transpose())
            .norm(),
        FLOAT_TOL);

    // compare singular values with actual SVD singular values
    Eigen::MatrixXf A_dense = diag_M * action_space->explore._A;
    Eigen::JacobiSVD<Eigen::MatrixXf> svd(A_dense, Eigen::ComputeThinU | Eigen::ComputeThinV);
    Eigen::VectorXf S = svd.singularValues();

    for (size_t i = 0; i < action_space->explore._S.rows(); i++)
    { BOOST_CHECK_SMALL(S(i) - action_space->explore._S(i), FLOAT_TOL); }

    vw.finish_example(examples);
  }
}

BOOST_AUTO_TEST_CASE(check_final_truncated_SVD_validity)
{
  auto d = 3;

  std::vector<std::tuple<VW::workspace*, bool, VW::cb_explore_adf::implementation_type>> vws;
  auto* vw_wo_interactions = VW::initialize("--cb_explore_adf --large_action_space --full_predictions --max_actions " +
          std::to_string(d) + " --quiet --random_seed 5",
      nullptr, false, nullptr, nullptr);

  vws.emplace_back(vw_wo_interactions, false, VW::cb_explore_adf::implementation_type::vanilla_rand_svd);

  auto* vw_w_interactions = VW::initialize("--cb_explore_adf --large_action_space --full_predictions --max_actions " +
          std::to_string(d) + " --quiet --random_seed 5 -q ::",
      nullptr, false, nullptr, nullptr);

  vws.emplace_back(vw_w_interactions, false, VW::cb_explore_adf::implementation_type::vanilla_rand_svd);

  auto* vw_wo_interactions_sq =
      VW::initialize("--cb_explore_adf --squarecb --large_action_space --full_predictions --max_actions " +
              std::to_string(d) + " --quiet --random_seed 5",
          nullptr, false, nullptr, nullptr);

  vws.emplace_back(vw_wo_interactions_sq, true, VW::cb_explore_adf::implementation_type::vanilla_rand_svd);

  auto* vw_w_interactions_sq =
      VW::initialize("--cb_explore_adf --squarecb --large_action_space --full_predictions --max_actions " +
              std::to_string(d) + " --quiet --random_seed 5 -q ::",
          nullptr, false, nullptr, nullptr);

  vws.emplace_back(vw_w_interactions_sq, true, VW::cb_explore_adf::implementation_type::vanilla_rand_svd);

  auto* vw_model_weight_w_interactions_sq =
      VW::initialize("--cb_explore_adf --squarecb --large_action_space --full_predictions --max_actions " +
              std::to_string(d) + " --quiet --random_seed 5 -q :: --model_weight",
          nullptr, false, nullptr, nullptr);

  vws.emplace_back(
      vw_model_weight_w_interactions_sq, true, VW::cb_explore_adf::implementation_type::model_weight_rand_svd);

  auto* vw_w_interactions_sq_sparse_weights = VW::initialize(
      "--cb_explore_adf --squarecb --sparse_weights --large_action_space --full_predictions --max_actions " +
          std::to_string(d) + " --quiet --random_seed 5 -q ::",
      nullptr, false, nullptr, nullptr);

  vws.emplace_back(
      vw_w_interactions_sq_sparse_weights, true, VW::cb_explore_adf::implementation_type::vanilla_rand_svd);

  for (auto& vw_pair : vws)
  {
    auto& vw = *std::get<0>(vw_pair);
    std::vector<Eigen::Triplet<float>> _triplets;
    auto apply_diag_M = std::get<1>(vw_pair);
    auto impl_type = std::get<2>(vw_pair);

    std::vector<std::string> e_r;
    vw.l->get_enabled_reductions(e_r);
    if (std::find(e_r.begin(), e_r.end(), "cb_explore_adf_large_action_space") == e_r.end())
    { BOOST_FAIL("cb_explore_adf_large_action_space not found in enabled reductions"); }

    VW::LEARNER::multi_learner* learner =
        as_multiline(vw.l->get_learner_by_name_prefix("cb_explore_adf_large_action_space"));

    if (impl_type == VW::cb_explore_adf::implementation_type::vanilla_rand_svd)
    {
      auto action_space = (internal_action_space*)learner->get_internal_type_erased_data_pointer_test_use_only();
      check_final_truncated_SVD_validity_impl<VW::cb_explore_adf::vanilla_rand_svd_impl>(
          vw, action_space, apply_diag_M, _triplets, d);
    }
    else if (impl_type == VW::cb_explore_adf::implementation_type::model_weight_rand_svd)
    {
      auto action_space = (internal_action_space_mw*)learner->get_internal_type_erased_data_pointer_test_use_only();
      check_final_truncated_SVD_validity_impl<VW::cb_explore_adf::model_weight_rand_svd_impl>(
          vw, action_space, apply_diag_M, _triplets, d);
    }
    else
    {
      BOOST_FAIL("test for implementation type not implemented");
    }

    VW::finish(vw);
  }
}

BOOST_AUTO_TEST_CASE(check_shrink_factor)
{
  auto d = 2;
  std::vector<std::pair<VW::workspace*, bool>> vws;
  auto* vw_epsilon = VW::initialize("--cb_explore_adf --large_action_space --full_predictions --max_actions " +
          std::to_string(d) + " --quiet --random_seed 5",
      nullptr, false, nullptr, nullptr);

  vws.push_back({vw_epsilon, false});

  auto* vw_squarecb =
      VW::initialize("--cb_explore_adf --squarecb --large_action_space --full_predictions --max_actions " +
              std::to_string(d) + " --quiet --random_seed 5",
          nullptr, false, nullptr, nullptr);

  vws.push_back({vw_squarecb, true});

  for (auto& vw_pair : vws)
  {
    auto& vw = *std::get<0>(vw_pair);
    auto apply_diag_M = std::get<1>(vw_pair);

    std::vector<std::string> e_r;
    vw.l->get_enabled_reductions(e_r);
    if (std::find(e_r.begin(), e_r.end(), "cb_explore_adf_large_action_space") == e_r.end())
    { BOOST_FAIL("cb_explore_adf_large_action_space not found in enabled reductions"); }

    VW::LEARNER::multi_learner* learner =
        as_multiline(vw.l->get_learner_by_name_prefix("cb_explore_adf_large_action_space"));

    auto action_space = (internal_action_space*)learner->get_internal_type_erased_data_pointer_test_use_only();

    BOOST_CHECK_EQUAL(action_space != nullptr, true);

    VW::multi_ex examples;

    examples.push_back(VW::read_example(vw, "| 1:0.1 2:0.12 3:0.13"));
    examples.push_back(VW::read_example(vw, "| a_1:0.5 a_2:0.65 a_3:0.12"));
    examples.push_back(VW::read_example(vw, "| a_4:0.8 a_5:0.32 a_6:0.15"));
    examples.push_back(VW::read_example(vw, "| a_7 a_8 a_9"));
    examples.push_back(VW::read_example(vw, "| a_10 a_11 a_12"));
    examples.push_back(VW::read_example(vw, "| a_13 a_14 a_15"));
    examples.push_back(VW::read_example(vw, "| a_16 a_17 a_18"));

    vw.predict(examples);

    auto num_actions = examples[0]->pred.a_s.size();

    BOOST_CHECK_EQUAL(num_actions, 7);

    action_space->explore._shrink_factor_config.calculate_shrink_factor(
        0, d, examples[0]->pred.a_s, action_space->explore.shrink_factors);

    Eigen::SparseMatrix<float> diag_M(num_actions, num_actions);
    Eigen::SparseMatrix<float> identity_diag_M(num_actions, num_actions);
    identity_diag_M.setIdentity();

    for (Eigen::Index i = 0; i < action_space->explore.shrink_factors.size(); i++)
    { diag_M.coeffRef(i, i) = action_space->explore.shrink_factors[i]; }

    if (apply_diag_M) { BOOST_CHECK_EQUAL(diag_M.isApprox(identity_diag_M), false); }
    else
    {
      BOOST_CHECK_EQUAL(diag_M.isApprox(identity_diag_M), true);
    }

    vw.finish_example(examples);
    VW::finish(vw);
  }
}

BOOST_AUTO_TEST_CASE(check_finding_max_volume)
{
  auto d = 3;
  auto& vw = *VW::initialize("--cb_explore_adf --large_action_space --full_predictions --max_actions " +
          std::to_string(d) + " --quiet --random_seed 5",
      nullptr, false, nullptr, nullptr);
  uint64_t seed = vw.get_random_state()->get_current_state() * 10.f;
  VW::cb_explore_adf::cb_explore_adf_large_action_space<VW::cb_explore_adf::model_weight_rand_svd_impl> largecb(
      /*d=*/0, /*gamma_scale=*/1.f, /*gamma_exponent=*/0.f, /*c=*/2, false, &vw, seed, 1 << vw.num_bits,
      VW::cb_explore_adf::implementation_type::model_weight_rand_svd);
  largecb.U = Eigen::MatrixXf{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}, {0, 0, 0}, {7, 5, 3}, {6, 4, 8}};
  Eigen::MatrixXf X{{1, 2, 3}, {3, 2, 1}, {2, 1, 3}};

  auto p = VW::cb_explore_adf::spanner_state::find_max_volume(largecb.U, 0, X);
  BOOST_CHECK_SMALL(p.first - 30, FLOAT_TOL);
  BOOST_CHECK_EQUAL(p.second, 2);

  p = VW::cb_explore_adf::spanner_state::find_max_volume(largecb.U, 1, X);
  BOOST_CHECK_SMALL(p.first - 27, FLOAT_TOL);
  BOOST_CHECK_EQUAL(p.second, 4);

  p = VW::cb_explore_adf::spanner_state::find_max_volume(largecb.U, 2, X);
  BOOST_CHECK_SMALL(p.first - 24, FLOAT_TOL);
  BOOST_CHECK_EQUAL(p.second, 5);

  VW::finish(vw);
}

BOOST_AUTO_TEST_CASE(check_spanner_results_squarecb)
{
  auto d = 2;
  std::vector<std::pair<VW::workspace*, bool>> vws;

  auto* vw_full_preds =
      VW::initialize("--cb_explore_adf --squarecb --large_action_space --full_predictions --max_actions " +
              std::to_string(d) + " --quiet --random_seed 5",
          nullptr, false, nullptr, nullptr);

  vws.push_back({vw_full_preds, true});

  auto* vw_sparse_preds = VW::initialize("--cb_explore_adf --squarecb --large_action_space --max_actions " +
          std::to_string(d) + " --quiet --random_seed 5",
      nullptr, false, nullptr, nullptr);

  vws.push_back({vw_sparse_preds, false});

  for (auto& vw_pair : vws)
  {
    auto& vw = *std::get<0>(vw_pair);
    auto full_preds = std::get<1>(vw_pair);

    {
      VW::multi_ex examples;

      examples.push_back(VW::read_example(vw, "0:1.0:0.5 | 1:0.1 2:0.12 3:0.13"));
      examples.push_back(VW::read_example(vw, "| a_1:0.5 a_2:0.65 a_3:0.12"));
      examples.push_back(VW::read_example(vw, "| a_4:0.8 a_5:0.32 a_6:0.15"));

      vw.learn(examples);
      vw.finish_example(examples);
    }

    {
      VW::multi_ex examples;

      examples.push_back(VW::read_example(vw, "| 1:0.1 2:0.12 3:0.13"));
      examples.push_back(VW::read_example(vw, "0:1.0:0.5 | a_1:0.5 a_2:0.65 a_3:0.12"));
      examples.push_back(VW::read_example(vw, "| a_4:0.8 a_5:0.32 a_6:0.15"));

      vw.learn(examples);
      vw.finish_example(examples);
    }

    {
      VW::multi_ex examples;

      examples.push_back(VW::read_example(vw, "| 1:0.1 2:0.12 3:0.13"));
      examples.push_back(VW::read_example(vw, "| a_1:0.5 a_2:0.65 a_3:0.12"));
      examples.push_back(VW::read_example(vw, "0:1.0:0.5 | a_4:0.8 a_5:0.32 a_6:0.15"));

      vw.learn(examples);
      vw.finish_example(examples);
    }

    std::vector<std::string> e_r;
    vw.l->get_enabled_reductions(e_r);
    if (std::find(e_r.begin(), e_r.end(), "cb_explore_adf_large_action_space") == e_r.end())
    { BOOST_FAIL("cb_explore_adf_large_action_space not found in enabled reductions"); }

    VW::LEARNER::multi_learner* learner =
        as_multiline(vw.l->get_learner_by_name_prefix("cb_explore_adf_large_action_space"));

    auto action_space = (internal_action_space*)learner->get_internal_type_erased_data_pointer_test_use_only();
    BOOST_CHECK_EQUAL(action_space != nullptr, true);

    {
      VW::multi_ex examples;

      examples.push_back(VW::read_example(vw, "| 1:0.1 2:0.12 3:0.13"));
      examples.push_back(VW::read_example(vw, "| a_1:0.5 a_2:0.65 a_3:0.12"));
      examples.push_back(VW::read_example(vw, "| a_4:0.8 a_5:0.32 a_6:0.15"));

      vw.predict(examples);

      const auto num_actions = examples.size();
      const auto& preds = examples[0]->pred.a_s;

      // Only d actions have non-zero scores.
      if (full_preds) { BOOST_CHECK_EQUAL(preds.size(), num_actions); }
      else
      {
        BOOST_CHECK_EQUAL(preds.size(), d);
      }
      BOOST_CHECK_SMALL(preds[0].score - 0.697270989f, FLOAT_TOL);
      BOOST_CHECK_EQUAL(preds[0].action, 1);

      BOOST_CHECK_SMALL(preds[1].score - 0.30272904f, FLOAT_TOL);
      BOOST_CHECK_EQUAL(preds[1].action, 2);

      if (full_preds)
      {
        BOOST_CHECK_SMALL(preds[2].score, FLOAT_TOL);
        BOOST_CHECK_EQUAL(preds[2].action, 0);
      }

      vw.finish_example(examples);
    }
    VW::finish(vw);
  }
}

BOOST_AUTO_TEST_CASE(check_spanner_results_epsilon_greedy)
{
  auto d = 2;
  float epsilon = 0.2f;

  std::vector<std::pair<VW::workspace*, bool>> vws;

  auto* vw_full_preds = VW::initialize("--cb_explore_adf --epsilon " + std::to_string(epsilon) +
          " --large_action_space --full_predictions --max_actions " + std::to_string(d) + " --quiet --random_seed 5",
      nullptr, false, nullptr, nullptr);

  vws.push_back({vw_full_preds, true});

  auto* vw_sparse_preds = VW::initialize("--cb_explore_adf --epsilon " + std::to_string(epsilon) +
          " --large_action_space --max_actions " + std::to_string(d) + " --quiet --random_seed 5",
      nullptr, false, nullptr, nullptr);

  vws.push_back({vw_sparse_preds, false});

  for (auto& vw_pair : vws)
  {
    auto& vw = *std::get<0>(vw_pair);
    auto full_preds = std::get<1>(vw_pair);

    {
      VW::multi_ex examples;

      examples.push_back(VW::read_example(vw, "0:1.0:0.5 | 1:0.1 2:0.12 3:0.13"));
      examples.push_back(VW::read_example(vw, "| a_1:0.5 a_2:0.65 a_3:0.12"));
      examples.push_back(VW::read_example(vw, "| a_4:0.8 a_5:0.32 a_6:0.15"));

      vw.learn(examples);
      vw.finish_example(examples);
    }

    {
      VW::multi_ex examples;

      examples.push_back(VW::read_example(vw, "| 1:0.1 2:0.12 3:0.13"));
      examples.push_back(VW::read_example(vw, "0:1.0:0.5 | a_1:0.5 a_2:0.65 a_3:0.12"));
      examples.push_back(VW::read_example(vw, "| a_4:0.8 a_5:0.32 a_6:0.15"));

      vw.learn(examples);
      vw.finish_example(examples);
    }

    {
      VW::multi_ex examples;

      examples.push_back(VW::read_example(vw, "| 1:0.1 2:0.12 3:0.13"));
      examples.push_back(VW::read_example(vw, "| a_1:0.5 a_2:0.65 a_3:0.12"));
      examples.push_back(VW::read_example(vw, "0:1.0:0.5 | a_4:0.8 a_5:0.32 a_6:0.15"));

      vw.learn(examples);
      vw.finish_example(examples);
    }

    std::vector<std::string> e_r;
    vw.l->get_enabled_reductions(e_r);
    if (std::find(e_r.begin(), e_r.end(), "cb_explore_adf_large_action_space") == e_r.end())
    { BOOST_FAIL("cb_explore_adf_large_action_space not found in enabled reductions"); }

    VW::LEARNER::multi_learner* learner =
        as_multiline(vw.l->get_learner_by_name_prefix("cb_explore_adf_large_action_space"));

    auto action_space = (internal_action_space*)learner->get_internal_type_erased_data_pointer_test_use_only();
    BOOST_CHECK_EQUAL(action_space != nullptr, true);

    {
      VW::multi_ex examples;

      examples.push_back(VW::read_example(vw, "| 1:0.1 2:0.12 3:0.13"));
      examples.push_back(VW::read_example(vw, "| a_1:0.5 a_2:0.65 a_3:0.12"));
      examples.push_back(VW::read_example(vw, "| a_4:0.8 a_5:0.32 a_6:0.15"));

      vw.predict(examples);

      const auto num_actions = examples.size();
      const auto& preds = examples[0]->pred.a_s;
      // Only d actions have non-zero scores.
      if (full_preds) { BOOST_CHECK_EQUAL(preds.size(), num_actions); }
      else
      {
        BOOST_CHECK_EQUAL(preds.size(), d);
      }

      size_t num_actions_non_zeroed = d;
      float epsilon_ur = epsilon / num_actions_non_zeroed;
      BOOST_CHECK_SMALL(preds[0].score - (epsilon_ur + (1.f - epsilon)), FLOAT_TOL);
      BOOST_CHECK_EQUAL(preds[0].action, 2);

      BOOST_CHECK_SMALL(preds[1].score - epsilon_ur, FLOAT_TOL);
      BOOST_CHECK_EQUAL(preds[1].action, 0);

      if (full_preds)
      {
        BOOST_CHECK_SMALL(preds[2].score, FLOAT_TOL);
        BOOST_CHECK_EQUAL(preds[2].action, 1);
      }

      vw.finish_example(examples);
    }
    VW::finish(vw);
  }
}

BOOST_AUTO_TEST_CASE(check_uniform_probabilities_before_learning)
{
  auto d = 2;
  std::vector<std::pair<VW::workspace*, bool>> vws;
  auto* vw_epsilon = VW::initialize("--cb_explore_adf --large_action_space --full_predictions --max_actions " +
          std::to_string(d) + " --quiet --random_seed 5 --noconstant",
      nullptr, false, nullptr, nullptr);

  vws.push_back({vw_epsilon, false});

  auto* vw_squarecb =
      VW::initialize("--cb_explore_adf --squarecb --large_action_space --full_predictions --max_actions " +
              std::to_string(d) + " --quiet --random_seed 5 --noconstant",
          nullptr, false, nullptr, nullptr);

  vws.push_back({vw_squarecb, true});

  for (auto& vw_pair : vws)
  {
    auto& vw = *std::get<0>(vw_pair);
    auto apply_diag_M = std::get<1>(vw_pair);

    VW::LEARNER::multi_learner* learner =
        as_multiline(vw.l->get_learner_by_name_prefix("cb_explore_adf_large_action_space"));

    {
      VW::multi_ex examples;

      examples.push_back(VW::read_example(vw, "| 1"));
      examples.push_back(VW::read_example(vw, "| 1"));
      examples.push_back(VW::read_example(vw, "| 1"));

      learner->predict(examples);

      const auto num_actions = examples.size();
      const auto& preds = examples[0]->pred.a_s;
      BOOST_CHECK_EQUAL(preds.size(), num_actions);
      for (const auto& pred : preds) { BOOST_CHECK_SMALL(pred.score - (1.f / 3.f), FLOAT_TOL); }

      vw.finish_example(examples);
    }
    VW::finish(vw);
  }
}

BOOST_AUTO_TEST_CASE(check_probabilities_when_d_is_larger)
{
  auto d = 3;
  auto& vw = *VW::initialize("--cb_explore_adf --large_action_space --full_predictions --max_actions " +
          std::to_string(d) + " --quiet --random_seed 5",
      nullptr, false, nullptr, nullptr);

  {
    VW::multi_ex examples;

    examples.push_back(VW::read_example(vw, "0:1.0:0.5 | 1:0.1 2:0.12 3:0.13"));
    examples.push_back(VW::read_example(vw, "| a_1:0.5 a_2:0.65 a_3:0.12"));
    examples.push_back(VW::read_example(vw, "| a_4:0.8 a_5:0.32 a_6:0.15"));

    vw.learn(examples);
    vw.finish_example(examples);
  }

  {
    VW::multi_ex examples;

    examples.push_back(VW::read_example(vw, "| 1:0.1 2:0.12 3:0.13"));
    examples.push_back(VW::read_example(vw, "0:1.0:0.5 | a_1:0.5 a_2:0.65 a_3:0.12"));
    examples.push_back(VW::read_example(vw, "| a_4:0.8 a_5:0.32 a_6:0.15"));

    vw.learn(examples);
    vw.finish_example(examples);
  }

  {
    VW::multi_ex examples;

    examples.push_back(VW::read_example(vw, "| 1:0.1 2:0.12 3:0.13"));
    examples.push_back(VW::read_example(vw, "| a_1:0.5 a_2:0.65 a_3:0.12"));
    examples.push_back(VW::read_example(vw, "0:1.0:0.5 | a_4:0.8 a_5:0.32 a_6:0.15"));

    vw.learn(examples);
    vw.finish_example(examples);
  }

  std::vector<std::string> e_r;
  vw.l->get_enabled_reductions(e_r);
  if (std::find(e_r.begin(), e_r.end(), "cb_explore_adf_large_action_space") == e_r.end())
  { BOOST_FAIL("cb_explore_adf_large_action_space not found in enabled reductions"); }

  VW::LEARNER::multi_learner* learner =
      as_multiline(vw.l->get_learner_by_name_prefix("cb_explore_adf_large_action_space"));

  auto action_space = (internal_action_space*)learner->get_internal_type_erased_data_pointer_test_use_only();
  BOOST_CHECK_EQUAL(action_space != nullptr, true);

  {
    VW::multi_ex examples;

    examples.push_back(VW::read_example(vw, "| 1:0.1 2:0.12 3:0.13"));
    examples.push_back(VW::read_example(vw, "| a_1:0.5 a_2:0.65 a_3:0.12"));
    examples.push_back(VW::read_example(vw, "| a_4:0.8 a_5:0.32 a_6:0.15"));

    vw.predict(examples);

    const auto num_actions = examples.size();
    const auto& preds = examples[0]->pred.a_s;
    BOOST_CHECK_EQUAL(preds.size(), num_actions);
    BOOST_CHECK_SMALL(preds[0].score - 0.966666639f, FLOAT_TOL);
    BOOST_CHECK_SMALL(preds[1].score - 0.0166666675f, FLOAT_TOL);
    BOOST_CHECK_SMALL(preds[2].score - 0.0166666675f, FLOAT_TOL);

    vw.finish_example(examples);
  }
  VW::finish(vw);
}

BOOST_AUTO_TEST_SUITE_END()