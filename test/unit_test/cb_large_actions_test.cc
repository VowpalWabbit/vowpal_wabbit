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

using internal_action_space =
    VW::cb_explore_adf::cb_explore_adf_base<VW::cb_explore_adf::cb_explore_adf_large_action_space>;

BOOST_AUTO_TEST_CASE(creation_of_the_og_A_matrix)
{
  auto d = 2;
  auto& vw = *VW::initialize(
      "--cb_explore_adf --large_action_space --max_actions " + std::to_string(d) + " --quiet --random_seed 5", nullptr,
      false, nullptr, nullptr);

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

    action_space->explore._generate_A(examples);

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
              action_space->explore._A.coeffRef(action_index, (ft_index & vw.weights.dense_weights.mask())),
              vw.weights.dense_weights[ft_index]);
        }
      }
    }
  }
}

BOOST_AUTO_TEST_CASE(check_At_times_Omega_is_Y)
{
  auto d = 2;
  auto& vw = *VW::initialize(
      "--cb_explore_adf --large_action_space --max_actions " + std::to_string(d) + " --quiet --random_seed 5", nullptr,
      false, nullptr, nullptr);

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

    action_space->explore._generate_A(examples);
    action_space->explore.generate_Y(examples);

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

    Eigen::SparseMatrix<float> Yd(action_space->explore.Y.rows(), d);
    Yd = action_space->explore._A.transpose() * Omega;
    // Orthonormalize Yd
    VW::gram_schmidt(Yd);
    BOOST_CHECK_EQUAL(Yd.isApprox(action_space->explore.Y), true);
  }
}

BOOST_AUTO_TEST_CASE(check_A_times_Y_is_B)
{
  auto d = 2;
  auto& vw = *VW::initialize(
      "--cb_explore_adf --large_action_space --max_actions " + std::to_string(d) + " --quiet --random_seed 5", nullptr,
      false, nullptr, nullptr);

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

    action_space->explore._generate_A(examples);
    action_space->explore.generate_Y(examples);
    action_space->explore.generate_B(examples);

    Eigen::MatrixXf B = action_space->explore._A * action_space->explore.Y;
    BOOST_CHECK_EQUAL(B.isApprox(action_space->explore.B), true);
  }
}

BOOST_AUTO_TEST_CASE(check_B_times_P_is_Z)
{
  auto d = 2;
  auto& vw = *VW::initialize(
      "--cb_explore_adf --large_action_space --max_actions " + std::to_string(d) + " --quiet --random_seed 5", nullptr,
      false, nullptr, nullptr);

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

    action_space->explore._generate_A(examples);
    action_space->explore.generate_Y(examples);
    action_space->explore.generate_B(examples);
    action_space->explore.generate_Z(examples);

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

    Eigen::MatrixXf Zp = action_space->explore.B * P;
    VW::gram_schmidt(Zp);
    BOOST_CHECK_EQUAL(Zp.isApprox(action_space->explore.Z), true);
  }
}

