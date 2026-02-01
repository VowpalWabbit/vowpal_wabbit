// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

// Batch 7: Tests to increase line coverage by 100+ lines.
// Targets: shared_data.cc, debug_print.cc, parse_primitives.cc,
//          confidence_sequence.cc, cressieread.cc, mf.cc, no_label.cc

#include "vw/core/debug_print.h"
#include "vw/core/estimators/confidence_sequence.h"
#include "vw/core/estimators/cressieread.h"
#include "vw/core/example.h"
#include "vw/core/memory.h"
#include "vw/core/metric_sink.h"
#include "vw/core/named_labels.h"
#include "vw/core/no_label.h"
#include "vw/core/parse_primitives.h"
#include "vw/core/shared_data.h"
#include "vw/core/vw.h"
#include "vw/io/logger.h"
#include "vw/test_common/test_common.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cfloat>
#include <cmath>
#include <utility>

// ============================================================
// shared_data.cc coverage: copy assignment, move ctor, move assignment
// (lines 56-158 are uncovered = ~103 lines)
// ============================================================

TEST(SharedDataBatch7, CopyAssignment)
{
  VW::shared_data original;
  original.queries = 42;
  original.example_number = 100;
  original.total_features = 500;
  original.t = 1.5;
  original.weighted_labeled_examples = 10.0;
  original.old_weighted_labeled_examples = 5.0;
  original.weighted_unlabeled_examples = 3.0;
  original.weighted_labels = 7.0;
  original.sum_loss = 2.5;
  original.sum_loss_since_last_dump = 1.0;
  original.dump_interval = 4.0f;
  original.gravity = 0.1;
  original.contraction = 0.9;
  original.min_label = -1.0f;
  original.max_label = 1.0f;
  original.weighted_holdout_examples = 2.0;
  original.weighted_holdout_examples_since_last_dump = 1.5;
  original.holdout_sum_loss_since_last_dump = 0.5;
  original.holdout_sum_loss = 1.0;
  original.holdout_best_loss = 0.3;
  original.weighted_holdout_examples_since_last_pass = 0.8;
  original.holdout_sum_loss_since_last_pass = 0.4;
  original.holdout_best_pass = 3;
  original.report_multiclass_log_loss = true;
  original.multiclass_log_loss = 0.6;
  original.holdout_multiclass_log_loss = 0.7;
  original.is_more_than_two_labels_observed = true;
  original.first_observed_label = 0.0f;
  original.second_observed_label = 1.0f;

  VW::shared_data assigned;
  assigned = original;

  EXPECT_EQ(assigned.queries, 42u);
  EXPECT_EQ(assigned.example_number, 100u);
  EXPECT_EQ(assigned.total_features, 500u);
  EXPECT_DOUBLE_EQ(assigned.t, 1.5);
  EXPECT_DOUBLE_EQ(assigned.weighted_labeled_examples, 10.0);
  EXPECT_DOUBLE_EQ(assigned.old_weighted_labeled_examples, 5.0);
  EXPECT_DOUBLE_EQ(assigned.weighted_unlabeled_examples, 3.0);
  EXPECT_DOUBLE_EQ(assigned.weighted_labels, 7.0);
  EXPECT_DOUBLE_EQ(assigned.sum_loss, 2.5);
  EXPECT_DOUBLE_EQ(assigned.sum_loss_since_last_dump, 1.0);
  EXPECT_FLOAT_EQ(assigned.dump_interval, 4.0f);
  EXPECT_DOUBLE_EQ(assigned.gravity, 0.1);
  EXPECT_DOUBLE_EQ(assigned.contraction, 0.9);
  EXPECT_FLOAT_EQ(assigned.min_label, -1.0f);
  EXPECT_FLOAT_EQ(assigned.max_label, 1.0f);
  EXPECT_DOUBLE_EQ(assigned.weighted_holdout_examples, 2.0);
  EXPECT_DOUBLE_EQ(assigned.holdout_sum_loss, 1.0);
  EXPECT_DOUBLE_EQ(assigned.holdout_best_loss, 0.3);
  EXPECT_EQ(assigned.holdout_best_pass, 3u);
  EXPECT_TRUE(assigned.report_multiclass_log_loss);
  EXPECT_DOUBLE_EQ(assigned.multiclass_log_loss, 0.6);
  EXPECT_DOUBLE_EQ(assigned.holdout_multiclass_log_loss, 0.7);
  EXPECT_TRUE(assigned.is_more_than_two_labels_observed);
  EXPECT_FLOAT_EQ(assigned.first_observed_label, 0.0f);
  EXPECT_FLOAT_EQ(assigned.second_observed_label, 1.0f);
}

