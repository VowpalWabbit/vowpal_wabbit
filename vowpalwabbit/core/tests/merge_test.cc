// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/merge.h"

#include "vw/config/options_cli.h"
#include "vw/core/reductions/cb/cb_adf.h"
#include "vw/core/shared_data.h"
#include "vw/core/vw.h"
#include "vw/io/io_adapter.h"
#include "vw/test_common/test_common.h"

#include <gtest/gtest.h>

TEST(Merge, AddSubtractModelDelta)
{
  auto vw_base = VW::initialize(vwtest::make_args("--quiet"));
  auto vw_new = VW::initialize(vwtest::make_args("--quiet"));

  // Create a base workspace and another workspace trained on additional example
  {
    auto* ex = VW::read_example(*vw_base, "1 | a b");
    VW::setup_example(*vw_base, ex);
    vw_base->learn(*ex);
    vw_base->finish_example(*ex);
  }
  {
    auto* ex = VW::read_example(*vw_new, "1 | a b");
    VW::setup_example(*vw_new, ex);
    vw_new->learn(*ex);
    vw_new->finish_example(*ex);
  }
  {
    auto* ex = VW::read_example(*vw_new, "1 | c");
    VW::setup_example(*vw_new, ex);
    vw_new->learn(*ex);
    vw_new->finish_example(*ex);
  }

  // Test that (base + (new - base)) == new
  auto delta = *vw_new - *vw_base;
  auto base_plus_delta = *vw_base + delta;
  const auto sd1 = vw_new->sd;
  const auto sd2 = base_plus_delta->sd;
  EXPECT_FLOAT_EQ(sd1->weighted_labeled_examples, sd2->weighted_labeled_examples);
  EXPECT_FLOAT_EQ(sd1->weighted_unlabeled_examples, sd2->weighted_unlabeled_examples);
  EXPECT_FLOAT_EQ(sd1->weighted_labels, sd2->weighted_labels);
  EXPECT_FLOAT_EQ(sd1->sum_loss, sd2->sum_loss);
  EXPECT_FLOAT_EQ(sd1->example_number, sd2->example_number);
  EXPECT_FLOAT_EQ(sd1->total_features, sd2->total_features);
}

TEST(Merge, MergeSimpleModel)
{
  auto vw1 = VW::initialize(vwtest::make_args("--quiet", "--sgd"));
  auto vw2 = VW::initialize(vwtest::make_args("--quiet", "--sgd"));

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

  // check that shared data got merged
  EXPECT_FLOAT_EQ(vw1->sd->weighted_labeled_examples, 1.f);
  EXPECT_FLOAT_EQ(vw2->sd->weighted_labeled_examples, 1.f);
  EXPECT_FLOAT_EQ(result->sd->weighted_labeled_examples, 2.f);

  // check that weight values got merged
  EXPECT_FALSE(result->weights.sparse);
  EXPECT_EQ(result->num_bits, vw1->num_bits);
  EXPECT_EQ(result->num_bits, vw2->num_bits);
  const size_t length = static_cast<size_t>(1) << result->num_bits;
  const auto& vw1_weights = vw1->weights.dense_weights;
  const auto& vw2_weights = vw2->weights.dense_weights;
  const auto& result_weights = result->weights.dense_weights;
  for (size_t i = 0; i < length; i++)
  {
    EXPECT_FLOAT_EQ(
        result_weights.strided_index(i), 0.5 * (vw1_weights.strided_index(i) + vw2_weights.strided_index(i)));
  }
}

