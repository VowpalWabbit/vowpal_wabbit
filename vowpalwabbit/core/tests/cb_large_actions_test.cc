// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "qr_decomposition.h"
#include "reductions/cb/details/large_action_space.h"
#include "vw/common/future_compat.h"
#include "vw/common/random.h"
#include "vw/core/constant.h"
#include "vw/core/numeric_casts.h"
#include "vw/core/reductions/cb/cb_explore_adf_common.h"
#include "vw/core/reductions/cb/cb_explore_adf_large_action_space.h"
#include "vw/core/vw.h"
#include "vw/test_common/test_common.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using internal_action_space =
    VW::cb_explore_adf::cb_explore_adf_base<VW::cb_explore_adf::cb_explore_adf_large_action_space<
        VW::cb_explore_adf::two_pass_svd_impl, VW::cb_explore_adf::one_rank_spanner_state>>;
using internal_action_space_op =
    VW::cb_explore_adf::cb_explore_adf_base<VW::cb_explore_adf::cb_explore_adf_large_action_space<
        VW::cb_explore_adf::one_pass_svd_impl, VW::cb_explore_adf::one_rank_spanner_state>>;

TEST(Las, CreationOfTheOgAMatrix)
{
  uint32_t d = 2;
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--large_action_space", "--max_actions",
      std::to_string(d), "--quiet", "--random_seed", "5", "--two_pass_svd"));

  std::vector<std::string> e_r;
  vw->l->get_enabled_learners(e_r);
  if (std::find(e_r.begin(), e_r.end(), "cb_explore_adf_large_action_space") == e_r.end())
  {
    FAIL() << "cb_explore_adf_large_action_space not found in enabled learners";
  }

  VW::LEARNER::learner* learner =
      require_multiline(vw->l->get_learner_by_name_prefix("cb_explore_adf_large_action_space"));

  auto action_space = (internal_action_space*)learner->get_internal_type_erased_data_pointer_test_use_only();

  EXPECT_EQ(action_space != nullptr, true);

  std::vector<Eigen::Triplet<float>> _triplets;

  {
    VW::multi_ex examples;

    examples.push_back(VW::read_example(*vw, "0:1.0:0.5 | 1:0.1 2:0.2 3:0.3"));

    std::vector<float> ft_values = {0.1f, 0.2f, 0.3f};

    vw->predict(examples);

    VW::cb_explore_adf::_test_only_generate_A(vw.get(), examples, _triplets, action_space->explore._A);

    auto num_actions = examples.size();
    EXPECT_EQ(num_actions, 1);

    uint64_t action_index = 0;
    auto* ex = examples[action_index];
    // test sanity - test assumes no shared features
    EXPECT_EQ(!VW::ec_is_example_header_cb(*ex), true);
    for (auto ns : ex->indices)
    {
      for (size_t i = 0; i < ex->feature_space[ns].indices.size(); i++)
      {
        auto ft_index = ex->feature_space[ns].indices[i];
        auto ft_value = ex->feature_space[ns].values[i];

        if (ns == VW::details::DEFAULT_NAMESPACE) { EXPECT_FLOAT_EQ(ft_value, ft_values[i]); }
        else if (ns == VW::details::CONSTANT_NAMESPACE) { EXPECT_FLOAT_EQ(ft_value, 1.f); }

        EXPECT_EQ(
            action_space->explore._A.coeffRef(action_index, (ft_index & vw->weights.dense_weights.mask())), ft_value);
      }
    }

    vw->finish_example(examples);
  }
}

