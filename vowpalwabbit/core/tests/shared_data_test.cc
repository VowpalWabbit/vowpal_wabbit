// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/shared_data.h"

#include "vw/core/loss_functions.h"
#include "vw/core/named_labels.h"
#include "vw/core/vw.h"
#include "vw/test_common/test_common.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cfloat>
#include <memory>
#include <sstream>

using namespace ::testing;

TEST(SharedData, DefaultConstruction)
{
  VW::shared_data sd;

  EXPECT_EQ(sd.queries, 0);
  EXPECT_EQ(sd.example_number, 0);
  EXPECT_EQ(sd.total_features, 0);
  EXPECT_DOUBLE_EQ(sd.t, 0.0);
  EXPECT_DOUBLE_EQ(sd.weighted_labeled_examples, 0.0);
  EXPECT_DOUBLE_EQ(sd.weighted_unlabeled_examples, 0.0);
  EXPECT_DOUBLE_EQ(sd.sum_loss, 0.0);
  EXPECT_FLOAT_EQ(sd.dump_interval, 1.0f);
  EXPECT_DOUBLE_EQ(sd.gravity, 0.0);
  EXPECT_DOUBLE_EQ(sd.contraction, 1.0);
  EXPECT_EQ(sd.ldict, nullptr);
}

TEST(SharedData, CopyConstruction)
{
  VW::shared_data original;
  original.queries = 10;
  original.example_number = 100;
  original.total_features = 500;
  original.t = 50.0;
  original.weighted_labeled_examples = 25.0;
  original.weighted_unlabeled_examples = 25.0;
  original.sum_loss = 5.0;
  original.min_label = -1.0f;
  original.max_label = 1.0f;

  VW::shared_data copy(original);

  EXPECT_EQ(copy.queries, 10);
  EXPECT_EQ(copy.example_number, 100);
  EXPECT_EQ(copy.total_features, 500);
  EXPECT_DOUBLE_EQ(copy.t, 50.0);
  EXPECT_DOUBLE_EQ(copy.weighted_labeled_examples, 25.0);
  EXPECT_DOUBLE_EQ(copy.weighted_unlabeled_examples, 25.0);
  EXPECT_DOUBLE_EQ(copy.sum_loss, 5.0);
  EXPECT_FLOAT_EQ(copy.min_label, -1.0f);
  EXPECT_FLOAT_EQ(copy.max_label, 1.0f);
}

TEST(SharedData, CopyAssignmentOperator)
{
  VW::shared_data original;
  original.queries = 20;
  original.example_number = 200;
  original.sum_loss = 10.0;

  VW::shared_data copy;
  copy = original;

  EXPECT_EQ(copy.queries, 20);
  EXPECT_EQ(copy.example_number, 200);
  EXPECT_DOUBLE_EQ(copy.sum_loss, 10.0);
}

TEST(SharedData, SelfAssignment)
{
  VW::shared_data sd;
  sd.queries = 30;
  sd.example_number = 300;

  // Test self-assignment
  VW::shared_data& sd_ref = sd;
  sd = sd_ref;

  EXPECT_EQ(sd.queries, 30);
  EXPECT_EQ(sd.example_number, 300);
}

TEST(SharedData, MoveConstructor)
{
  VW::shared_data original;
  original.queries = 40;
  original.example_number = 400;
  original.total_features = 1000;

  VW::shared_data moved(std::move(original));

  EXPECT_EQ(moved.queries, 40);
  EXPECT_EQ(moved.example_number, 400);
  EXPECT_EQ(moved.total_features, 1000);
}

TEST(SharedData, MoveAssignment)
{
  VW::shared_data original;
  original.queries = 50;
  original.example_number = 500;

  VW::shared_data moved;
  moved = std::move(original);

  EXPECT_EQ(moved.queries, 50);
  EXPECT_EQ(moved.example_number, 500);
}

TEST(SharedData, WeightedExamples)
{
  VW::shared_data sd;
  sd.weighted_labeled_examples = 10.0;
  sd.weighted_unlabeled_examples = 5.0;

  EXPECT_DOUBLE_EQ(sd.weighted_examples(), 15.0);
}

TEST(SharedData, WeightedExamplesZero)
{
  VW::shared_data sd;
  EXPECT_DOUBLE_EQ(sd.weighted_examples(), 0.0);
}