TEST(Merge, MergeSimpleModelDelta)
{
  auto vw_base = VW::initialize(vwtest::make_args("--quiet"));
  auto vw1 = VW::initialize(vwtest::make_args("--quiet"));
  auto vw2 = VW::initialize(vwtest::make_args("--quiet"));

  // instead of copying base model, we train all models on the same base example
  {
    auto* ex = VW::read_example(*vw_base, "1 | x y z");
    VW::setup_example(*vw_base, ex);
    vw_base->learn(*ex);
    vw_base->finish_example(*ex);
  }
  {
    auto* ex = VW::read_example(*vw1, "1 | x y z");
    VW::setup_example(*vw1, ex);
    vw1->learn(*ex);
    vw1->finish_example(*ex);
  }
  {
    auto* ex = VW::read_example(*vw2, "1 | x y z");
    VW::setup_example(*vw2, ex);
    vw2->learn(*ex);
    vw2->finish_example(*ex);
  }

  // train models 1 and 2 on different examples
  auto* ex1 = VW::read_example(*vw1, "1 | a b");
  VW::setup_example(*vw1, ex1);
  vw1->learn(*ex1);
  vw1->finish_example(*ex1);

  auto* ex2 = VW::read_example(*vw2, "1 | c d");
  VW::setup_example(*vw2, ex2);
  vw2->learn(*ex2);
  vw2->finish_example(*ex2);

  EXPECT_FLOAT_EQ(vw_base->sd->weighted_labeled_examples, 1.f);
  EXPECT_FLOAT_EQ(vw1->sd->weighted_labeled_examples, 2.f);
  EXPECT_FLOAT_EQ(vw2->sd->weighted_labeled_examples, 2.f);

  // Take model deltas and merge them
  auto delta1 = *vw1 - *vw_base;
  auto delta2 = *vw2 - *vw_base;
  auto deltas_merged = VW::merge_deltas(std::vector<const VW::model_delta*>{&delta1, &delta2});
  auto result_delta_merge = *vw_base + deltas_merged;

  EXPECT_FLOAT_EQ(delta1.unsafe_get_workspace_ptr()->sd->weighted_labeled_examples, 1.f);
  EXPECT_FLOAT_EQ(delta2.unsafe_get_workspace_ptr()->sd->weighted_labeled_examples, 1.f);
  EXPECT_FLOAT_EQ(deltas_merged.unsafe_get_workspace_ptr()->sd->weighted_labeled_examples, 2.f);
  EXPECT_FLOAT_EQ(result_delta_merge->sd->weighted_labeled_examples, 3.f);

  // Merge workspaces directly, this should produce same results
  std::vector<const VW::workspace*> workspaces{vw1.get(), vw2.get()};
  auto result_model_merge = VW::merge_models(vw_base.get(), workspaces);

  EXPECT_FLOAT_EQ(result_model_merge->sd->weighted_labeled_examples, 3.f);
}

TEST(Merge, MergeCbModel)
{
  auto vw1 = VW::initialize(vwtest::make_args("--quiet", "--cb_explore_adf"));
  auto vw2 = VW::initialize(vwtest::make_args("--quiet", "--cb_explore_adf"));

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

  std::vector<const VW::workspace*> workspaces{vw1.get(), vw2.get()};
  auto result = VW::merge_models(nullptr, workspaces);

  EXPECT_FLOAT_EQ(vw1->sd->weighted_labeled_examples, 1.f);
  EXPECT_FLOAT_EQ(vw2->sd->weighted_labeled_examples, 1.f);
  EXPECT_FLOAT_EQ(result->sd->weighted_labeled_examples, 2.f);

  auto* vw1_cb_adf = reinterpret_cast<VW::reductions::cb_adf*>(
      vw1->l->get_learner_by_name_prefix("cb_adf")->get_internal_type_erased_data_pointer_test_use_only());
  auto* vw2_cb_adf = reinterpret_cast<VW::reductions::cb_adf*>(
      vw2->l->get_learner_by_name_prefix("cb_adf")->get_internal_type_erased_data_pointer_test_use_only());
  auto* vw_merged_cb_adf = reinterpret_cast<VW::reductions::cb_adf*>(
      result->l->get_learner_by_name_prefix("cb_adf")->get_internal_type_erased_data_pointer_test_use_only());

  EXPECT_EQ(vw_merged_cb_adf->get_gen_cs().event_sum,
      vw1_cb_adf->get_gen_cs().event_sum + vw2_cb_adf->get_gen_cs().event_sum);
  EXPECT_EQ(vw_merged_cb_adf->get_gen_cs().action_sum,
      vw1_cb_adf->get_gen_cs().action_sum + vw2_cb_adf->get_gen_cs().action_sum);
}