TEST(Las, CheckInteractionsOnY)
{
  uint32_t d = 2;
  std::vector<std::pair<std::unique_ptr<VW::workspace>, bool>> vws;
  auto vw_no_interactions = VW::initialize(vwtest::make_args("--cb_explore_adf", "--large_action_space",
      "--max_actions", std::to_string(d), "--quiet", "--random_seed", "5", "--two_pass_svd"));

  vws.emplace_back(std::move(vw_no_interactions), false);

  auto vw_yes_interactions = VW::initialize(vwtest::make_args("--cb_explore_adf", "--large_action_space",
      "--max_actions", std::to_string(d), "--quiet", "--random_seed", "5", "-q", "::", "--two_pass_svd"));

  vws.emplace_back(std::move(vw_yes_interactions), true);

  size_t interactions_rows = 0;
  size_t non_interactions_rows = 0;

  for (auto& vw_pair : vws)
  {
    auto& vw = *std::get<0>(vw_pair);
    bool interactions = std::get<1>(vw_pair);

    std::vector<std::string> e_r;
    vw.l->get_enabled_learners(e_r);
    if (std::find(e_r.begin(), e_r.end(), "cb_explore_adf_large_action_space") == e_r.end())
    {
      FAIL() << "cb_explore_adf_large_action_space not found in enabled learners";
    }

    VW::LEARNER::learner* learner =
        require_multiline(vw.l->get_learner_by_name_prefix("cb_explore_adf_large_action_space"));

    auto action_space = (internal_action_space*)learner->get_internal_type_erased_data_pointer_test_use_only();

    EXPECT_EQ(action_space != nullptr, true);

    {
      VW::multi_ex examples;

      examples.push_back(VW::read_example(vw, "shared |U b c"));
      examples.push_back(VW::read_example(vw, "|f 1:0.1 2:0.12 3:0.13"));
      examples.push_back(VW::read_example(vw, "|f a_1:0.5 a_2:0.65 a_3:0.12"));
      examples.push_back(VW::read_example(vw, "|f a_4:0.8 a_5:0.32 a_6:0.15"));

      vw.predict(examples);

      std::set<uint64_t> non_zero_rows;
      for (int k = 0; k < action_space->explore.impl.Y.outerSize(); ++k)
      {
        for (Eigen::SparseMatrix<float>::InnerIterator it(action_space->explore.impl.Y, k); it; ++it)
        {
          non_zero_rows.emplace(it.row());
        }
      }

      if (!interactions) { non_interactions_rows = non_zero_rows.size(); }
      if (interactions) { interactions_rows = non_zero_rows.size(); }
      vw.finish_example(examples);
    }
  }
  EXPECT_GT(interactions_rows, non_interactions_rows);
}

TEST(Las, CheckInteractionsOnB)
{
  uint32_t d = 2;
  std::vector<std::pair<std::unique_ptr<VW::workspace>, bool>> vws;
  auto vw_no_interactions = VW::initialize(vwtest::make_args("--cb_explore_adf", "--large_action_space",
      "--max_actions", std::to_string(d), "--quiet", "--random_seed", "5", "--two_pass_svd"));

  vws.emplace_back(std::move(vw_no_interactions), false);

  auto vw_yes_interactions = VW::initialize(vwtest::make_args("--cb_explore_adf", "--large_action_space",
      "--max_actions", std::to_string(d), "--quiet", "--random_seed", "5", "-q", "::", "--two_pass_svd"));

  vws.emplace_back(std::move(vw_yes_interactions), true);

  Eigen::MatrixXf B_non_interactions;
  Eigen::MatrixXf B_interactions;

  for (auto& vw_pair : vws)
  {
    auto& vw = *std::get<0>(vw_pair);
    bool interactions = std::get<1>(vw_pair);

    std::vector<std::string> e_r;
    vw.l->get_enabled_learners(e_r);
    if (std::find(e_r.begin(), e_r.end(), "cb_explore_adf_large_action_space") == e_r.end())
    {
      FAIL() << "cb_explore_adf_large_action_space not found in enabled learners";
    }

    VW::LEARNER::learner* learner =
        require_multiline(vw.l->get_learner_by_name_prefix("cb_explore_adf_large_action_space"));

    auto action_space = (internal_action_space*)learner->get_internal_type_erased_data_pointer_test_use_only();

    EXPECT_EQ(action_space != nullptr, true);

    {
      VW::multi_ex examples;

      examples.push_back(VW::read_example(vw, "shared |U b c"));
      examples.push_back(VW::read_example(vw, "|f 1:0.1 2:0.12 3:0.13"));
      examples.push_back(VW::read_example(vw, "|f a_1:0.5 a_2:0.65 a_3:0.12"));
      examples.push_back(VW::read_example(vw, "|f a_4:0.8 a_5:0.32 a_6:0.15"));

      vw.predict(examples);

      if (!interactions) { B_non_interactions = action_space->explore.impl.B; }
      if (interactions) { B_interactions = action_space->explore.impl.B; }
      vw.finish_example(examples);
    }
  }
  EXPECT_EQ(B_interactions.isApprox(B_non_interactions), false);
}