TEST(SharedDataBatch7, CopyAssignmentSelfAssign)
{
  VW::shared_data sd;
  sd.queries = 10;
  sd = sd;
  EXPECT_EQ(sd.queries, 10u);
}

TEST(SharedDataBatch7, CopyAssignmentWithNamedLabels)
{
  VW::shared_data original;
  original.queries = 5;
  original.ldict = VW::make_unique<VW::named_labels>("hello world");

  VW::shared_data assigned;
  assigned = original;

  EXPECT_EQ(assigned.queries, 5u);
  EXPECT_NE(assigned.ldict, nullptr);
}

TEST(SharedDataBatch7, MoveConstructor)
{
  VW::shared_data original;
  original.queries = 77;
  original.example_number = 200;
  original.total_features = 1000;
  original.t = 3.14;
  original.weighted_labeled_examples = 20.0;
  original.old_weighted_labeled_examples = 15.0;
  original.weighted_unlabeled_examples = 5.0;
  original.weighted_labels = 12.0;
  original.sum_loss = 4.0;
  original.sum_loss_since_last_dump = 2.0;
  original.dump_interval = 8.0f;
  original.gravity = 0.2;
  original.contraction = 0.8;
  original.min_label = -2.0f;
  original.max_label = 2.0f;
  original.ldict = VW::make_unique<VW::named_labels>("cat dog");
  original.weighted_holdout_examples = 4.0;
  original.weighted_holdout_examples_since_last_dump = 3.0;
  original.holdout_sum_loss_since_last_dump = 1.5;
  original.holdout_sum_loss = 2.5;
  original.holdout_best_loss = 0.5;
  original.weighted_holdout_examples_since_last_pass = 1.2;
  original.holdout_sum_loss_since_last_pass = 0.6;
  original.holdout_best_pass = 5;
  original.report_multiclass_log_loss = true;
  original.multiclass_log_loss = 1.1;
  original.holdout_multiclass_log_loss = 1.2;
  original.is_more_than_two_labels_observed = true;
  original.first_observed_label = -1.0f;
  original.second_observed_label = 1.0f;

  VW::shared_data moved(std::move(original));

  EXPECT_EQ(moved.queries, 77u);
  EXPECT_EQ(moved.example_number, 200u);
  EXPECT_EQ(moved.total_features, 1000u);
  EXPECT_DOUBLE_EQ(moved.t, 3.14);
  EXPECT_DOUBLE_EQ(moved.weighted_labeled_examples, 20.0);
  EXPECT_DOUBLE_EQ(moved.old_weighted_labeled_examples, 15.0);
  EXPECT_DOUBLE_EQ(moved.weighted_unlabeled_examples, 5.0);
  EXPECT_DOUBLE_EQ(moved.weighted_labels, 12.0);
  EXPECT_DOUBLE_EQ(moved.sum_loss, 4.0);
  EXPECT_FLOAT_EQ(moved.dump_interval, 8.0f);
  EXPECT_DOUBLE_EQ(moved.gravity, 0.2);
  EXPECT_DOUBLE_EQ(moved.contraction, 0.8);
  EXPECT_FLOAT_EQ(moved.min_label, -2.0f);
  EXPECT_FLOAT_EQ(moved.max_label, 2.0f);
  EXPECT_NE(moved.ldict, nullptr);
  EXPECT_DOUBLE_EQ(moved.weighted_holdout_examples, 4.0);
  EXPECT_DOUBLE_EQ(moved.holdout_sum_loss, 2.5);
  EXPECT_DOUBLE_EQ(moved.holdout_best_loss, 0.5);
  EXPECT_EQ(moved.holdout_best_pass, 5u);
  EXPECT_TRUE(moved.report_multiclass_log_loss);
  EXPECT_TRUE(moved.is_more_than_two_labels_observed);
  EXPECT_FLOAT_EQ(moved.first_observed_label, -1.0f);
  EXPECT_FLOAT_EQ(moved.second_observed_label, 1.0f);
}

