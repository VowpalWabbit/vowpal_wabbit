// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "reductions/cb/details/large_action_space.h"
#include "test_common.h"
#include "vw/core/rand48.h"
#include "vw/core/rand_state.h"
#include "vw/core/reductions/cb/cb_explore_adf_common.h"
#include "vw/core/reductions/cb/cb_explore_adf_large_action_space.h"
#include "vw/core/vw.h"
#include "../../vowpalwabbit/redsvd-0.2.0/src/redsvd.hpp"

#include <boost/test/unit_test.hpp>

using internal_action_space =
    VW::cb_explore_adf::cb_explore_adf_base<VW::cb_explore_adf::cb_explore_adf_large_action_space>;

void check_matrix_multiplication(
    VW::workspace& vw, internal_action_space* action_space, const VW::multi_ex& examples, uint64_t d)
{
  // assuming no shared features or interactions at this point since doing manual calculations by re-using the raw
  // unprocessed example

  auto num_actions = examples.size();
  // gather the feature indexes

  for (size_t action_index = 0; action_index < num_actions; action_index++)
  {
    auto* ex = examples[action_index];

    std::vector<feature_index> ft_indexes;
    // test sanity - test assumes no shared features
    BOOST_CHECK_EQUAL(!CB::ec_is_example_header(*ex), true);
    for (auto ns : ex->indices)
    {
      for (auto ft_index : ex->feature_space[ns].indices) { ft_indexes.push_back(ft_index); }
    }

    std::vector<weight> ft_weights;
    for (auto ft_index : ft_indexes) { ft_weights.push_back(vw.weights.dense_weights[ft_index]); }

    uint64_t seed = vw.get_random_state()->get_current_state();

    // checking the multiplication of matrix A of vw action weights dim(#actions x #features) which is a sparse
    // matrix, with the random matrix with Gaussian entries of dim(#features x d)
    for (uint64_t gaussian_col = 0; gaussian_col < d; gaussian_col++)
    {
      float dot_product = 0;
      for (auto ft_index : ft_indexes)
      {
        auto gaussian_row = ft_index;
        auto combined_index = gaussian_row + gaussian_col + seed;
        auto lazyg_cell = merand48_boxmuller(combined_index);
        auto Action_matrix_cell_weight = vw.weights.dense_weights[ft_index];
        dot_product += Action_matrix_cell_weight * lazyg_cell;
      }
      BOOST_CHECK_EQUAL(dot_product, action_space->explore.Q(action_index, gaussian_col));
    }
  }
}

BOOST_AUTO_TEST_CASE(creation_of_Q_with_lazy_gaussian)
{
  auto d = 2;
  auto& vw = *VW::initialize("--cb_explore_adf --large_action_space --max_actions " + std::to_string(d) + " --quiet",
      nullptr, false, nullptr, nullptr);

  {
    VW::multi_ex examples;

    examples.push_back(VW::read_example(vw, "0:1.0:0.5 | 1 2 3"));

    vw.learn(examples);
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

    examples.push_back(VW::read_example(vw, "0:1.0:0.5 | 1 2 3"));

    vw.predict(examples);

    action_space->explore.generate_Q(examples);

    check_matrix_multiplication(vw, action_space, examples, d);

    // check that feature values do not affect the matrix manipulation as they are not part of it
    // push new example but since we did not run predict, Q should be the same
    examples.clear();
    examples.push_back(VW::read_example(vw, "0:1.0:0.5 | 1:100 2:100 3:100"));
    vw.predict(examples);
    action_space->explore.generate_Q(examples);
    check_matrix_multiplication(vw, action_space, examples, d);
  }
}

BOOST_AUTO_TEST_CASE(creation_of_the_og_A_matrix)
{
  auto d = 2;
  auto& vw = *VW::initialize("--cb_explore_adf --large_action_space --max_actions " + std::to_string(d) + " --quiet",
      nullptr, false, nullptr, nullptr);

  {
    VW::multi_ex examples;

    examples.push_back(VW::read_example(vw, "0:1.0:0.5 | 1 2 3"));

    vw.learn(examples);
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

    examples.push_back(VW::read_example(vw, "0:1.0:0.5 | 1 2 3"));

    vw.predict(examples);

    action_space->explore.generate_A(examples);

    auto num_actions = examples.size();
    // gather the feature indexes

    for (size_t action_index = 0; action_index < num_actions; action_index++)
    {
      auto* ex = examples[action_index];
      // test sanity - test assumes no shared features
      BOOST_CHECK_EQUAL(!CB::ec_is_example_header(*ex), true);
      for (auto ns : ex->indices)
      {
        for (auto ft_index : ex->feature_space[ns].indices)
        {
          BOOST_CHECK_EQUAL(
              action_space->explore.A.coeffRef(action_index, ft_index), vw.weights.dense_weights[ft_index]);
        }
      }
    }
  }
}

