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
    for (int k = 0; k < Omega.outerSize(); ++k)
      for (Eigen::SparseMatrix<float>::InnerIterator it(Omega, k); it; ++it)
      {
        std::cout << "row: " << it.row() << std::endl;  // row index
        std::cout << "col: " << it.col() << std::endl;  // col index (here it is equal to k)
      }
    Eigen::SparseMatrix<float> Yd(action_space->explore.Y.rows(), d);
    Yd = action_space->explore.A.transpose() * Omega;
    BOOST_CHECK_EQUAL(Yd.isApprox(action_space->explore.Y), true);
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