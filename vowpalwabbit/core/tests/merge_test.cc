// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/merge.h"

#include "vw/config/options_cli.h"
#include "vw/core/reductions/cb/cb_adf.h"
#include "vw/core/shared_data.h"
#include "vw/core/vw.h"

#include <gtest/gtest.h>

TEST(merge_tests, merge_simple_model)
{
  auto options_strings = std::vector<std::string>{"--quiet"};
  auto vw1 = VW::initialize_experimental(VW::make_unique<VW::config::options_cli>(options_strings));
  auto vw2 = VW::initialize_experimental(VW::make_unique<VW::config::options_cli>(options_strings));

  auto* ex1 = VW::read_example(*vw1, "1 | a b");
  VW::setup_example(*vw1, ex1);
  vw1->learn(*ex1);
  vw1->finish_example(*ex1);

  auto* ex2 = VW::read_example(*vw2, "1 | c d");
  VW::setup_example(*vw2, ex2);
  vw2->learn(*ex2);
  vw2->finish_example(*ex2);

  std::vector<const VW::workspace*> workspaces = {vw1.get(), vw2.get()};
  auto result = VW::merge_models(nullptr, workspaces);

  EXPECT_FLOAT_EQ(vw1->sd->weighted_labeled_examples, 1.f);
  EXPECT_FLOAT_EQ(vw2->sd->weighted_labeled_examples, 1.f);
  EXPECT_FLOAT_EQ(result->sd->weighted_labeled_examples, 2.f);
}

TEST(merge_tests, merge_cb_model)
{
  auto options_strings = std::vector<std::string>{"--quiet", "--cb_explore_adf"};
  auto vw1 = VW::initialize_experimental(VW::make_unique<VW::config::options_cli>(options_strings));
  auto vw2 = VW::initialize_experimental(VW::make_unique<VW::config::options_cli>(options_strings));

  VW::multi_ex examples1;
  examples1.push_back(VW::read_example(*vw1, "shared |User user=Tom time_of_day=morning"));
  examples1.push_back(VW::read_example(*vw1, "|Action article=politics"));
  examples1.push_back(VW::read_example(*vw1, "|Action article=sports"));
  examples1.push_back(VW::read_example(*vw1, "|Action article=music"));
  examples1.push_back(VW::read_example(*vw1, "|Action article=food"));
  examples1.push_back(VW::read_example(*vw1, "0:0.0:0.14285714498588012 |Action article=finance"));
  examples1.push_back(VW::read_example(*vw1, "|Action article=health"));
  examples1.push_back(VW::read_example(*vw1, "|Action article=camping"));
  VW::setup_examples(*vw1, examples1);
  vw1->learn(examples1);
  vw1->finish_example(examples1);

  VW::multi_ex examples2;
  examples2.push_back(VW::read_example(*vw2, "shared |User user=Anna time_of_day=morning"));
  examples2.push_back(VW::read_example(*vw2, "0:0.0:0.14285713008471937 |Action article=politics"));
  examples2.push_back(VW::read_example(*vw2, "|Action article=sports"));
  examples2.push_back(VW::read_example(*vw2, "|Action article=music"));
  examples2.push_back(VW::read_example(*vw2, "|Action article=food"));
  examples2.push_back(VW::read_example(*vw2, "|Action article=finance"));
  examples2.push_back(VW::read_example(*vw2, "|Action article=health"));
  examples2.push_back(VW::read_example(*vw2, "|Action article=camping"));

  VW::setup_examples(*vw2, examples2);
  vw2->learn(examples2);
  vw2->finish_example(examples2);

  std::vector<const VW::workspace*> workspaces = {vw1.get(), vw2.get()};
  auto result = VW::merge_models(nullptr, workspaces);

  EXPECT_FLOAT_EQ(vw1->sd->weighted_labeled_examples, 1.f);
  EXPECT_FLOAT_EQ(vw2->sd->weighted_labeled_examples, 1.f);
  EXPECT_FLOAT_EQ(result->sd->weighted_labeled_examples, 2.f);

  auto* vw1_cb_adf = reinterpret_cast<CB_ADF::cb_adf*>(
      vw1->l->get_learner_by_name_prefix("cb_adf")->get_internal_type_erased_data_pointer_test_use_only());
  auto* vw2_cb_adf = reinterpret_cast<CB_ADF::cb_adf*>(
      vw2->l->get_learner_by_name_prefix("cb_adf")->get_internal_type_erased_data_pointer_test_use_only());
  auto* vw_merged_cb_adf = reinterpret_cast<CB_ADF::cb_adf*>(
      result->l->get_learner_by_name_prefix("cb_adf")->get_internal_type_erased_data_pointer_test_use_only());

  EXPECT_EQ(vw_merged_cb_adf->get_gen_cs().event_sum,
      vw1_cb_adf->get_gen_cs().event_sum + vw2_cb_adf->get_gen_cs().event_sum);
  EXPECT_EQ(vw_merged_cb_adf->get_gen_cs().action_sum,
      vw1_cb_adf->get_gen_cs().action_sum + vw2_cb_adf->get_gen_cs().action_sum);
}