TEST(Merge, MergeCbModelDelta)
{
  auto options_strings = std::vector<std::string>{"--quiet", "--cb_explore_adf"};
  auto vw_base = VW::initialize(vwtest::make_args("--quiet", "--cb_explore_adf"));
  auto vw1 = VW::initialize(vwtest::make_args("--quiet", "--cb_explore_adf"));
  auto vw2 = VW::initialize(vwtest::make_args("--quiet", "--cb_explore_adf"));

  {
    VW::multi_ex examples;
    examples.push_back(VW::read_example(*vw_base, "shared |User user=Tom time_of_day=morning"));
    examples.push_back(VW::read_example(*vw_base, "|Action article=politics"));
    examples.push_back(VW::read_example(*vw_base, "|Action article=sports"));
    examples.push_back(VW::read_example(*vw_base, "|Action article=music"));
    examples.push_back(VW::read_example(*vw_base, "|Action article=food"));
    examples.push_back(VW::read_example(*vw_base, "0:0.0:0.14285714498588012 |Action article=finance"));
    examples.push_back(VW::read_example(*vw_base, "|Action article=health"));
    examples.push_back(VW::read_example(*vw_base, "|Action article=camping"));
    VW::setup_examples(*vw_base, examples);
    vw_base->learn(examples);
    vw_base->finish_example(examples);
  }
  {
    VW::multi_ex examples;
    examples.push_back(VW::read_example(*vw1, "shared |User user=Tom time_of_day=morning"));
    examples.push_back(VW::read_example(*vw1, "|Action article=politics"));
    examples.push_back(VW::read_example(*vw1, "|Action article=sports"));
    examples.push_back(VW::read_example(*vw1, "|Action article=music"));
    examples.push_back(VW::read_example(*vw1, "|Action article=food"));
    examples.push_back(VW::read_example(*vw1, "0:0.0:0.14285714498588012 |Action article=finance"));
    examples.push_back(VW::read_example(*vw1, "|Action article=health"));
    examples.push_back(VW::read_example(*vw1, "|Action article=camping"));
    VW::setup_examples(*vw1, examples);
    vw1->learn(examples);
    vw1->finish_example(examples);
  }
  {
    VW::multi_ex examples;
    examples.push_back(VW::read_example(*vw2, "shared |User user=Tom time_of_day=morning"));
    examples.push_back(VW::read_example(*vw2, "|Action article=politics"));
    examples.push_back(VW::read_example(*vw2, "|Action article=sports"));
    examples.push_back(VW::read_example(*vw2, "|Action article=music"));
    examples.push_back(VW::read_example(*vw2, "|Action article=food"));
    examples.push_back(VW::read_example(*vw2, "0:0.0:0.14285714498588012 |Action article=finance"));
    examples.push_back(VW::read_example(*vw2, "|Action article=health"));
    examples.push_back(VW::read_example(*vw2, "|Action article=camping"));
    VW::setup_examples(*vw2, examples);
    vw2->learn(examples);
    vw2->finish_example(examples);
  }

  VW::multi_ex examples1;
  examples1.push_back(VW::read_example(*vw1, "shared |User user=Tom time_of_day=afternoon"));
  examples1.push_back(VW::read_example(*vw1, "|Action article=politics"));
  examples1.push_back(VW::read_example(*vw1, "|Action article=sports"));
  examples1.push_back(VW::read_example(*vw1, "|Action article=music"));
  examples1.push_back(VW::read_example(*vw1, "0:0.0:0.14285714498588012 |Action article=food"));
  examples1.push_back(VW::read_example(*vw1, "|Action article=finance"));
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

  VW::multi_ex examples3;
  examples3.push_back(VW::read_example(*vw2, "shared |User user=Anna time_of_day=afternoon"));
  examples3.push_back(VW::read_example(*vw2, "|Action article=politics"));
  examples3.push_back(VW::read_example(*vw2, "0:0.0:0.14285713008471937 |Action article=sports"));
  examples3.push_back(VW::read_example(*vw2, "|Action article=music"));
  examples3.push_back(VW::read_example(*vw2, "|Action article=food"));
  examples3.push_back(VW::read_example(*vw2, "|Action article=finance"));
  examples3.push_back(VW::read_example(*vw2, "|Action article=health"));
  examples3.push_back(VW::read_example(*vw2, "|Action article=camping"));
  VW::setup_examples(*vw2, examples3);
  vw2->learn(examples3);
  vw2->finish_example(examples3);

  EXPECT_FLOAT_EQ(vw_base->sd->weighted_labeled_examples, 1.f);
  EXPECT_FLOAT_EQ(vw1->sd->weighted_labeled_examples, 2.f);
  EXPECT_FLOAT_EQ(vw2->sd->weighted_labeled_examples, 3.f);

  // compute model deltas and merge them
  auto delta1 = *vw1 - *vw_base;
  auto delta2 = *vw2 - *vw_base;
  auto merged = VW::merge_deltas(std::vector<const VW::model_delta*>{&delta1, &delta2});
  auto result_delta_merge = *vw_base + merged;

  auto* vw_base_cb_adf = reinterpret_cast<VW::reductions::cb_adf*>(
      vw_base->l->get_learner_by_name_prefix("cb_adf")->get_internal_type_erased_data_pointer_test_use_only());
  auto* vw1_cb_adf = reinterpret_cast<VW::reductions::cb_adf*>(
      vw1->l->get_learner_by_name_prefix("cb_adf")->get_internal_type_erased_data_pointer_test_use_only());
  auto* vw2_cb_adf = reinterpret_cast<VW::reductions::cb_adf*>(
      vw2->l->get_learner_by_name_prefix("cb_adf")->get_internal_type_erased_data_pointer_test_use_only());
  auto* delta_merged_cb_adf =
      reinterpret_cast<VW::reductions::cb_adf*>(result_delta_merge->l->get_learner_by_name_prefix("cb_adf")
                                                    ->get_internal_type_erased_data_pointer_test_use_only());
  EXPECT_FLOAT_EQ(result_delta_merge->sd->weighted_labeled_examples, 4.f);
  EXPECT_EQ(delta_merged_cb_adf->get_gen_cs().event_sum,
      vw1_cb_adf->get_gen_cs().event_sum + vw2_cb_adf->get_gen_cs().event_sum - vw_base_cb_adf->get_gen_cs().event_sum);
  EXPECT_EQ(delta_merged_cb_adf->get_gen_cs().action_sum,
      vw1_cb_adf->get_gen_cs().action_sum + vw2_cb_adf->get_gen_cs().action_sum -
          vw_base_cb_adf->get_gen_cs().action_sum);

  // merge workspaces directly without deltas
  std::vector<const VW::workspace*> workspaces{vw1.get(), vw2.get()};
  auto result_model_merge = VW::merge_models(vw_base.get(), workspaces);

  auto* model_merged_cb_adf =
      reinterpret_cast<VW::reductions::cb_adf*>(result_model_merge->l->get_learner_by_name_prefix("cb_adf")
                                                    ->get_internal_type_erased_data_pointer_test_use_only());
  EXPECT_FLOAT_EQ(result_delta_merge->sd->weighted_labeled_examples, result_model_merge->sd->weighted_labeled_examples);
  EXPECT_EQ(delta_merged_cb_adf->get_gen_cs().event_sum, model_merged_cb_adf->get_gen_cs().event_sum);
  EXPECT_EQ(delta_merged_cb_adf->get_gen_cs().action_sum, model_merged_cb_adf->get_gen_cs().action_sum);
}