TEST(SharedDataBatch7, MoveAssignment)
{
  VW::shared_data original;
  original.queries = 99;
  original.example_number = 300;
  original.total_features = 2000;
  original.t = 2.72;
  original.weighted_labeled_examples = 30.0;
  original.old_weighted_labeled_examples = 25.0;
  original.weighted_unlabeled_examples = 8.0;
  original.weighted_labels = 18.0;
  original.sum_loss = 6.0;
  original.sum_loss_since_last_dump = 3.0;
  original.dump_interval = 16.0f;
  original.gravity = 0.3;
  original.contraction = 0.7;
  original.min_label = -3.0f;
  original.max_label = 3.0f;
  original.ldict = VW::make_unique<VW::named_labels>("alpha beta");
  original.weighted_holdout_examples = 6.0;
  original.weighted_holdout_examples_since_last_dump = 5.0;
  original.holdout_sum_loss_since_last_dump = 2.5;
  original.holdout_sum_loss = 3.5;
  original.holdout_best_loss = 0.7;
  original.weighted_holdout_examples_since_last_pass = 1.5;
  original.holdout_sum_loss_since_last_pass = 0.8;
  original.holdout_best_pass = 7;
  original.report_multiclass_log_loss = true;
  original.multiclass_log_loss = 1.5;
  original.holdout_multiclass_log_loss = 1.6;
  original.is_more_than_two_labels_observed = true;
  original.first_observed_label = -2.0f;
  original.second_observed_label = 2.0f;

  VW::shared_data assigned;
  assigned = std::move(original);

  EXPECT_EQ(assigned.queries, 99u);
  EXPECT_EQ(assigned.example_number, 300u);
  EXPECT_EQ(assigned.total_features, 2000u);
  EXPECT_DOUBLE_EQ(assigned.t, 2.72);
  EXPECT_DOUBLE_EQ(assigned.weighted_labeled_examples, 30.0);
  EXPECT_DOUBLE_EQ(assigned.old_weighted_labeled_examples, 25.0);
  EXPECT_DOUBLE_EQ(assigned.weighted_unlabeled_examples, 8.0);
  EXPECT_DOUBLE_EQ(assigned.weighted_labels, 18.0);
  EXPECT_DOUBLE_EQ(assigned.sum_loss, 6.0);
  EXPECT_FLOAT_EQ(assigned.dump_interval, 16.0f);
  EXPECT_DOUBLE_EQ(assigned.gravity, 0.3);
  EXPECT_DOUBLE_EQ(assigned.contraction, 0.7);
  EXPECT_FLOAT_EQ(assigned.min_label, -3.0f);
  EXPECT_FLOAT_EQ(assigned.max_label, 3.0f);
  EXPECT_NE(assigned.ldict, nullptr);
  EXPECT_DOUBLE_EQ(assigned.weighted_holdout_examples, 6.0);
  EXPECT_DOUBLE_EQ(assigned.holdout_sum_loss, 3.5);
  EXPECT_DOUBLE_EQ(assigned.holdout_best_loss, 0.7);
  EXPECT_EQ(assigned.holdout_best_pass, 7u);
  EXPECT_TRUE(assigned.report_multiclass_log_loss);
  EXPECT_TRUE(assigned.is_more_than_two_labels_observed);
  EXPECT_FLOAT_EQ(assigned.first_observed_label, -2.0f);
  EXPECT_FLOAT_EQ(assigned.second_observed_label, 2.0f);
}

// ============================================================
// debug_print.cc coverage: all functions (59 uncovered lines)
// ============================================================

TEST(DebugPrintBatch7, SimpleLabelToString)
{
  auto ex = VW::make_unique<VW::example>();
  ex->l.simple.label = 1.5f;
  ex->weight = 2.0f;

  std::string result = VW::debug::simple_label_to_string(*ex);
  EXPECT_THAT(result, testing::HasSubstr("l=1.5"));
  EXPECT_THAT(result, testing::HasSubstr("w=2"));
}