BOOST_AUTO_TEST_CASE(check_A_times_Omega_is_Q)
{
  auto d = 2;
  auto& vw = *VW::initialize("--cb_explore_adf --large_action_space --max_actions " + std::to_string(d) + " --quiet",
      nullptr, false, nullptr, nullptr);

  {
    VW::multi_ex examples;

    examples.push_back(VW::read_example(vw, "0:1.0:0.5 | 1 2 3"));
    examples.push_back(VW::read_example(vw, "| a_1 a_2 a_3"));

    vw.learn(examples);
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

    examples.push_back(VW::read_example(vw, "0:1.0:0.5 | 1 2 3"));
    examples.push_back(VW::read_example(vw, "| a_1 a_2 a_3"));

    vw.predict(examples);

    action_space->explore.generate_A(examples);
    action_space->explore.generate_Q(examples);

    uint64_t num_actions = examples[0]->pred.a_s.size();

    // Generate Omega
    std::vector<Eigen::Triplet<float>> omega_triplets;
    uint64_t max_ft_index = 0;
    uint64_t seed = vw.get_random_state()->get_current_state();

    for (size_t action_index = 0; action_index < num_actions; action_index++)
    {
      auto* ex = examples[action_index];
      // test sanity - test assumes no shared features
      BOOST_CHECK_EQUAL(!CB::ec_is_example_header(*ex), true);
      for (auto ns : ex->indices)
      {
        for (size_t col = 0; col < d; col++)
        {
          for (auto ft_index : ex->feature_space[ns].indices)
          {
            auto combined_index = ft_index + col + seed;
            auto mm = merand48_boxmuller(combined_index);
            omega_triplets.push_back(Eigen::Triplet<float>(ft_index, col, mm));
            if (ft_index > max_ft_index) { max_ft_index = ft_index; }
          }
        }
      }
    }

    Eigen::SparseMatrix<float> Omega(max_ft_index + 1, d);
    Omega.setFromTriplets(omega_triplets.begin(), omega_triplets.end(), [](const float& a, const float& b) {
      assert(a == b);
      return b;
    });

    Eigen::MatrixXf Qd(num_actions, d);
    Qd = action_space->explore.A * Omega;
    BOOST_CHECK_EQUAL(Qd.isApprox(action_space->explore.Q), true);
  }
}

BOOST_AUTO_TEST_CASE(check_At_times_Omega_is_Y)
{
  auto d = 2;
  auto& vw = *VW::initialize("--cb_explore_adf --large_action_space --max_actions " + std::to_string(d) + " --quiet",
      nullptr, false, nullptr, nullptr);

  {
    VW::multi_ex examples;

    examples.push_back(VW::read_example(vw, "0:1.0:0.5 | 1 2 3"));
    examples.push_back(VW::read_example(vw, "| a_1 a_2 a_3"));

    vw.learn(examples);
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

    examples.push_back(VW::read_example(vw, "0:1.0:0.5 | 1 2 3"));
    examples.push_back(VW::read_example(vw, "| a_1 a_2 a_3"));

    vw.predict(examples);

    action_space->explore.generate_A(examples);
    action_space->explore.generate_Y(examples);

    uint64_t num_actions = examples[0]->pred.a_s.size();

    // Generate Omega
    std::vector<Eigen::Triplet<float>> omega_triplets;
    uint64_t max_ft_index = 0;
    uint64_t seed = vw.get_random_state()->get_current_state();

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

    // for (int k = 0; k < Omega.outerSize(); ++k)
    //   for (Eigen::SparseMatrix<float>::InnerIterator it(Omega, k); it; ++it)
    //   {
    //     std::cout << "row: " << it.row() << std::endl;
    //     std::cout << "col: " << it.col() << std::endl;
    //   }
    Eigen::SparseMatrix<float> Yd(action_space->explore.Y.rows(), d);
    Yd = action_space->explore.A.transpose() * Omega;
    BOOST_CHECK_EQUAL(Yd.isApprox(action_space->explore.Y), true);
  }
}