BOOST_AUTO_TEST_CASE(check_final_U_dimensions)
{
  auto d = 2;
  auto& vw = *VW::initialize(
      "--cb_explore_adf --large_action_space --max_actions " + std::to_string(d) + " --quiet --random_seed 5", nullptr,
      false, nullptr, nullptr);

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

void generate_random_vector(
    Eigen::SparseVector<float>& vec, VW::workspace& vw, uint64_t min, uint64_t max, uint64_t& min_index)
{
  for (size_t i = 0; i < vec.rows(); i++)
  {
    uint64_t seed = vw.get_random_state()->get_and_update_random() * 1000.f;
    auto combined_index = i + seed;

    uint64_t index = merand48_boxmuller(combined_index) * 1000.f;
    while (index < min || index > max) { index = merand48_boxmuller(combined_index) * 1000.f; }

    auto stride = vw.weights.sparse ? vw.weights.sparse_weights.stride() : vw.weights.dense_weights.stride();
    auto mask = vw.weights.sparse ? vw.weights.sparse_weights.mask() : vw.weights.dense_weights.mask();
    auto strided_index = (index << stride) & mask;
    combined_index = i + strided_index + seed;
    auto gaussian_value = merand48_boxmuller(combined_index);
    vec.coeffRef(strided_index) = gaussian_value;

    if (strided_index < min_index) { min_index = strided_index; }
  }
}

std::vector<uint64_t> generate_A_square(
    Eigen::SparseMatrix<float>& A_square, VW::workspace& vw, uint64_t num_actions, uint64_t max_col)
{
  uint64_t min = 0;
  uint64_t max = 20;

  std::vector<uint64_t> min_indexes;
  for (size_t i = 0; i < num_actions; i++)
  {
    Eigen::SparseVector<float> v_vec(max_col);
    uint64_t min_index = max_col + 1;

    generate_random_vector(v_vec, vw, min, max, min_index);

    min_indexes.push_back(min_index);
    Eigen::SparseMatrix<float> temp = v_vec * v_vec.transpose();
    A_square += temp;
    min = max + 20;
    max = min + 20;
  }
  return min_indexes;
}

std::unordered_map<uint64_t, uint64_t> map_A_square_rows_to_A_rows(
    const Eigen::SparseMatrix<float>& A_square, const std::vector<uint64_t>& index_cutoffs, uint64_t num_actions)
{
  std::unordered_map<uint64_t, uint64_t> rows_map;

  uint64_t latest_index = 0;

  for (int k = 0; k < A_square.outerSize(); ++k)
  {
    size_t i = 0;
    for (Eigen::SparseMatrix<float>::InnerIterator it(A_square, k); it; ++it)
    {
      if (it.row() >= index_cutoffs[i])
      {
        i++;
        if (rows_map.find(it.row()) == rows_map.end())
        {
          rows_map.insert({it.row(), latest_index});
          latest_index++;
          if (latest_index >= num_actions) { return rows_map; }
        }
      }
    }
  }
  return rows_map;
}

multi_ex setup_vw_weights_and_examples_and_create_A(Eigen::SparseMatrix<float>& A,
    const Eigen::SparseMatrix<float>& A_square, VW::workspace& vw, std::unordered_map<uint64_t, uint64_t>& rows_map,
    const std::vector<uint64_t>& index_cutoffs, uint64_t num_actions)
{
  multi_ex examples;

  std::vector<std::string> ssvector;
  for (size_t row = 0; row < num_actions; row++)
  {
    std::string ss("| ");
    ssvector.push_back(ss);
  }

  for (int k = 0; k < A_square.outerSize(); ++k)
  {
    size_t i = 0;
    for (Eigen::SparseMatrix<float>::InnerIterator it(A_square, k); it; ++it)
    {
      if (it.row() >= index_cutoffs[i] && rows_map.find(it.row()) != rows_map.end())
      {
        auto index_mapped = rows_map[it.row()];
        A.coeffRef(index_mapped, k) = it.value();

        auto vw_index = it.col() >> vw.weights.stride_shift();
        ssvector[index_mapped] += std::to_string(vw_index) + " ";

        if (vw.weights.sparse) { vw.weights.sparse_weights[it.col()] = it.value(); }
        else
        {
          vw.weights.dense_weights[it.col()] = it.value();
        }
      }
    }
  }

  for (size_t row = 0; row < num_actions; row++)
  {
    examples.push_back(VW::read_example(vw, ssvector[row]));
    examples[0]->pred.a_s.push_back({0, 0});
  }

  return examples;
}

BOOST_AUTO_TEST_CASE(check_final_truncated_SVD_validity)
{
  /*
  To test the correctness of randomized truncated svd we need to create an appropriate low rank matrix A to test.
  A is created by the sum of the outer products of n vectors with themselves (A = sum{i = 1, n} (v * v.transpose()))
  n is the upper bound to A's potential rank.

  A is going to be translated into vw examples so we want to avoid an A matrix where all rows have the same columns
  populated. Column indexes will be translated into vw indexes and we want them to overlap as little as possible, since
  in a generated A matrix it is fine for the same column to have a different value for different rows, but in vw, since
  it represents a feature, we want same columns to have the same value on different rows. To avoid this we try to crate
  the vectors that will comprise A as sparse vectors, each creating indexes in different feature index regions.

   A is going to be a square matrix but we don't expect to have square matrixes in real-world usage so we create a
  rectangular matrix from A_square.

   We then proceed to translate that rectangular matrix to vw actions (examples) by
  taking each column index and applying the opposite stride shift that vw will apply after parsing the features
  (features are integers here so that vw doesn't apply extra hashing)

  After applying the randomized truncated SVD step we check that U and V matrixes are orthonormal, that recreating A
  from the decomposed matrixes is close enough to the original A matrix, and that the singular values of the truncated
  and randomized decomposition match the singular values of a traditional SVD applied to the same matrix.
   */

  auto& vw =
      *VW::initialize("--cb_explore_adf --noconstant --large_action_space --max_actions 0 --quiet --random_seed 5",
          nullptr, false, nullptr, nullptr);

  uint64_t num_actions = 5;  // rows
  uint64_t max_col = vw.weights.sparse ? vw.weights.sparse_weights.mask() : vw.weights.dense_weights.mask();

  Eigen::SparseMatrix<float> A_square(max_col, max_col);

  auto index_cutoffs = generate_A_square(A_square, vw, num_actions, max_col);

  // create rectangulare A so that we can figure out its rank and set d to that
  // we need the matrix rank to be able to test the reconstruction correctly
  Eigen::SparseMatrix<float> A(num_actions, max_col);
  // it is cheaper to traverse the sparse matrixes by their internal representation (non-zero columns first then their
  // rows)
  // we want to take #num_actions non-zero rows from A_square and place them into the 0..num_actions rows of matrix A
  // so we proceed to create a map between those non-zero rows of A_square and the row that they correspond to in the A
  // matrix
  auto rows_map = map_A_square_rows_to_A_rows(A_square, index_cutoffs, num_actions);
  // translate to internal vw representation (setting vw's weights) and create the examples so that we can call SVD
  multi_ex examples = setup_vw_weights_and_examples_and_create_A(A, A_square, vw, rows_map, index_cutoffs, num_actions);

  std::vector<std::string> e_r;
  vw.l->get_enabled_reductions(e_r);
  if (std::find(e_r.begin(), e_r.end(), "cb_explore_adf_large_action_space") == e_r.end())
  { BOOST_FAIL("cb_explore_adf_large_action_space not found in enabled reductions"); }

  VW::LEARNER::multi_learner* learner =
      as_multiline(vw.l->get_learner_by_name_prefix("cb_explore_adf_large_action_space"));

  auto action_space = (internal_action_space*)learner->get_internal_type_erased_data_pointer_test_use_only();
  BOOST_CHECK_EQUAL(action_space != nullptr, true);

  {
    Eigen::FullPivLU<Eigen::MatrixXf> lu_decomp(A);
    auto rank = lu_decomp.rank();
    // for test set actual rank of A
    action_space->explore.set_rank(rank);
    // should have a rank larger than 1 for the test
    BOOST_CHECK_GT(rank, 1);
  }

  action_space->explore._populate_all_SVD_components();

  action_space->explore._generate_A(examples);
  action_space->explore.randomized_SVD(examples);

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
      (action_space->explore._A -
          action_space->explore.U * action_space->explore._S.asDiagonal() * action_space->explore._V.transpose())
          .norm(),
      FLOAT_TOL);

  // compare singular values with actual SVD singular values
  Eigen::MatrixXf A_dense = action_space->explore._A;
  Eigen::JacobiSVD<Eigen::MatrixXf> svd(A_dense, Eigen::ComputeThinU | Eigen::ComputeThinV);
  Eigen::VectorXf S = svd.singularValues();

  for (size_t i = 0; i < action_space->explore._S.rows(); i++)
  { BOOST_CHECK_CLOSE(S(i), action_space->explore._S(i), FLOAT_TOL); }
}