TEST(DebugPrintBatch7, CbLabelToString)
{
  auto ex = VW::make_unique<VW::example>();
  VW::cb_class cl;
  cl.cost = 0.5f;
  cl.action = 1;
  cl.probability = 0.8f;
  cl.partial_prediction = 0.0f;
  ex->l.cb.costs.push_back(cl);

  std::string result = VW::debug::cb_label_to_string(*ex);
  EXPECT_THAT(result, testing::HasSubstr("l.cb="));
  EXPECT_THAT(result, testing::HasSubstr("c=0.5"));
  EXPECT_THAT(result, testing::HasSubstr("a=1"));
  EXPECT_THAT(result, testing::HasSubstr("p=0.8"));
}

TEST(DebugPrintBatch7, ScalarPredToString)
{
  auto ex = VW::make_unique<VW::example>();
  ex->pred.scalar = 0.75f;
  ex->partial_prediction = 0.3f;

  std::string result = VW::debug::scalar_pred_to_string(*ex);
  EXPECT_THAT(result, testing::HasSubstr("p=0.75"));
  EXPECT_THAT(result, testing::HasSubstr("pp=0.3"));
}

TEST(DebugPrintBatch7, ActionScorePredToString)
{
  auto ex = VW::make_unique<VW::example>();
  ex->pred.a_s.push_back({0, 0.6f});
  ex->pred.a_s.push_back({1, 0.4f});

  std::string result = VW::debug::a_s_pred_to_string(*ex);
  EXPECT_THAT(result, testing::HasSubstr("ec.pred.a_s"));
}

TEST(DebugPrintBatch7, MulticlassPredToString)
{
  auto ex = VW::make_unique<VW::example>();
  ex->pred.multiclass = 2;

  std::string result = VW::debug::multiclass_pred_to_string(*ex);
  EXPECT_THAT(result, testing::HasSubstr("ec.pred.multiclass = 2"));
}

TEST(DebugPrintBatch7, ProbDistPredToString)
{
  auto ex = VW::make_unique<VW::example>();
  VW::continuous_actions::pdf_segment seg;
  seg.left = 0.0f;
  seg.right = 0.5f;
  seg.pdf_value = 1.0f;
  ex->pred.pdf.push_back(seg);

  std::string result = VW::debug::prob_dist_pred_to_string(*ex);
  EXPECT_THAT(result, testing::HasSubstr("ec.pred.prob_dist"));
}

TEST(DebugPrintBatch7, FeaturesToString)
{
  auto ex = VW::make_unique<VW::example>();
  // Manually add features
  auto ns_index = static_cast<VW::namespace_index>('a');
  ex->feature_space[ns_index].push_back(1.0f, 42);
  ex->feature_space[ns_index].push_back(2.0f, 99);
  ex->indices.push_back(ns_index);

  std::string result = VW::debug::features_to_string(*ex);
  EXPECT_THAT(result, testing::HasSubstr("off="));
  EXPECT_THAT(result, testing::HasSubstr("h="));
  EXPECT_THAT(result, testing::HasSubstr("v="));
}

TEST(DebugPrintBatch7, DebugDepthIndentStringZero)
{
  std::string result = VW::debug::debug_depth_indent_string(0);
  EXPECT_EQ(result, "- ");
}

TEST(DebugPrintBatch7, DebugDepthIndentStringPositive)
{
  std::string depth1 = VW::debug::debug_depth_indent_string(1);
  EXPECT_EQ(depth1, "- ");

  std::string depth2 = VW::debug::debug_depth_indent_string(2);
  EXPECT_EQ(depth2, "  - ");

  std::string depth3 = VW::debug::debug_depth_indent_string(3);
  EXPECT_EQ(depth3, "    - ");
}

TEST(DebugPrintBatch7, DebugDepthIndentStringFromExample)
{
  auto ex = VW::make_unique<VW::example>();
  ex->debug_current_reduction_depth = 2;

  std::string result = VW::debug::debug_depth_indent_string(*ex);
  EXPECT_EQ(result, "  - ");
}