BOOST_AUTO_TEST_CASE(check_A_times_Y_is_B)
{
  auto d = 2;
  auto& vw = *VW::initialize("--cb_explore_adf --large_action_space --max_actions " + std::to_string(d) + " --quiet",
      nullptr, false, nullptr, nullptr);

  {
    VW::multi_ex examples;

    examples.push_back(VW::read_example(vw, "0:1.0:0.5 | 1 2 3"));
    examples.push_back(VW::read_example(vw, "| a_1 a_2 a_3"));

    vw.learn(examples);
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

    examples.push_back(VW::read_example(vw, "0:1.0:0.5 | 1 2 3"));
    examples.push_back(VW::read_example(vw, "| a_1 a_2 a_3"));

    vw.predict(examples);

    action_space->explore.generate_A(examples);
    action_space->explore.generate_Y(examples);
    action_space->explore.generate_B(examples);

    Eigen::MatrixXf B = action_space->explore.A * action_space->explore.Y;
    BOOST_CHECK_EQUAL(B.isApprox(action_space->explore.B), true);
  }
}

BOOST_AUTO_TEST_CASE(check_B_times_P_is_Z)
{
  auto d = 2;
  auto& vw = *VW::initialize("--cb_explore_adf --large_action_space --max_actions " + std::to_string(d) + " --quiet",
      nullptr, false, nullptr, nullptr);

  {
    VW::multi_ex examples;

    examples.push_back(VW::read_example(vw, "0:1.0:0.5 | 1 2 3"));
    examples.push_back(VW::read_example(vw, "| a_1 a_2 a_3"));

    vw.learn(examples);
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

    examples.push_back(VW::read_example(vw, "0:1.0:0.5 | 1 2 3"));
    examples.push_back(VW::read_example(vw, "| a_1 a_2 a_3"));

    vw.predict(examples);

    action_space->explore.generate_A(examples);
    action_space->explore.generate_Y(examples);
    action_space->explore.generate_B(examples);
    action_space->explore.generate_Z(examples);

    Eigen::MatrixXf P(d, d);

    uint64_t seed = vw.get_random_state()->get_current_state();

    for (size_t row = 0; row < d; row++)
    {
      for (size_t col = 0; col < d; col++)
      {
        auto combined_index = row + col + seed;
        auto mm = merand48_boxmuller(combined_index);
        P(row, col) = mm;
      }
    }

    Eigen::MatrixXf Zp = action_space->explore.B * P;
    BOOST_CHECK_EQUAL(Zp.isApprox(action_space->explore.Z), true);
  }
}

BOOST_AUTO_TEST_CASE(check_final_U_dimensions)
{
  auto d = 2;
  auto& vw = *VW::initialize("--cb_explore_adf --large_action_space --max_actions " + std::to_string(d) + " --quiet",
      nullptr, false, nullptr, nullptr);

  {
    VW::multi_ex examples;

    examples.push_back(VW::read_example(vw, "0:1.0:0.5 | 1 2 3"));
    examples.push_back(VW::read_example(vw, "| a_1 a_2 a_3"));
    examples.push_back(VW::read_example(vw, "| a_4 a_5 a_6"));

    vw.learn(examples);
  }

  {
    VW::multi_ex examples;

    examples.push_back(VW::read_example(vw, "| 1 2 3"));
    examples.push_back(VW::read_example(vw, "0:1.0:0.5 | a_1 a_2 a_3"));
    examples.push_back(VW::read_example(vw, "| a_4 a_5 a_6"));

    vw.learn(examples);
  }

  {
    VW::multi_ex examples;

    examples.push_back(VW::read_example(vw, "| 1 2 3"));
    examples.push_back(VW::read_example(vw, "| a_1 a_2 a_3"));
    examples.push_back(VW::read_example(vw, "0:1.0:0.5 | a_4 a_5 a_6"));

    vw.learn(examples);
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

    examples.push_back(VW::read_example(vw, "| 1 2 3"));
    examples.push_back(VW::read_example(vw, "| a_1 a_2 a_3"));
    examples.push_back(VW::read_example(vw, "| a_4 a_5 a_6"));

    vw.predict(examples);

    auto num_actions = examples.size();

    // U dimensions should be K x d
    BOOST_CHECK_EQUAL(action_space->explore.U.rows(), num_actions);
    BOOST_CHECK_EQUAL(action_space->explore.U.cols(), d);
  }
}

