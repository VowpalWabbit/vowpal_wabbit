// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/common/random.h"
#include "vw/core/reductions/cb/cb_explore_adf_common.h"
#include "vw/core/reductions/cb/cb_explore_adf_graph_feedback.h"
#include "vw/core/vw.h"
#include "vw/test_common/test_common.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>

TEST(GraphFeedback, CheckIdentityG)
{
  std::vector<std::string> args{"--cb_explore_adf", "--graph_feedback", "--quiet"};
  auto vw_graph = VW::initialize(VW::make_unique<VW::config::options_cli>(args));

  //   // cb_adf is used to feed the graph feedback algorithm, so we will get its output
  //   std::vector<std::string> args_cbadf{"--cb_adf", "--quiet"};
  //   auto vw_cb_adf = VW::initialize(VW::make_unique<VW::config::options_cli>(args_cbadf));

  auto& vw = *vw_graph.get();

  {
    VW::multi_ex examples;

    examples.push_back(VW::read_example(vw, "shared |U b c"));
    examples.push_back(VW::read_example(vw, "|A 1:0.1 2:0.12 3:0.13 b200:2 c500:9"));
    // duplicates start
    examples.push_back(VW::read_example(vw, "|A a_1:0.5 a_2:0.65 a_3:0.12 a100:4 a200:33"));
    examples.push_back(VW::read_example(vw, "|A a_1:0.5 a_2:0.65 a_3:0.12 a100:4 a200:33"));
    // duplicates end
    examples.push_back(VW::read_example(vw, "|A a_4:0.8 a_5:0.32 a_6:0.15 d1:0.2 d10:0.2"));
    examples.push_back(VW::read_example(vw, "|A a_7 a_8 a_9 v1:0.99"));
    examples.push_back(VW::read_example(vw, "|A a_10 a_11 a_12"));
    examples.push_back(VW::read_example(vw, "|A a_13 a_14 a_15"));
    examples.push_back(VW::read_example(vw, "|A a_16 a_17 a_18:0.2"));

    vw.predict(examples);

    vw.finish_example(examples);
  }
}