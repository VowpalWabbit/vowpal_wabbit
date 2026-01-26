// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/learner.h"

#include "vw/core/global_data.h"
#include "vw/core/vw.h"
#include "vw/test_common/test_common.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <vector>

using namespace ::testing;

TEST(Learner, RequireSinglelineSucceedsForSinglelineLearner)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  // Default learner is singleline
  auto* l = VW::LEARNER::require_singleline(vw->l.get());
  EXPECT_NE(l, nullptr);
  EXPECT_FALSE(l->is_multiline());
}

TEST(Learner, RequireSinglelineThrowsForMultilineLearner)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--cb_explore_adf"));
  // cb_explore_adf creates a multiline learner
  EXPECT_THROW(VW::LEARNER::require_singleline(vw->l.get()), VW::vw_exception);
}

TEST(Learner, RequireMultilineSucceedsForMultilineLearner)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--cb_explore_adf"));
  // cb_explore_adf creates a multiline learner
  auto* l = VW::LEARNER::require_multiline(vw->l.get());
  EXPECT_NE(l, nullptr);
  EXPECT_TRUE(l->is_multiline());
}

TEST(Learner, RequireMultilineThrowsForSinglelineLearner)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  // Default learner is singleline
  EXPECT_THROW(VW::LEARNER::require_multiline(vw->l.get()), VW::vw_exception);
}

TEST(Learner, RequireSinglelineSharedPtrSucceedsForSinglelineLearner)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto l = VW::LEARNER::require_singleline(vw->l);
  EXPECT_NE(l, nullptr);
  EXPECT_FALSE(l->is_multiline());
}

TEST(Learner, RequireMultilineSharedPtrSucceedsForMultilineLearner)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--cb_explore_adf"));
  auto l = VW::LEARNER::require_multiline(vw->l);
  EXPECT_NE(l, nullptr);
  EXPECT_TRUE(l->is_multiline());
}

TEST(Learner, GetLearnerByNamePrefixFindsExistingLearner)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--oaa", "3"));
  // "oaa" should be in the learner stack
  auto* l = vw->l->get_learner_by_name_prefix("oaa");
  EXPECT_NE(l, nullptr);
  EXPECT_EQ(l->get_name(), "oaa");
}

TEST(Learner, GetLearnerByNamePrefixThrowsForNonExistentLearner)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  EXPECT_THROW(vw->l->get_learner_by_name_prefix("nonexistent_learner_xyz"), VW::vw_exception);
}

TEST(Learner, GetLearnerByNamePrefixFindsGD)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  // gd should be in every stack
  auto* l = vw->l->get_learner_by_name_prefix("gd");
  EXPECT_NE(l, nullptr);
}

TEST(Learner, GetEnabledLearners)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--oaa", "3"));
  std::vector<std::string> learners;
  vw->l->get_enabled_learners(learners);
  EXPECT_FALSE(learners.empty());
  // Should contain at least gd and oaa
  EXPECT_THAT(learners, Contains("gd"));
  EXPECT_THAT(learners, Contains("oaa"));
}

TEST(Learner, GetBaseLearner)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--oaa", "3"));
  // OAA is built on top of other learners
  auto* base = vw->l->get_base_learner();
  EXPECT_NE(base, nullptr);
}

TEST(Learner, GetBaseLearnerForBottomLearnerReturnsNull)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  // Navigate to the bottom of the stack
  auto* current = vw->l.get();
  while (current->get_base_learner() != nullptr) { current = current->get_base_learner(); }
  EXPECT_EQ(current->get_base_learner(), nullptr);
}

TEST(Learner, FeatureWidthForOAA)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--oaa", "5"));
  // OAA with 5 classes should have feature_width >= 5
  auto* oaa_learner = vw->l->get_learner_by_name_prefix("oaa");
  EXPECT_NE(oaa_learner, nullptr);
  EXPECT_GE(oaa_learner->feature_width, 5);
}