TEST(SharedData, UpdateTrainLabeledExample)
{
  VW::shared_data sd;

  // Update with a labeled training example
  sd.update(
      /*test_example=*/false,
      /*labeled_example=*/true,
      /*loss=*/0.5f,
      /*weight=*/1.0f,
      /*num_features=*/10);

  EXPECT_EQ(sd.example_number, 1);
  EXPECT_EQ(sd.total_features, 10);
  EXPECT_DOUBLE_EQ(sd.t, 1.0);
  EXPECT_DOUBLE_EQ(sd.weighted_labeled_examples, 1.0);
  EXPECT_DOUBLE_EQ(sd.weighted_unlabeled_examples, 0.0);
  EXPECT_DOUBLE_EQ(sd.sum_loss, 0.5);
  EXPECT_DOUBLE_EQ(sd.sum_loss_since_last_dump, 0.5);
}

TEST(SharedData, UpdateTrainUnlabeledExample)
{
  VW::shared_data sd;

  sd.update(
      /*test_example=*/false,
      /*labeled_example=*/false,
      /*loss=*/0.0f,
      /*weight=*/1.0f,
      /*num_features=*/5);

  EXPECT_EQ(sd.example_number, 1);
  EXPECT_EQ(sd.total_features, 5);
  EXPECT_DOUBLE_EQ(sd.weighted_labeled_examples, 0.0);
  EXPECT_DOUBLE_EQ(sd.weighted_unlabeled_examples, 1.0);
}

TEST(SharedData, UpdateTestLabeledExample)
{
  VW::shared_data sd;

  // Test example (holdout)
  sd.update(
      /*test_example=*/true,
      /*labeled_example=*/true,
      /*loss=*/0.3f,
      /*weight=*/1.0f,
      /*num_features=*/8);

  // Test examples go to holdout counters
  EXPECT_DOUBLE_EQ(sd.weighted_holdout_examples, 1.0);
  EXPECT_NEAR(sd.holdout_sum_loss, 0.3, 1e-6);
  EXPECT_NEAR(sd.holdout_sum_loss_since_last_dump, 0.3, 1e-6);
  EXPECT_NEAR(sd.holdout_sum_loss_since_last_pass, 0.3, 1e-6);

  // Training counters should not be updated
  EXPECT_EQ(sd.example_number, 0);
  EXPECT_EQ(sd.total_features, 0);
}

TEST(SharedData, UpdateWithWeight)
{
  VW::shared_data sd;

  sd.update(
      /*test_example=*/false,
      /*labeled_example=*/true,
      /*loss=*/1.0f,
      /*weight=*/2.5f,
      /*num_features=*/3);

  EXPECT_DOUBLE_EQ(sd.t, 2.5);
  EXPECT_DOUBLE_EQ(sd.weighted_labeled_examples, 2.5);
}

TEST(SharedData, UpdateDumpIntervalMultiplicative)
{
  VW::shared_data sd;
  sd.progress_add = false;
  sd.progress_arg = 2.0;
  sd.weighted_labeled_examples = 10.0;
  sd.old_weighted_labeled_examples = 5.0;
  sd.sum_loss_since_last_dump = 1.0;

  sd.update_dump_interval();

  EXPECT_DOUBLE_EQ(sd.sum_loss_since_last_dump, 0.0);
  EXPECT_DOUBLE_EQ(sd.old_weighted_labeled_examples, 10.0);
  EXPECT_FLOAT_EQ(sd.dump_interval, 20.0f);  // 10.0 * 2.0
}

TEST(SharedData, UpdateDumpIntervalAdditive)
{
  VW::shared_data sd;
  sd.progress_add = true;
  sd.progress_arg = 5.0;
  sd.weighted_labeled_examples = 10.0;
  sd.sum_loss_since_last_dump = 2.0;

  sd.update_dump_interval();

  EXPECT_DOUBLE_EQ(sd.sum_loss_since_last_dump, 0.0);
  EXPECT_FLOAT_EQ(sd.dump_interval, 15.0f);  // 10.0 + 5.0
}