TEST(DebugPrintBatch7, DebugDepthIndentStringFromMultiEx)
{
  auto ex = VW::make_unique<VW::example>();
  ex->debug_current_reduction_depth = 3;
  VW::multi_ex multi;
  multi.push_back(ex.get());

  std::string result = VW::debug::debug_depth_indent_string(multi);
  EXPECT_EQ(result, "    - ");
}

// ============================================================
// parse_primitives.cc coverage (58 uncovered lines)
// escaped_tokenize, trim_whitespace, split_command_line, split_by_limit
// ============================================================

TEST(ParsePrimitivesBatch7, EscapedTokenizeBasic)
{
  auto tokens = VW::details::escaped_tokenize(' ', "hello world");
  ASSERT_EQ(tokens.size(), 2u);
  EXPECT_EQ(tokens[0], "hello");
  EXPECT_EQ(tokens[1], "world");
}

TEST(ParsePrimitivesBatch7, EscapedTokenizeWithEscape)
{
  auto tokens = VW::details::escaped_tokenize(' ', "hello\\ world foo");
  ASSERT_EQ(tokens.size(), 2u);
  EXPECT_EQ(tokens[0], "hello world");
  EXPECT_EQ(tokens[1], "foo");
}

TEST(ParsePrimitivesBatch7, EscapedTokenizeWithEscapeAtEnd)
{
  auto tokens = VW::details::escaped_tokenize(' ', "hello\\");
  ASSERT_EQ(tokens.size(), 1u);
  EXPECT_EQ(tokens[0], "hello");
}

TEST(ParsePrimitivesBatch7, EscapedTokenizeEmpty)
{
  auto tokens = VW::details::escaped_tokenize(' ', "");
  EXPECT_TRUE(tokens.empty());
}

TEST(ParsePrimitivesBatch7, EscapedTokenizeAllowEmpty)
{
  auto tokens = VW::details::escaped_tokenize(',', "a,,b", true);
  ASSERT_EQ(tokens.size(), 3u);
  EXPECT_EQ(tokens[0], "a");
  EXPECT_EQ(tokens[1], "");
  EXPECT_EQ(tokens[2], "b");
}

TEST(ParsePrimitivesBatch7, EscapedTokenizeTrailingDelimAllowEmpty)
{
  // "a," with allow_empty: trailing delimiter with last_space=true produces ["a", ""]
  auto tokens = VW::details::escaped_tokenize(',', "a,,", true);
  ASSERT_EQ(tokens.size(), 3u);
  EXPECT_EQ(tokens[0], "a");
  EXPECT_EQ(tokens[1], "");
  EXPECT_EQ(tokens[2], "");
}

TEST(ParsePrimitivesBatch7, TrimWhitespaceString)
{
  EXPECT_EQ(VW::trim_whitespace(std::string("  hello  ")), "hello");
  EXPECT_EQ(VW::trim_whitespace(std::string("")), "");
  EXPECT_EQ(VW::trim_whitespace(std::string("   ")), "");
  EXPECT_EQ(VW::trim_whitespace(std::string("no_trim")), "no_trim");
  EXPECT_EQ(VW::trim_whitespace(std::string("\t\n hello \t\n")), "hello");
}

TEST(ParsePrimitivesBatch7, TrimWhitespaceStringView)
{
  EXPECT_EQ(VW::trim_whitespace(VW::string_view("  hello  ")), "hello");
  EXPECT_EQ(VW::trim_whitespace(VW::string_view("")), "");
  EXPECT_EQ(VW::trim_whitespace(VW::string_view("   ")), "");
  EXPECT_EQ(VW::trim_whitespace(VW::string_view("abc")), "abc");
}

TEST(ParsePrimitivesBatch7, SplitCommandLineBasic)
{
  auto tokens = VW::split_command_line(std::string("--quiet --learning_rate 0.5"));
  ASSERT_EQ(tokens.size(), 3u);
  EXPECT_EQ(tokens[0], "--quiet");
  EXPECT_EQ(tokens[1], "--learning_rate");
  EXPECT_EQ(tokens[2], "0.5");
}