TEST(Las, CheckAtTimesOmegaIsY)
{
  uint32_t d = 2;
  std::vector<std::pair<std::unique_ptr<VW::workspace>, bool>> vws;
  auto vw_epsilon = VW::initialize(vwtest::make_args("--cb_explore_adf", "--large_action_space", "--max_actions",
      std::to_string(d), "--quiet", "--random_seed", "5", "--two_pass_svd"));

  vws.emplace_back(std::move(vw_epsilon), false);

  auto vw_squarecb = VW::initialize(vwtest::make_args("--cb_explore_adf", "--squarecb", "--large_action_space",
      "--max_actions", std::to_string(d), "--quiet", "--random_seed", "5", "--two_pass_svd"));

  vws.emplace_back(std::move(vw_squarecb), true);

  for (auto& vw_pair : vws)
  {
    auto& vw = *std::get<0>(vw_pair);
    auto apply_diag_M = std::get<1>(vw_pair);

    std::vector<std::string> e_r;
    vw.l->get_enabled_learners(e_r);
    if (std::find(e_r.begin(), e_r.end(), "cb_explore_adf_large_action_space") == e_r.end())
    {
      FAIL() << "cb_explore_adf_large_action_space not found in enabled learners";
    }

    VW::LEARNER::learner* learner =
        require_multiline(vw.l->get_learner_by_name_prefix("cb_explore_adf_large_action_space"));

    auto action_space = (internal_action_space*)learner->get_internal_type_erased_data_pointer_test_use_only();

    EXPECT_EQ(action_space != nullptr, true);

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

      action_space->explore.shrink_fact_config.calculate_shrink_factor(
          0, d, examples[0]->pred.a_s, action_space->explore.shrink_factors);

      VW::cb_explore_adf::_test_only_generate_A(&vw, examples, _triplets, action_space->explore._A);
      action_space->explore.impl.generate_Y(examples, action_space->explore.shrink_factors);

      uint64_t num_actions = examples[0]->pred.a_s.size();

      // Generate Omega
      std::vector<Eigen::Triplet<float>> omega_triplets;

      float seed = (vw.get_random_state()->get_random() + 1) * 10.f;

      for (uint64_t action_index = 0; action_index < num_actions; action_index++)
      {
        auto* ex = examples[action_index];
        // test sanity - test assumes no shared features
        EXPECT_EQ(!VW::ec_is_example_header_cb(*ex), true);
        for (auto ns : ex->indices)
        {
          _UNUSED(ns);
          for (uint64_t col = 0; col < d; col++)
          {
            uint64_t combined_index = action_index + col + seed;
            auto mm = VW::details::merand48_boxmuller(combined_index);
            omega_triplets.push_back(Eigen::Triplet<float>(action_index, col, mm));
          }
        }
      }

      Eigen::SparseMatrix<float> Omega(num_actions, d);
      Omega.setFromTriplets(omega_triplets.begin(), omega_triplets.end(),
          [](const float& a, const float& b)
          {
            assert(a == b);
            // Make release build warnings happy.
            _UNUSED(a);
            return b;
          });

      Eigen::SparseMatrix<float> diag_M(num_actions, num_actions);

      if (apply_diag_M)
      {
        for (Eigen::Index i = 0;
             i < VW::cast_unsigned_to_signed<Eigen::Index>(action_space->explore.shrink_factors.size()); i++)
        {
          diag_M.coeffRef(i, i) = action_space->explore.shrink_factors[i];
        }
      }
      else { diag_M.setIdentity(); }

      Eigen::SparseMatrix<float> Yd(action_space->explore.impl.Y.rows(), d);

      Yd = action_space->explore._A.transpose() * diag_M * Omega;
      // Orthonormalize Yd
      VW::gram_schmidt(Yd);
      EXPECT_EQ(Yd.isApprox(action_space->explore.impl.Y), true);

      vw.finish_example(examples);
    }
  }
}