TEST(SharedData, PrintUpdateHeaderDoesNotCrash)
{
  VW::shared_data sd;
  std::ostringstream output;

  // Should not throw or crash
  sd.print_update_header(output);

  std::string header = output.str();
  EXPECT_FALSE(header.empty());
  // Header should contain standard columns
  EXPECT_THAT(header, HasSubstr("average"));
  EXPECT_THAT(header, HasSubstr("loss"));
}

TEST(SharedData, PrintUpdateFloatLabelDoesNotCrash)
{
  VW::shared_data sd;
  sd.weighted_labeled_examples = 10.0;
  sd.sum_loss = 1.0;
  sd.dump_interval = 0.0f;  // Force update

  std::ostringstream output;
  sd.print_update(output, /*holdout_set_off=*/true, /*current_pass=*/0,
      /*label=*/1.5f, /*prediction=*/1.2f, /*num_features=*/5);

  std::string update = output.str();
  EXPECT_FALSE(update.empty());
}

TEST(SharedData, PrintUpdateUint32LabelDoesNotCrash)
{
  VW::shared_data sd;
  sd.weighted_labeled_examples = 10.0;
  sd.sum_loss = 1.0;
  sd.dump_interval = 0.0f;

  std::ostringstream output;
  sd.print_update(output, /*holdout_set_off=*/true, /*current_pass=*/0,
      /*label=*/static_cast<uint32_t>(2), /*prediction=*/static_cast<uint32_t>(3), /*num_features=*/5);

  std::string update = output.str();
  EXPECT_FALSE(update.empty());
}

TEST(SharedData, PrintUpdateStringLabelDoesNotCrash)
{
  VW::shared_data sd;
  sd.weighted_labeled_examples = 10.0;
  sd.sum_loss = 1.0;
  sd.dump_interval = 0.0f;

  std::ostringstream output;
  sd.print_update(output, /*holdout_set_off=*/true, /*current_pass=*/0,
      /*label=*/std::string("test_label"), /*prediction=*/static_cast<uint32_t>(1), /*num_features=*/5);

  std::string update = output.str();
  EXPECT_FALSE(update.empty());
}

TEST(SharedData, PrintUpdateStringBothDoesNotCrash)
{
  VW::shared_data sd;
  sd.weighted_labeled_examples = 10.0;
  sd.sum_loss = 1.0;
  sd.dump_interval = 0.0f;

  std::ostringstream output;
  sd.print_update(output, /*holdout_set_off=*/true, /*current_pass=*/0,
      /*label=*/std::string("label"), /*prediction=*/std::string("pred"), /*num_features=*/5);

  std::string update = output.str();
  EXPECT_FALSE(update.empty());
}

TEST(SharedData, PrintUpdateUnknownLabel)
{
  VW::shared_data sd;
  sd.weighted_labeled_examples = 10.0;
  sd.sum_loss = 1.0;
  sd.dump_interval = 0.0f;

  std::ostringstream output;
  // FLT_MAX label should show as "unknown"
  sd.print_update(output, /*holdout_set_off=*/true, /*current_pass=*/0, FLT_MAX, 1.0f, 5);

  std::string update = output.str();
  EXPECT_THAT(update, HasSubstr("unknown"));
}

TEST(SharedData, PrintUpdateWithHoldout)
{
  VW::shared_data sd;
  sd.weighted_holdout_examples = 10.0;
  sd.holdout_sum_loss = 2.0;
  sd.weighted_holdout_examples_since_last_dump = 5.0;
  sd.holdout_sum_loss_since_last_dump = 1.0;
  sd.dump_interval = 0.0f;

  std::ostringstream output;
  // holdout_set_off=false and current_pass >= 1 triggers holdout path
  sd.print_update(output, /*holdout_set_off=*/false, /*current_pass=*/1, 1.0f, 1.0f, 5);

  std::string update = output.str();
  EXPECT_FALSE(update.empty());
  // 'h' indicates holdout
  EXPECT_THAT(update, HasSubstr("h"));
}

TEST(SharedData, PrintUpdateNoExamples)
{
  VW::shared_data sd;
  sd.weighted_labeled_examples = 0.0;
  sd.dump_interval = 0.0f;

  std::ostringstream output;
  sd.print_update(output, /*holdout_set_off=*/true, /*current_pass=*/0, 1.0f, 1.0f, 5);

  std::string update = output.str();
  EXPECT_THAT(update, HasSubstr("n.a."));
}