TEST(ParsePrimitivesBatch7, SplitCommandLineWithQuotes)
{
  auto tokens = VW::split_command_line(VW::string_view("--msg \"hello world\" --flag"));
  ASSERT_EQ(tokens.size(), 3u);
  EXPECT_EQ(tokens[0], "--msg");
  EXPECT_EQ(tokens[1], "hello world");
  EXPECT_EQ(tokens[2], "--flag");
}

TEST(ParsePrimitivesBatch7, SplitCommandLineWithEscape)
{
  auto tokens = VW::split_command_line(VW::string_view("--val hello\\ world"));
  ASSERT_EQ(tokens.size(), 2u);
  EXPECT_EQ(tokens[0], "--val");
  EXPECT_EQ(tokens[1], "hello world");
}

TEST(ParsePrimitivesBatch7, SplitCommandLineEscapeSequences)
{
  auto tokens = VW::split_command_line(VW::string_view("\\n \\t \\x"));
  ASSERT_EQ(tokens.size(), 3u);
  EXPECT_EQ(tokens[0], "\n");
  EXPECT_EQ(tokens[1], "\t");
  EXPECT_EQ(tokens[2], "x");
}

TEST(ParsePrimitivesBatch7, SplitCommandLineEmpty)
{
  auto tokens = VW::split_command_line(std::string(""));
  EXPECT_TRUE(tokens.empty());
}

TEST(ParsePrimitivesBatch7, SplitCommandLineNestedQuotes)
{
  auto tokens = VW::split_command_line(VW::string_view("'hello \"inner\" world'"));
  ASSERT_EQ(tokens.size(), 1u);
  EXPECT_EQ(tokens[0], "hello \"inner\" world");
}

TEST(ParsePrimitivesBatch7, SplitByLimitBasic)
{
  auto parts = VW::split_by_limit(VW::string_view("abcdefgh"), 3);
  ASSERT_EQ(parts.size(), 3u);
  EXPECT_EQ(parts[0], "abc");
  EXPECT_EQ(parts[1], "def");
  EXPECT_EQ(parts[2], "gh");
}

TEST(ParsePrimitivesBatch7, SplitByLimitExact)
{
  auto parts = VW::split_by_limit(VW::string_view("abcdef"), 3);
  ASSERT_EQ(parts.size(), 2u);
  EXPECT_EQ(parts[0], "abc");
  EXPECT_EQ(parts[1], "def");
}

TEST(ParsePrimitivesBatch7, SplitByLimitEmpty)
{
  auto parts = VW::split_by_limit(VW::string_view(""), 5);
  EXPECT_TRUE(parts.empty());
}

TEST(ParsePrimitivesBatch7, SplitByLimitLargerThanString)
{
  auto parts = VW::split_by_limit(VW::string_view("hi"), 10);
  ASSERT_EQ(parts.size(), 1u);
  EXPECT_EQ(parts[0], "hi");
}

// ============================================================
// cressieread.cc coverage (41 uncovered lines)
// ============================================================

TEST(CressieReadBatch7, UpdateAndBounds)
{
  VW::estimators::cressieread cr;
  cr.update(1.0f, 0.5f);
  cr.update(1.0f, 0.3f);
  cr.update(1.0f, 0.7f);

  EXPECT_FLOAT_EQ(cr.current_ips(), (0.5f + 0.3f + 0.7f) / 3.0f);
  EXPECT_EQ(cr.update_count, 3u);
  EXPECT_FLOAT_EQ(cr.last_w, 1.0f);
  EXPECT_FLOAT_EQ(cr.last_r, 0.7f);
}

TEST(CressieReadBatch7, CurrentIpsWithZeroUpdates)
{
  VW::estimators::cressieread cr;
  EXPECT_FLOAT_EQ(cr.current_ips(), 0.0f);
}

TEST(CressieReadBatch7, LowerAndUpperBound)
{
  VW::estimators::cressieread cr;
  for (int i = 0; i < 20; ++i) { cr.update(1.0f, 0.5f); }
  float lb = cr.lower_bound();
  float ub = cr.upper_bound();
  EXPECT_LE(lb, ub);
}