TEST(Las, CheckATimesYIsB)
{
  uint32_t d = 2;
  std::vector<std::pair<std::unique_ptr<VW::workspace>, bool>> vws;
  auto vw_epsilon = VW::initialize(vwtest::make_args("--cb_explore_adf", "--large_action_space", "--max_actions",
      std::to_string(d), "--quiet", "--random_seed", "5", "--two_pass_svd"));

  vws.emplace_back(std::move(vw_epsilon), false);

  auto vw_squarecb = VW::initialize(vwtest::make_args("--cb_explore_adf", "--squarecb", "--large_action_space",
      "--max_actions", std::to_string(d), "--quiet", "--random_seed", "5", "--two_pass_svd"));

  vws.emplace_back(std::move(vw_squarecb), true);

  for (auto& vw_pair : vws)
  {
    auto& vw = *std::get<0>(vw_pair);
    std::vector<Eigen::Triplet<float>> _triplets;
    auto apply_diag_M = std::get<1>(vw_pair);

    std::vector<std::string> e_r;
    vw.l->get_enabled_learners(e_r);
    if (std::find(e_r.begin(), e_r.end(), "cb_explore_adf_large_action_space") == e_r.end())
    {
      FAIL() << "cb_explore_adf_large_action_space not found in enabled learners";
    }

    VW::LEARNER::learner* learner =
        require_multiline(vw.l->get_learner_by_name_prefix("cb_explore_adf_large_action_space"));

    auto action_space = (internal_action_space*)learner->get_internal_type_erased_data_pointer_test_use_only();

    EXPECT_EQ(action_space != nullptr, true);

    {
      VW::multi_ex examples;

      examples.push_back(VW::read_example(vw, "0:1.0:0.5 | 1:0.1 2:0.12 3:0.13"));
      examples.push_back(VW::read_example(vw, "| a_1:0.5 a_2:0.65 a_3:0.12"));

      vw.predict(examples);

      VW::cb_explore_adf::_test_only_generate_A(&vw, examples, _triplets, action_space->explore._A);
      action_space->explore.shrink_fact_config.calculate_shrink_factor(
          0, d, examples[0]->pred.a_s, action_space->explore.shrink_factors);

      action_space->explore.impl.generate_Y(examples, action_space->explore.shrink_factors);
      action_space->explore.impl.generate_B(examples, action_space->explore.shrink_factors);

      auto num_actions = examples[0]->pred.a_s.size();
      Eigen::SparseMatrix<float> diag_M(num_actions, num_actions);

      if (apply_diag_M)
      {
        for (Eigen::Index i = 0;
             i < VW::cast_unsigned_to_signed<Eigen::Index>(action_space->explore.shrink_factors.size()); i++)
        {
          diag_M.coeffRef(i, i) = action_space->explore.shrink_factors[i];
        }
      }
      else { diag_M.setIdentity(); }

      Eigen::MatrixXf B = diag_M * action_space->explore._A * action_space->explore.impl.Y;
      EXPECT_EQ(B.isApprox(action_space->explore.impl.B), true);

      vw.finish_example(examples);
    }
  }
}