TEST(Learner, GetLearnerName)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--oaa", "3"));
  auto* oaa_learner = vw->l->get_learner_by_name_prefix("oaa");
  EXPECT_NE(oaa_learner, nullptr);
  EXPECT_EQ(oaa_learner->get_name(), "oaa");
}

TEST(Learner, IsMultilineReturnsFalseForSinglelineLearner)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  EXPECT_FALSE(vw->l->is_multiline());
}

TEST(Learner, IsMultilineReturnsTrueForMultilineLearner)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--cb_explore_adf"));
  EXPECT_TRUE(vw->l->is_multiline());
}

TEST(Learner, GetOutputPredictionType)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto pred_type = vw->l->get_output_prediction_type();
  EXPECT_EQ(pred_type, VW::prediction_type_t::SCALAR);
}

TEST(Learner, GetOutputPredictionTypeForOAA)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--oaa", "3"));
  auto pred_type = vw->l->get_output_prediction_type();
  EXPECT_EQ(pred_type, VW::prediction_type_t::MULTICLASS);
}

TEST(Learner, GetOutputPredictionTypeForCBAdf)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--cb_explore_adf"));
  auto pred_type = vw->l->get_output_prediction_type();
  EXPECT_EQ(pred_type, VW::prediction_type_t::ACTION_PROBS);
}

TEST(Learner, GetInputLabelType)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto label_type = vw->l->get_input_label_type();
  EXPECT_EQ(label_type, VW::label_type_t::SIMPLE);
}

TEST(Learner, GetInputLabelTypeForOAA)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--oaa", "3"));
  auto label_type = vw->l->get_input_label_type();
  EXPECT_EQ(label_type, VW::label_type_t::MULTICLASS);
}

TEST(Learner, SaveLoadBasic)
{
  // Create and train a model
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);

  // Save the model
  auto backing_vector = std::make_shared<std::vector<char>>();
  VW::io_buf io_writer;
  io_writer.add_file(VW::io::create_vector_writer(backing_vector));
  VW::save_predictor(*vw, io_writer);
  io_writer.flush();

  EXPECT_FALSE(backing_vector->empty());

  // Load the model
  auto vw2 = VW::initialize(vwtest::make_args("--quiet", "-t"),
      VW::io::create_buffer_view(backing_vector->data(), backing_vector->size()));
  EXPECT_NE(vw2, nullptr);
}

TEST(Learner, EndPassIsCalled)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--passes", "2", "-c"));
  // Just verify the learner has end_pass functionality
  // This is more of an integration test
  vw->l->end_pass();
}

TEST(Learner, FinishIsCalled)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  // Just verify the learner can be finished
  // This is automatically called by workspace destructor
  vw->l->finish();
}

TEST(Learner, HasLegacyFinishForOldLearners)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  // Check the API exists
  bool has_legacy = vw->l->has_legacy_finish();
  // The actual value depends on implementation
  (void)has_legacy;
}

TEST(Learner, HasUpdateStats)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--oaa", "3"));
  // Check the API exists and returns something
  bool has_update = vw->l->has_update_stats();
  (void)has_update;
}

TEST(Learner, HasPrintUpdate)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--oaa", "3"));
  bool has_print = vw->l->has_print_update();
  (void)has_print;
}

TEST(Learner, HasOutputExamplePrediction)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--oaa", "3"));
  bool has_output = vw->l->has_output_example_prediction();
  (void)has_output;
}

TEST(Learner, LearnReturnsPredictionForDefaultLearner)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  // The default learner (gd) has learn_returns_prediction = true
  EXPECT_TRUE(vw->l->learn_returns_prediction);
}

TEST(Learner, FeatureWidthBelowForReductionStack)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--oaa", "5"));
  // Feature width below should be set properly in the stack
  EXPECT_GE(vw->l->feature_width_below, 1);
}