BOOST_AUTO_TEST_CASE(check_finding_max_volume)
{
  auto d = 3;
  auto& vw = *VW::initialize(
      "--cb_explore_adf --large_action_space --max_actions " + std::to_string(d) + " --quiet --random_seed 5", nullptr,
      false, nullptr, nullptr);
  VW::cb_explore_adf::cb_explore_adf_large_action_space largecb(0, 1, &vw);
  largecb.U = Eigen::MatrixXf{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}, {0, 0, 0}, {7, 5, 3}, {6, 4, 8}};
  Eigen::MatrixXf X{{1, 2, 3}, {3, 2, 1}, {2, 1, 3}};

  auto p = largecb.find_max_volume(0, X);
  BOOST_CHECK_CLOSE(p.first, 30, FLOAT_TOL);
  BOOST_CHECK_EQUAL(p.second, 2);

  p = largecb.find_max_volume(1, X);
  BOOST_CHECK_CLOSE(p.first, 27, FLOAT_TOL);
  BOOST_CHECK_EQUAL(p.second, 4);

  p = largecb.find_max_volume(2, X);
  BOOST_CHECK_CLOSE(p.first, 24, FLOAT_TOL);
  BOOST_CHECK_EQUAL(p.second, 5);
}

BOOST_AUTO_TEST_CASE(check_spanner_results)
{
  auto d = 2;
  auto& vw = *VW::initialize(
      "--cb_explore_adf --large_action_space --max_actions " + std::to_string(d) + " --quiet --random_seed 5", nullptr,
      false, nullptr, nullptr);

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
  {
    BOOST_FAIL("cb_explore_adf_large_action_space not found in enabled reductions");
  }

  VW::LEARNER::multi_learner* learner =
      as_multiline(vw.l->get_learner_by_name_prefix("cb_explore_adf_large_action_space"));

  auto action_space = (internal_action_space*)learner->get_internal_type_erased_data_pointer_test_use_only();
  BOOST_CHECK_EQUAL(action_space != nullptr, true);

  {
    VW::multi_ex examples;

    examples.push_back(VW::read_example(vw, "| 1 2 3"));
    examples.push_back(VW::read_example(vw, "| a_1 a_2 a_3"));
    examples.push_back(VW::read_example(vw, "| a_4 a_5 a_6"));

    learner->predict(examples);

    const auto num_actions = examples.size();
    const auto& preds = examples[0]->pred.a_s;
    BOOST_CHECK_EQUAL(preds.size(), num_actions);
    // Only d actions have non-zero scores.
    BOOST_CHECK_CLOSE(preds[0].score, 2.0 / 3, FLOAT_TOL);
    BOOST_CHECK_CLOSE(preds[1].score, 0.0, FLOAT_TOL);
    BOOST_CHECK_CLOSE(preds[2].score, 1.0 / 3, FLOAT_TOL);
  }
}

BOOST_AUTO_TEST_CASE(check_uniform_probabilities_before_learning)
{
  auto d = 2;
  auto& vw = *VW::initialize(
      "--cb_explore_adf --large_action_space --max_actions " + std::to_string(d) + " --quiet --random_seed 5", nullptr,
      false, nullptr, nullptr);

  VW::LEARNER::multi_learner* learner =
      as_multiline(vw.l->get_learner_by_name_prefix("cb_explore_adf_large_action_space"));

  {
    VW::multi_ex examples;

    examples.push_back(VW::read_example(vw, "| 1 2 3"));
    examples.push_back(VW::read_example(vw, "| a_1 a_2 a_3"));
    examples.push_back(VW::read_example(vw, "| a_4 a_5 a_6"));

    learner->predict(examples);

    const auto num_actions = examples.size();
    const auto& preds = examples[0]->pred.a_s;
    BOOST_CHECK_EQUAL(preds.size(), num_actions);
    for (const auto& pred : preds) { BOOST_CHECK_CLOSE(pred.score, 1.0 / 3, FLOAT_TOL); }
  }
}
