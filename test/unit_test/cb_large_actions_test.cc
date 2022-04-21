// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "reductions/cb/details/large_action_space.h"
#include "test_common.h"
#include "vw/core/rand48.h"
#include "vw/core/rand_state.h"
#include "vw/core/reductions/cb/cb_explore_adf_large_action_space.h"
#include "vw/core/vw.h"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(creation_of_Q_with_lazy_gaussian)
{
  auto& vw = *VW::initialize(
      "--cb_explore_adf --large_action_space --max_actions 2 --quiet", nullptr, false, nullptr, nullptr);

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

  auto action_space = (VW::cb_explore_adf::cb_explore_adf_base<VW::cb_explore_adf::cb_explore_adf_large_action_space>*)
                          learner->get_internal_type_erased_data_pointer_test_use_only();

  BOOST_CHECK_EQUAL(action_space != nullptr, true);

  {
    VW::multi_ex examples;

    examples.push_back(VW::read_example(vw, "0:1.0:0.5 | 1 2 3"));

    vw.predict(examples);

    action_space->explore.generate_Q(examples);
    std::cout << action_space->explore.Q << std::endl;

    // feature 1 -> stride shifted
    // feature 2 -> stride shifted
    // feature 3 -> stride shifted
    auto f1_i = 1 << 2;
    auto f2_i = 2 << 2;
    auto f3_i = 3 << 2;
    auto constant_i = examples[0]->feature_space[constant_namespace].indices[0];

    auto fw1 = vw.weights.dense_weights[f1_i];
    auto fw2 = vw.weights.dense_weights[f2_i];
    auto fw3 = vw.weights.dense_weights[f3_i];

    auto fw_constant = vw.weights.dense_weights[constant_i];

    uint64_t seed = vw.get_random_state()->get_current_state();
    // checking the multiplication of matrix A of vw action weights dim(#actions x #features) which is a sparse
    // matrix, with the random matrix with Gaussian entries of dim(#features x d)

    {
      uint64_t gauss_col = 0;

      uint64_t A_row = f1_i;
      auto combined_index = A_row + gauss_col + seed;
      auto lazyg_col_0_row_f1 = merand48_boxmuller(combined_index);

      A_row = f2_i;
      combined_index = A_row + gauss_col + seed;
      auto lazyg_col_0_row_f2 = merand48_boxmuller(combined_index);

      A_row = f3_i;
      combined_index = A_row + gauss_col + seed;
      auto lazyg_col_0_row_f3 = merand48_boxmuller(combined_index);

      A_row = constant_i;
      combined_index = A_row + gauss_col + seed;
      auto lazyg_col_0_row_const = merand48_boxmuller(combined_index);

      float dot_product = fw1 * lazyg_col_0_row_f1 + fw2 * lazyg_col_0_row_f2 + fw3 * lazyg_col_0_row_f3 +
          fw_constant * lazyg_col_0_row_const;
      BOOST_CHECK_EQUAL(dot_product, action_space->explore.Q(0, gauss_col));
    }
    {
      uint64_t gauss_col = 1;

      uint64_t A_row = f1_i;
      auto combined_index = A_row + gauss_col + seed;
      auto lazyg_col_1_row_f1 = merand48_boxmuller(combined_index);

      A_row = f2_i;
      combined_index = A_row + gauss_col + seed;
      auto lazyg_col_1_row_f2 = merand48_boxmuller(combined_index);

      A_row = f3_i;
      combined_index = A_row + gauss_col + seed;
      auto lazyg_col_1_row_f3 = merand48_boxmuller(combined_index);

      A_row = constant_i;
      combined_index = A_row + gauss_col + seed;
      auto lazyg_col_1_row_const = merand48_boxmuller(combined_index);

      float dot_product = fw1 * lazyg_col_1_row_f1 + fw2 * lazyg_col_1_row_f2 + fw3 * lazyg_col_1_row_f3 +
          fw_constant * lazyg_col_1_row_const;
      BOOST_CHECK_EQUAL(dot_product, action_space->explore.Q(0, gauss_col));
    }
  }
}