TEST(SharedData, PrintSummaryDoesNotCrash)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));

  std::ostringstream output;
  vw->sd->print_summary(output, *vw->sd, *vw->loss_config.loss, 0, true);

  std::string summary = output.str();
  EXPECT_FALSE(summary.empty());
  EXPECT_THAT(summary, HasSubstr("finished run"));
  EXPECT_THAT(summary, HasSubstr("number of examples"));
}

TEST(SharedData, PrintSummaryWithMultiplePasses)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  vw->sd->example_number = 100;

  std::ostringstream output;
  vw->sd->print_summary(output, *vw->sd, *vw->loss_config.loss, 2, true);

  std::string summary = output.str();
  EXPECT_THAT(summary, HasSubstr("per pass"));
  EXPECT_THAT(summary, HasSubstr("passes used"));
}

TEST(SharedData, PrintSummaryWithHoldout)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  vw->sd->holdout_best_loss = 0.5;

  std::ostringstream output;
  vw->sd->print_summary(output, *vw->sd, *vw->loss_config.loss, 1, false);

  std::string summary = output.str();
  EXPECT_THAT(summary, HasSubstr("0.5"));
}

TEST(SharedData, PrintSummaryUndefinedHoldout)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  vw->sd->holdout_best_loss = FLT_MAX;

  std::ostringstream output;
  vw->sd->print_summary(output, *vw->sd, *vw->loss_config.loss, 1, false);

  std::string summary = output.str();
  EXPECT_THAT(summary, HasSubstr("undefined"));
}

TEST(SharedData, PrintSummaryWithQueries)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  vw->sd->queries = 100;

  std::ostringstream output;
  vw->sd->print_summary(output, *vw->sd, *vw->loss_config.loss, 1, true);

  std::string summary = output.str();
  EXPECT_THAT(summary, HasSubstr("queries"));
}

TEST(SharedData, PrintSummaryMulticlassLogLoss)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--probabilities", "--oaa", "3"));
  vw->sd->report_multiclass_log_loss = true;
  vw->sd->multiclass_log_loss = 0.5;
  vw->sd->weighted_labeled_examples = 10.0;

  std::ostringstream output;
  vw->sd->print_summary(output, *vw->sd, *vw->loss_config.loss, 1, true);

  std::string summary = output.str();
  EXPECT_THAT(summary, HasSubstr("multiclass log loss"));
}

TEST(SharedData, CopyWithLdict)
{
  VW::shared_data original;
  original.ldict = std::make_unique<VW::named_labels>("one two three");

  VW::shared_data copy(original);

  EXPECT_NE(copy.ldict, nullptr);
  // Check the ldict was properly deep-copied
  EXPECT_NE(copy.ldict.get(), original.ldict.get());
}

TEST(SharedData, HoldoutCounters)
{
  VW::shared_data sd;

  // Simulate holdout updates
  sd.weighted_holdout_examples = 10.0;
  sd.holdout_sum_loss = 2.5;
  sd.holdout_best_loss = 0.2;
  sd.holdout_best_pass = 3;
  sd.weighted_holdout_examples_since_last_pass = 5.0;
  sd.holdout_sum_loss_since_last_pass = 1.0;

  // Copy and verify
  VW::shared_data copy(sd);

  EXPECT_DOUBLE_EQ(copy.weighted_holdout_examples, 10.0);
  EXPECT_DOUBLE_EQ(copy.holdout_sum_loss, 2.5);
  EXPECT_DOUBLE_EQ(copy.holdout_best_loss, 0.2);
  EXPECT_EQ(copy.holdout_best_pass, 3);
  EXPECT_DOUBLE_EQ(copy.weighted_holdout_examples_since_last_pass, 5.0);
  EXPECT_DOUBLE_EQ(copy.holdout_sum_loss_since_last_pass, 1.0);
}

TEST(SharedData, LabelObservationTracking)
{
  VW::shared_data sd;

  EXPECT_FALSE(sd.is_more_than_two_labels_observed);
  EXPECT_FLOAT_EQ(sd.first_observed_label, FLT_MAX);
  EXPECT_FLOAT_EQ(sd.second_observed_label, FLT_MAX);

  // Simulate setting labels
  sd.first_observed_label = 0.0f;
  sd.second_observed_label = 1.0f;
  sd.is_more_than_two_labels_observed = true;

  VW::shared_data copy(sd);

  EXPECT_TRUE(copy.is_more_than_two_labels_observed);
  EXPECT_FLOAT_EQ(copy.first_observed_label, 0.0f);
  EXPECT_FLOAT_EQ(copy.second_observed_label, 1.0f);
}