TEST(Merge, SerializeDeserializeDelta)
{
  auto vw_base = VW::initialize(vwtest::make_args("--quiet"));
  auto vw_new = VW::initialize(vwtest::make_args("--quiet"));

  // Create a base workspace and another workspace trained on additional example
  {
    auto* ex = VW::read_example(*vw_base, "1 | a b");
    VW::setup_example(*vw_base, ex);
    vw_base->learn(*ex);
    vw_base->finish_example(*ex);
  }
  {
    auto* ex = VW::read_example(*vw_new, "1 | a b");
    VW::setup_example(*vw_new, ex);
    vw_new->learn(*ex);
    vw_new->finish_example(*ex);
  }
  {
    auto* ex = VW::read_example(*vw_new, "1 | c");
    VW::setup_example(*vw_new, ex);
    vw_new->learn(*ex);
    vw_new->finish_example(*ex);
  }

  // Test that (base + (new - base)) == new
  auto delta = *vw_new - *vw_base;
  auto base_plus_delta = *vw_base + delta;
  const auto sd1 = vw_new->sd;
  const auto sd2 = base_plus_delta->sd;
  EXPECT_FLOAT_EQ(sd1->weighted_labeled_examples, sd2->weighted_labeled_examples);
  EXPECT_FLOAT_EQ(sd1->weighted_unlabeled_examples, sd2->weighted_unlabeled_examples);
  EXPECT_FLOAT_EQ(sd1->weighted_labels, sd2->weighted_labels);
  EXPECT_FLOAT_EQ(sd1->sum_loss, sd2->sum_loss);
  EXPECT_FLOAT_EQ(sd1->example_number, sd2->example_number);
  EXPECT_FLOAT_EQ(sd1->total_features, sd2->total_features);

  auto backing_buffer = std::make_shared<std::vector<char>>();
  auto writer = VW::io::create_vector_writer(backing_buffer);
  delta.serialize(*writer);

  writer->flush();
  auto reader = VW::io::create_buffer_view(backing_buffer->data(), backing_buffer->size());
  auto deserialized_delta = VW::model_delta::deserialize(*reader);

  EXPECT_FLOAT_EQ(delta.unsafe_get_workspace_ptr()->sd->weighted_labeled_examples,
      deserialized_delta->unsafe_get_workspace_ptr()->sd->weighted_labeled_examples);
  EXPECT_FLOAT_EQ(delta.unsafe_get_workspace_ptr()->sd->weighted_unlabeled_examples,
      deserialized_delta->unsafe_get_workspace_ptr()->sd->weighted_unlabeled_examples);
  EXPECT_FLOAT_EQ(delta.unsafe_get_workspace_ptr()->sd->weighted_labels,
      deserialized_delta->unsafe_get_workspace_ptr()->sd->weighted_labels);
  EXPECT_FLOAT_EQ(
      delta.unsafe_get_workspace_ptr()->sd->sum_loss, deserialized_delta->unsafe_get_workspace_ptr()->sd->sum_loss);
  EXPECT_FLOAT_EQ(delta.unsafe_get_workspace_ptr()->sd->example_number,
      deserialized_delta->unsafe_get_workspace_ptr()->sd->example_number);
  EXPECT_FLOAT_EQ(delta.unsafe_get_workspace_ptr()->sd->total_features,
      deserialized_delta->unsafe_get_workspace_ptr()->sd->total_features);
}