TEST(CressieReadBatch7, ResetStats)
{
  VW::estimators::cressieread cr;
  cr.update(1.0f, 0.5f);
  cr.update(2.0f, 0.3f);
  EXPECT_EQ(cr.update_count, 2u);

  cr.reset_stats();
  EXPECT_EQ(cr.update_count, 0u);
  EXPECT_FLOAT_EQ(cr.ips, 0.0f);
  EXPECT_FLOAT_EQ(cr.last_w, 0.0f);
  EXPECT_FLOAT_EQ(cr.last_r, 0.0f);
}

TEST(CressieReadBatch7, PersistMetrics)
{
  VW::estimators::cressieread cr;
  cr.update(1.0f, 0.5f);
  cr.update(1.0f, 0.8f);

  VW::metric_sink metrics;
  cr.persist(metrics, "_test");

  EXPECT_EQ(metrics.get_uint("upcnt_test"), 2u);
  EXPECT_GT(metrics.get_float("ips_test"), 0.0f);
  EXPECT_FLOAT_EQ(metrics.get_float("w_test"), 1.0f);
  EXPECT_FLOAT_EQ(metrics.get_float("r_test"), 0.8f);
}

TEST(CressieReadBatch7, CustomAlphaTau)
{
  VW::estimators::cressieread cr(0.1, 0.99);
  cr.update(1.0f, 0.5f);
  EXPECT_EQ(cr.update_count, 1u);
}

// ============================================================
// confidence_sequence.cc coverage: fill remaining gaps
// update basic path, lower_bound, upper_bound, reset_stats full coverage
// ============================================================

TEST(ConfidenceSequenceBatch7, BasicUpdateAndBounds)
{
  VW::estimators::confidence_sequence cs(0.05, 0.0, 1.0, false);

  for (int i = 0; i < 10; ++i) { cs.update(1.0, 0.5); }

  EXPECT_EQ(cs.update_count, 10u);
  EXPECT_DOUBLE_EQ(cs.last_w, 1.0);
  EXPECT_DOUBLE_EQ(cs.last_r, 0.5);

  float lb = cs.lower_bound();
  float ub = cs.upper_bound();
  EXPECT_LE(lb, ub);
  EXPECT_GE(lb, 0.0f);
  EXPECT_LE(ub, 1.0f);
}

TEST(ConfidenceSequenceBatch7, UpdateWithAdjust)
{
  VW::estimators::confidence_sequence cs(0.05, 0.0, 1.0, true);

  cs.update(1.0, 0.3);
  cs.update(1.0, 0.7);
  cs.update(1.0, 0.5);

  EXPECT_DOUBLE_EQ(cs.rmin, 0.0);
  EXPECT_DOUBLE_EQ(cs.rmax, 1.0);

  float lb = cs.lower_bound();
  float ub = cs.upper_bound();
  EXPECT_LE(lb, ub);
}

TEST(ConfidenceSequenceBatch7, UpdateWithClamp)
{
  VW::estimators::confidence_sequence cs(0.05, 0.0, 1.0, false);

  cs.update(1.0, -0.5);
  cs.update(1.0, 1.5);
  cs.update(1.0, 0.5);

  EXPECT_EQ(cs.update_count, 3u);
}

TEST(ConfidenceSequenceBatch7, BoundsWithZeroUpdates)
{
  VW::estimators::confidence_sequence cs(0.05, 0.0, 1.0, false);

  EXPECT_FLOAT_EQ(cs.lower_bound(), 0.0f);
  EXPECT_FLOAT_EQ(cs.upper_bound(), 1.0f);
}

TEST(ConfidenceSequenceBatch7, BoundsWithEqualRminRmax)
{
  VW::estimators::confidence_sequence cs(0.05, 0.5, 0.5, false);
  cs.update(1.0, 0.5);

  EXPECT_FLOAT_EQ(cs.lower_bound(), 0.5f);
  EXPECT_FLOAT_EQ(cs.upper_bound(), 0.5f);
}

TEST(ConfidenceSequenceBatch7, UpdateWithDropProbability)
{
  VW::estimators::confidence_sequence cs(0.05, 0.0, 1.0, false);

  cs.update(1.0, 0.5, 0.0);
  cs.update(1.0, 0.4, 0.2);
  cs.update(1.0, 0.6, 0.1, 2.0);

  EXPECT_EQ(cs.update_count, 3u);
  float lb = cs.lower_bound();
  float ub = cs.upper_bound();
  EXPECT_LE(lb, ub);
}

