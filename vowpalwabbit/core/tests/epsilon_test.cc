// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/epsilon_reduction_features.h"
#include "vw/core/reduction_features.h"
#include "vw/core/vw.h"
#include "vw/test_common/test_common.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(Epsilon, SetEpsilonTest)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--cb_explore_adf"));
  VW::multi_ex examples;
  examples.push_back(VW::read_example(*vw, std::string("")));
  auto& ep_fts = examples[0]->ex_reduction_features.template get<VW::cb_explore_adf::greedy::reduction_features>();
  EXPECT_FLOAT_EQ(ep_fts.epsilon, -1.f);
  ep_fts.epsilon = 0.5f;
  auto& ep_fts2 = examples[0]->ex_reduction_features.template get<VW::cb_explore_adf::greedy::reduction_features>();
  EXPECT_FLOAT_EQ(ep_fts2.epsilon, 0.5f);
  ep_fts2.reset_to_default();
  EXPECT_FLOAT_EQ(ep_fts2.epsilon, -1.f);
  vw->finish_example(examples);
}