void generate_random_matrix(Eigen::MatrixXf& mat, uint64_t seed)
{
  for (size_t row = 0; row < mat.rows(); row++)
  {
    for (size_t col = 0; col < mat.cols(); col++)
    {
      auto combined_index = row + col + seed;
      auto mm = merand48_boxmuller(combined_index);
      mat(row, col) = mm;
    }
  }
}

void generate_random_vector(Eigen::VectorXf& vec, uint64_t seed)
{
  for (size_t row = 0; row < vec.rows(); row++)
  {
    auto combined_index = row + seed;
    auto mm = merand48_boxmuller(combined_index);
    vec(row) = mm;
  }
}

BOOST_AUTO_TEST_CASE(check_final_truncated_SVD_validity)
{
  auto d = 3;
  auto& vw = *VW::initialize(
      "--cb_explore_adf --noconstant --large_action_space --max_actions " + std::to_string(d) + " --quiet", nullptr,
      false, nullptr, nullptr);

  uint64_t num_actions = 4;  // rows
  uint64_t num_fts = 5;      // cols
  uint64_t seed = 0;

  vw.get_random_state()->set_random_state(0);
  vw.get_random_state()->get_and_update_random();

  Eigen::MatrixXf U(num_actions, d);
  Eigen::MatrixXf V(num_fts, d);
  seed = vw.get_random_state()->get_and_update_random() * 100.f;
  generate_random_matrix(U, seed);
  REDSVD::Util::processGramSchmidt(U);
  seed = vw.get_random_state()->get_and_update_random() * 100.f;
  generate_random_matrix(V, seed);
  REDSVD::Util::processGramSchmidt(V);

  Eigen::VectorXf S(d);
  for (int i = 0; i < d; ++i) { S(i) = d - i; }
  Eigen::MatrixXf A = U * S.asDiagonal() * V.transpose();

  std::vector<std::string> e_r;
  vw.l->get_enabled_reductions(e_r);
  if (std::find(e_r.begin(), e_r.end(), "cb_explore_adf_large_action_space") == e_r.end())
  { BOOST_FAIL("cb_explore_adf_large_action_space not found in enabled reductions"); }

  VW::LEARNER::multi_learner* learner =
      as_multiline(vw.l->get_learner_by_name_prefix("cb_explore_adf_large_action_space"));

  auto action_space = (internal_action_space*)learner->get_internal_type_erased_data_pointer_test_use_only();
  BOOST_CHECK_EQUAL(action_space != nullptr, true);

  action_space->explore._populate_all_SVD_components();

  // need to populate examples with the indexes and the vw weights with the indexes values
  // then I need to proceed to make this sparse

  multi_ex examples;
  action_space->explore.SVD(A, examples);

  constexpr float FLOAT_TOL = 0.0001f;

  // truncated randomized SVD reconstruction
  for (int i = 0; i < action_space->explore.U.cols(); ++i)
  {
    BOOST_CHECK_CLOSE(1.f, action_space->explore.U.col(i).norm(), FLOAT_TOL);
    for (int j = 0; j < i; ++j)
    { BOOST_CHECK_SMALL(action_space->explore.U.col(i).dot(action_space->explore.U.col(j)), FLOAT_TOL); }
  }

  for (int i = 0; i < action_space->explore._V.cols(); ++i)
  {
    BOOST_CHECK_CLOSE(1.f, action_space->explore._V.col(i).norm(), FLOAT_TOL);
    for (int j = 0; j < i; ++j)
    { BOOST_CHECK_SMALL(action_space->explore._V.col(i).dot(action_space->explore._V.col(j)), FLOAT_TOL); }
  }

  BOOST_CHECK_SMALL(
      (A - action_space->explore.U * action_space->explore._S.asDiagonal() * action_space->explore._V.transpose())
          .norm(),
      FLOAT_TOL);
}