TEST(Las, CheckBTimesPIsZ)
{
  uint32_t d = 2;

  std::vector<std::pair<std::unique_ptr<VW::workspace>, bool>> vws;
  auto vw_epsilon = VW::initialize(vwtest::make_args("--cb_explore_adf", "--large_action_space", "--max_actions",
      std::to_string(d), "--quiet", "--random_seed", "5", "--two_pass_svd"));

  vws.emplace_back(std::move(vw_epsilon), false);

  auto vw_squarecb = VW::initialize(vwtest::make_args("--cb_explore_adf", "--squarecb", "--large_action_space",
      "--max_actions", std::to_string(d), "--quiet", "--random_seed", "5", "--two_pass_svd"));

  vws.emplace_back(std::move(vw_squarecb), true);

  for (auto& vw_pair : vws)
  {
    auto& vw = *std::get<0>(vw_pair);
    std::vector<Eigen::Triplet<float>> _triplets;

    std::vector<std::string> e_r;
    vw.l->get_enabled_learners(e_r);
    if (std::find(e_r.begin(), e_r.end(), "cb_explore_adf_large_action_space") == e_r.end())
    {
      FAIL() << "cb_explore_adf_large_action_space not found in enabled learners";
    }

    VW::LEARNER::learner* learner =
        require_multiline(vw.l->get_learner_by_name_prefix("cb_explore_adf_large_action_space"));

    auto action_space = (internal_action_space*)learner->get_internal_type_erased_data_pointer_test_use_only();

    EXPECT_EQ(action_space != nullptr, true);

    {
      float seed = (vw.get_random_state()->get_random() + 1) * 10.f;

      VW::multi_ex examples;

      examples.push_back(VW::read_example(vw, "0:1.0:0.5 | 1:0.1 2:0.12 3:0.13"));
      examples.push_back(VW::read_example(vw, "| a_1:0.5 a_2:0.65 a_3:0.12"));

      vw.predict(examples);

      action_space->explore.shrink_fact_config.calculate_shrink_factor(
          0, d, examples[0]->pred.a_s, action_space->explore.shrink_factors);
      VW::cb_explore_adf::_test_only_generate_A(&vw, examples, _triplets, action_space->explore._A);
      action_space->explore.impl.generate_Y(examples, action_space->explore.shrink_factors);
      action_space->explore.impl.generate_B(examples, action_space->explore.shrink_factors);
      VW::cb_explore_adf::generate_Z(examples, action_space->explore.impl.Z, action_space->explore.impl.B, d, seed);

      Eigen::MatrixXf P(d, d);

      for (size_t row = 0; row < d; row++)
      {
        for (size_t col = 0; col < d; col++)
        {
          auto combined_index = row + col + static_cast<uint64_t>(seed);
          auto mm = VW::details::merand48_boxmuller(combined_index);
          P(row, col) = mm;
        }
      }

      Eigen::MatrixXf Zp = action_space->explore.impl.B * P;
      VW::gram_schmidt(Zp);
      EXPECT_EQ(Zp.isApprox(action_space->explore.impl.Z), true);
      vw.finish_example(examples);
    }
  }
}