TEST(ConfidenceSequenceBatch7, ResetAndReuse)
{
  VW::estimators::confidence_sequence cs(0.05, 0.0, 1.0, true);
  for (int i = 0; i < 5; ++i) { cs.update(1.0, 0.5); }
  EXPECT_EQ(cs.update_count, 5u);

  cs.reset_stats();
  EXPECT_EQ(cs.update_count, 0u);
  EXPECT_EQ(cs.t, 0);
  EXPECT_DOUBLE_EQ(cs.last_w, 0.0);
  EXPECT_DOUBLE_EQ(cs.last_r, 0.0);
  EXPECT_FLOAT_EQ(cs.lower_bound(), 0.0f);
  EXPECT_FLOAT_EQ(cs.upper_bound(), 1.0f);
}

TEST(ConfidenceSequenceBatch7, PersistMetrics)
{
  VW::estimators::confidence_sequence cs(0.05, 0.0, 1.0, false);
  cs.update(1.0, 0.5);

  VW::metric_sink metrics;
  cs.persist(metrics, "_cs");

  EXPECT_EQ(metrics.get_uint("upcnt_cs"), 1u);
  EXPECT_GT(metrics.get_float("ub_cs"), 0.0f);
  EXPECT_FLOAT_EQ(metrics.get_float("last_w_cs"), 1.0f);
  EXPECT_FLOAT_EQ(metrics.get_float("last_r_cs"), 0.5f);
}

// ============================================================
// mf.cc coverage: matrix factorization reduction (78 uncovered lines)
// ============================================================

TEST(ReductionsBatch7, MfBasicTraining)
{
  auto vw = VW::initialize(vwtest::make_args("--new_mf", "5", "--quiet", "--learning_rate", "0.1"));

  for (int i = 0; i < 10; ++i)
  {
    float label = static_cast<float>(i % 3);
    auto* ex = VW::read_example(*vw,
        std::to_string(label) + " |user a:" + std::to_string(i) + " b |item x:" + std::to_string(i % 3) + " y");
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }

  auto* ex = VW::read_example(*vw, "|user a:3 b |item x:1 y");
  vw->predict(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  VW::finish_example(*vw, *ex);
}

TEST(ReductionsBatch7, MfSaveLoadRoundTrip)
{
  auto vw = VW::initialize(vwtest::make_args("--new_mf", "3", "--quiet"));

  for (int i = 0; i < 5; ++i)
  {
    auto* ex = VW::read_example(*vw, "1.0 |user a b |item x y");
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }

  auto model_buffer = std::make_shared<std::vector<char>>();
  VW::io_buf io_writer;
  io_writer.add_file(VW::io::create_vector_writer(model_buffer));
  VW::save_predictor(*vw, io_writer);
  io_writer.flush();

  auto vw2 = VW::initialize(vwtest::make_args("--quiet"),
      VW::io::create_buffer_view(model_buffer->data(), model_buffer->size()));

  auto* ex = VW::read_example(*vw2, "|user a b |item x y");
  vw2->predict(*ex);
  EXPECT_TRUE(std::isfinite(ex->pred.scalar));
  VW::finish_example(*vw2, *ex);
}

// ============================================================
// no_label.cc: exercise parse_no_label with too many tokens
// ============================================================

TEST(NoLabelBatch7, ParseNoLabelWithExtraTokens)
{
  VW::label_parser_reuse_mem reuse;
  VW::polylabel label;
  VW::reduction_features red_features;
  std::vector<VW::string_view> words = {"extra_token"};
  auto logger = VW::io::create_default_logger();
  VW::no_label_parser_global.parse_label(label, red_features, reuse, nullptr, words, logger);
}

TEST(NoLabelBatch7, DefaultLabelAndTestLabel)
{
  VW::polylabel label;
  VW::no_label_parser_global.default_label(label);

  EXPECT_FALSE(VW::no_label_parser_global.test_label(label));
  VW::reduction_features red_features;
  EXPECT_FLOAT_EQ(VW::no_label_parser_global.get_weight(label, red_features), 1.0f);
}