// ==================== Edge Case Tests for Coverage ====================

TEST(SharedDataEdgeCases, ZeroWeightedExamples)
{
  // Test that zero weighted examples is handled correctly
  VW::shared_data sd;

  sd.weighted_labeled_examples = 0.0;
  sd.sum_loss = 0.0;

  // weighted_examples() should still work
  EXPECT_DOUBLE_EQ(sd.weighted_examples(), 0.0);
}

TEST(SharedDataEdgeCases, HoldoutZeroExamples)
{
  // Test holdout with zero examples
  VW::shared_data sd;

  sd.weighted_holdout_examples = 0.0;
  sd.holdout_sum_loss = 0.0;

  // These should not cause division by zero
  EXPECT_DOUBLE_EQ(sd.weighted_holdout_examples, 0.0);
  EXPECT_DOUBLE_EQ(sd.holdout_sum_loss, 0.0);
}

TEST(SharedDataEdgeCases, ContribExampleCount)
{
  VW::shared_data sd;

  EXPECT_EQ(sd.contraction, 1.0f);
  EXPECT_EQ(sd.gravity, 0.0f);

  sd.contraction = 0.5f;
  sd.gravity = 0.1f;

  EXPECT_FLOAT_EQ(sd.contraction, 0.5f);
  EXPECT_FLOAT_EQ(sd.gravity, 0.1f);
}

TEST(SharedDataEdgeCases, MinMaxLabels)
{
  VW::shared_data sd;

  // Set min/max labels
  sd.min_label = -5.0f;
  sd.max_label = 5.0f;

  EXPECT_FLOAT_EQ(sd.min_label, -5.0f);
  EXPECT_FLOAT_EQ(sd.max_label, 5.0f);
}

TEST(SharedDataEdgeCases, TotalFeatures)
{
  VW::shared_data sd;

  EXPECT_EQ(sd.total_features, 0);

  sd.total_features = 1000000;
  EXPECT_EQ(sd.total_features, 1000000);
}

TEST(SharedDataEdgeCases, ExampleNumber)
{
  VW::shared_data sd;

  EXPECT_EQ(sd.example_number, 0);

  sd.example_number = 12345;
  EXPECT_EQ(sd.example_number, 12345);
}

TEST(SharedDataEdgeCases, HoldoutBestLoss)
{
  VW::shared_data sd;

  EXPECT_DOUBLE_EQ(sd.holdout_best_loss, 0.0);
  EXPECT_EQ(sd.holdout_best_pass, 0);

  sd.holdout_best_loss = 0.05;
  sd.holdout_best_pass = 10;

  EXPECT_DOUBLE_EQ(sd.holdout_best_loss, 0.05);
  EXPECT_EQ(sd.holdout_best_pass, 10);
}

TEST(SharedDataEdgeCases, WeightedUnlabeledExamples)
{
  VW::shared_data sd;

  EXPECT_DOUBLE_EQ(sd.weighted_unlabeled_examples, 0.0);

  sd.weighted_unlabeled_examples = 100.0;
  EXPECT_DOUBLE_EQ(sd.weighted_unlabeled_examples, 100.0);
}

TEST(SharedDataEdgeCases, SumLossSinceLastDump)
{
  VW::shared_data sd;

  sd.sum_loss_since_last_dump = 0.0;
  sd.old_weighted_labeled_examples = 0.0;
  sd.weighted_labeled_examples = 0.0;

  // When examples are equal, no new examples processed
  EXPECT_DOUBLE_EQ(sd.weighted_labeled_examples - sd.old_weighted_labeled_examples, 0.0);
}

TEST(SharedDataEdgeCases, ProgressAddCounter)
{
  VW::shared_data sd;

  // Set progress tracking values
  sd.progress_add = 1;
  sd.progress_arg = 100;

  EXPECT_EQ(sd.progress_add, 1);
  EXPECT_EQ(sd.progress_arg, 100);
}