template <typename T>
void check_final_truncated_SVD_validity_impl(VW::workspace& vw,
    VW::cb_explore_adf::cb_explore_adf_base<VW::cb_explore_adf::cb_explore_adf_large_action_space<T,
        VW::cb_explore_adf::one_rank_spanner_state>>* action_space,
    bool apply_diag_M, std::vector<Eigen::Triplet<float>> _triplets, uint64_t d)
{
  EXPECT_EQ(action_space != nullptr, true);

  {
    VW::multi_ex examples;

    examples.push_back(VW::read_example(vw, "|f 1:0.1 2:0.12 3:0.13"));
    examples.push_back(VW::read_example(vw, "|f a_1:0.5 a_2:0.65 a_3:0.12"));
    examples.push_back(VW::read_example(vw, "|f a_4:0.8 a_5:0.32 a_6:0.15"));
    action_space->explore._populate_all_testing_components();

    vw.predict(examples);

    action_space->explore.shrink_fact_config.calculate_shrink_factor(
        0, d, examples[0]->pred.a_s, action_space->explore.shrink_factors);
    action_space->explore.randomized_SVD(examples);

    VW::cb_explore_adf::_test_only_generate_A(&vw, examples, _triplets, action_space->explore._A);
    {
      Eigen::FullPivLU<Eigen::MatrixXf> lu_decomp(action_space->explore._A);
      auto rank = lu_decomp.rank();
      // for test set actual rank of A
      action_space->explore._test_only_set_rank(rank);
      // should have a rank larger than 1 for the test
      EXPECT_GT(rank, 1);
    }

    vw.predict(examples);

    action_space->explore.shrink_fact_config.calculate_shrink_factor(
        0, d, examples[0]->pred.a_s, action_space->explore.shrink_factors);
    action_space->explore.randomized_SVD(examples);

    auto num_actions = examples.size();

    // U dimensions should be K x d
    EXPECT_EQ(action_space->explore.U.rows(), num_actions);
    EXPECT_EQ(action_space->explore.U.cols(), d);

    // truncated randomized SVD reconstruction
    for (int i = 0; i < action_space->explore.U.cols(); ++i)
    {
      EXPECT_NEAR(1.f - action_space->explore.U.col(i).norm(), 0.f, vwtest::EXPLICIT_FLOAT_TOL);
      for (int j = 0; j < i; ++j)
      {
        EXPECT_NEAR(
            action_space->explore.U.col(i).dot(action_space->explore.U.col(j)), 0.f, vwtest::EXPLICIT_FLOAT_TOL);
      }
    }

    for (int i = 0; i < action_space->explore._V.cols(); ++i)
    {
      EXPECT_NEAR(1.f - action_space->explore._V.col(i).norm(), 0.f, vwtest::EXPLICIT_FLOAT_TOL);
      for (int j = 0; j < i; ++j)
      {
        EXPECT_NEAR(
            action_space->explore._V.col(i).dot(action_space->explore._V.col(j)), 0.f, vwtest::EXPLICIT_FLOAT_TOL);
      }
    }

    Eigen::SparseMatrix<float> diag_M(num_actions, num_actions);

    if (apply_diag_M)
    {
      for (Eigen::Index i = 0;
           i < VW::cast_unsigned_to_signed<Eigen::Index>(action_space->explore.shrink_factors.size()); i++)
      {
        diag_M.coeffRef(i, i) = action_space->explore.shrink_factors[i];
      }
    }
    else { diag_M.setIdentity(); }

    EXPECT_NEAR(
        ((diag_M * action_space->explore._A) -
            action_space->explore.U * action_space->explore.S.asDiagonal() * action_space->explore._V.transpose())
            .norm(),
        0.f, vwtest::EXPLICIT_FLOAT_TOL);

    // compare singular values with actual SVD singular values
    Eigen::MatrixXf A_dense = diag_M * action_space->explore._A;
    Eigen::JacobiSVD<Eigen::MatrixXf> svd(A_dense, Eigen::ComputeThinU | Eigen::ComputeThinV);
    Eigen::VectorXf S = svd.singularValues();

    for (int i = 0; i < action_space->explore.S.rows(); i++)
    {
      EXPECT_NEAR(S(i) - action_space->explore.S(i), 0.f, vwtest::EXPLICIT_FLOAT_TOL);
    }

    vw.finish_example(examples);
  }
}

