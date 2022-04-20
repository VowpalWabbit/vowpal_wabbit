// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "test_common.h"
#include "vw/core/reductions/cb/cb_explore_adf_large_action_space.h"
#include "reductions/cb/details/large_action_space.h"
#include "vw/core/vw.h"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(creation_of_Q_with_lazy_gaussian)
{
  auto& vw = *VW::initialize(
      "--cb_explore_adf --large_action_space --max_actions 2 --quiet", nullptr, false, nullptr, nullptr);

  {
    VW::multi_ex examples;

    examples.push_back(VW::read_example(vw, "shared | s_1 s_2"));
    examples.push_back(VW::read_example(vw, "0:1.0:0.5 | a_1 b_1 c_1"));
    examples.push_back(VW::read_example(vw, "| a_2 b_2 c_2"));
    examples.push_back(VW::read_example(vw, "| a_3 b_3 c_3"));

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

    examples.push_back(VW::read_example(vw, "shared | s_1 s_2"));
    examples.push_back(VW::read_example(vw, "0:1.0:0.5 | a_1 b_1 c_1"));
    examples.push_back(VW::read_example(vw, "| a_2 b_2 c_2"));
    examples.push_back(VW::read_example(vw, "| a_3 b_3 c_3"));

    vw.predict(examples);

    action_space->explore.generate_Q(examples);
    std::cout << action_space->explore.Q << std::endl;
  }
}