TEST(Las, CheckFinalTruncatedSVDValidity)
{
  uint32_t d = 3;

  std::vector<std::tuple<std::unique_ptr<VW::workspace>, bool, VW::cb_explore_adf::implementation_type>> vws;

  auto vw_w_interactions_sq = VW::initialize(vwtest::make_args("--cb_explore_adf", "--squarecb", "--large_action_space",
      "--max_actions", std::to_string(d), "--quiet", "--random_seed", "5", "-q", "::", "--two_pass_svd"));

  vws.emplace_back(std::move(vw_w_interactions_sq), true, VW::cb_explore_adf::implementation_type::two_pass_svd);

  auto vw_w_interactions_sq_sparse_weights =
      VW::initialize(vwtest::make_args("--cb_explore_adf", "--squarecb", "--sparse_weights", "--large_action_space",
          "--max_actions", std::to_string(d), "--quiet", "--random_seed", "5", "-q", "::", "--two_pass_svd"));

  vws.emplace_back(
      std::move(vw_w_interactions_sq_sparse_weights), true, VW::cb_explore_adf::implementation_type::two_pass_svd);

  for (auto& vw_pair : vws)
  {
    auto& vw = *std::get<0>(vw_pair);
    std::vector<Eigen::Triplet<float>> _triplets;
    auto apply_diag_M = std::get<1>(vw_pair);
    auto impl_type = std::get<2>(vw_pair);

    std::vector<std::string> e_r;
    vw.l->get_enabled_learners(e_r);
    if (std::find(e_r.begin(), e_r.end(), "cb_explore_adf_large_action_space") == e_r.end())
    {
      FAIL() << "cb_explore_adf_large_action_space not found in enabled learners";
    }

    VW::LEARNER::learner* learner =
        require_multiline(vw.l->get_learner_by_name_prefix("cb_explore_adf_large_action_space"));

    if (impl_type == VW::cb_explore_adf::implementation_type::two_pass_svd)
    {
      auto action_space = (internal_action_space*)learner->get_internal_type_erased_data_pointer_test_use_only();
      check_final_truncated_SVD_validity_impl<VW::cb_explore_adf::two_pass_svd_impl>(
          vw, action_space, apply_diag_M, _triplets, d);
    }
    else { FAIL() << "test for implementation type not implemented"; }
  }
}

TEST(Las, CheckShrinkFactor)
{
  uint32_t d = 2;
  std::vector<std::pair<std::unique_ptr<VW::workspace>, bool>> vws;
  auto vw_epsilon = VW::initialize(vwtest::make_args("--cb_explore_adf", "--large_action_space", "--max_actions",
      std::to_string(d), "--quiet", "--random_seed", "5", "--two_pass_svd"));

  vws.emplace_back(std::move(vw_epsilon), false);

  auto vw_squarecb = VW::initialize(vwtest::make_args("--cb_explore_adf", "--squarecb", "--large_action_space",
      "--max_actions", std::to_string(d), "--quiet", "--random_seed", "5", "--two_pass_svd"));

  vws.emplace_back(std::move(vw_squarecb), true);

  for (auto& vw_pair : vws)
  {
    auto& vw = *std::get<0>(vw_pair);
    auto apply_diag_M = std::get<1>(vw_pair);

    std::vector<std::string> e_r;
    vw.l->get_enabled_learners(e_r);
    if (std::find(e_r.begin(), e_r.end(), "cb_explore_adf_large_action_space") == e_r.end())
    {
      FAIL() << "cb_explore_adf_large_action_space not found in enabled learners";
    }

    VW::LEARNER::learner* learner =
        require_multiline(vw.l->get_learner_by_name_prefix("cb_explore_adf_large_action_space"));

    auto action_space = (internal_action_space*)learner->get_internal_type_erased_data_pointer_test_use_only();

    EXPECT_EQ(action_space != nullptr, true);

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

    EXPECT_EQ(num_actions, 7);

    action_space->explore.shrink_fact_config.calculate_shrink_factor(
        0, d, examples[0]->pred.a_s, action_space->explore.shrink_factors);

    Eigen::SparseMatrix<float> diag_M(num_actions, num_actions);
    Eigen::SparseMatrix<float> identity_diag_M(num_actions, num_actions);
    identity_diag_M.setIdentity();

    for (Eigen::Index i = 0; i < VW::cast_unsigned_to_signed<Eigen::Index>(action_space->explore.shrink_factors.size());
         i++)
    {
      diag_M.coeffRef(i, i) = action_space->explore.shrink_factors[i];
    }

    if (apply_diag_M) { EXPECT_EQ(diag_M.isApprox(identity_diag_M), false); }
    else { EXPECT_EQ(diag_M.isApprox(identity_diag_M), true); }

    vw.finish_example(examples);
  }
}