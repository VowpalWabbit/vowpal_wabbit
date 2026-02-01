// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

// Targeted tests for remaining coverage gaps to push line coverage to 90%.

#include "vw/core/action_score.h"
#include "vw/core/ccb_label.h"
#include "vw/core/debug_print.h"
#include "vw/core/estimators/distributionally_robust.h"
#include "vw/core/example.h"
#include "vw/core/io_buf.h"
#include "vw/core/model_utils.h"
#include "vw/core/prob_dist_cont.h"
#include "vw/core/vw.h"
#include "vw/io/io_adapter.h"
#include "vw/test_common/test_common.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <sstream>

// --- action_score operator<< ---

TEST(CoverageGap, ActionScoreStreamOperator)
{
  VW::action_score as;
  as.action = 3;
  as.score = 0.75f;

  std::ostringstream oss;
  oss << as;
  EXPECT_EQ(oss.str(), "(3,0.75)");
}

// --- debug_print prob_dist_pred_to_string ---

TEST(CoverageGap, ProbDistPredToString)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* ex = VW::new_unused_example(*vw);

  // Populate pdf prediction
  ex->pred.pdf.push_back(VW::continuous_actions::pdf_segment(0.0f, 1.0f, 0.5f));

  std::string result = VW::debug::prob_dist_pred_to_string(*ex);
  EXPECT_THAT(result, testing::HasSubstr("ec.pred.prob_dist["));
  EXPECT_THAT(result, testing::HasSubstr("0"));

  VW::finish_example(*vw, *ex);
}

// --- ccb_label: is_labeled, copy ctor with outcome, assignment with outcome ---

TEST(CoverageGap, CcbLabelIsLabeled)
{
  VW::ccb_label label;
  EXPECT_TRUE(label.is_test_label());
  EXPECT_FALSE(label.is_labeled());

  // Set an outcome
  label.outcome = new VW::ccb_outcome();
  label.outcome->cost = 1.5f;
  EXPECT_FALSE(label.is_test_label());
  EXPECT_TRUE(label.is_labeled());
}

TEST(CoverageGap, CcbLabelCopyConstructorWithOutcome)
{
  VW::ccb_label original;
  original.type = VW::ccb_example_type::SLOT;
  original.outcome = new VW::ccb_outcome();
  original.outcome->cost = 2.5f;
  original.outcome->probabilities.push_back({1, 0.8f});
  original.weight = 3.0f;

  // Copy constructor
  VW::ccb_label copied(original);
  EXPECT_EQ(copied.type, VW::ccb_example_type::SLOT);
  EXPECT_NE(copied.outcome, nullptr);
  EXPECT_NE(copied.outcome, original.outcome);  // deep copy
  EXPECT_FLOAT_EQ(copied.outcome->cost, 2.5f);
  EXPECT_EQ(copied.outcome->probabilities.size(), 1u);
  EXPECT_FLOAT_EQ(copied.weight, 3.0f);
}

TEST(CoverageGap, CcbLabelAssignmentWithOutcome)
{
  VW::ccb_label src;
  src.type = VW::ccb_example_type::SLOT;
  src.outcome = new VW::ccb_outcome();
  src.outcome->cost = 1.0f;
  src.weight = 2.0f;

  // First, create a label with an existing outcome (to test delete path)
  VW::ccb_label dest;
  dest.outcome = new VW::ccb_outcome();
  dest.outcome->cost = 99.0f;

  // Assignment should delete old outcome and deep-copy new one
  dest = src;
  EXPECT_EQ(dest.type, VW::ccb_example_type::SLOT);
  EXPECT_NE(dest.outcome, nullptr);
  EXPECT_NE(dest.outcome, src.outcome);
  EXPECT_FLOAT_EQ(dest.outcome->cost, 1.0f);
  EXPECT_FLOAT_EQ(dest.weight, 2.0f);
}

TEST(CoverageGap, CcbLabelResetToDefaultWithOutcome)
{
  VW::ccb_label label;
  label.outcome = new VW::ccb_outcome();
  label.outcome->cost = 5.0f;
  label.type = VW::ccb_example_type::SLOT;

  label.reset_to_default();
  EXPECT_EQ(label.outcome, nullptr);
  EXPECT_EQ(label.type, VW::ccb_example_type::UNSET);
}

// --- distributionally_robust Duals serialization ---

TEST(CoverageGap, DualsModelSerializationRoundTrip)
{
  VW::details::Duals original(false, 1.5, 2.5, 3.5, 100.0);

  // Serialize
  VW::io_buf write_buf;
  auto backing = std::make_shared<std::vector<char>>();
  write_buf.add_file(VW::io::create_vector_writer(backing));
  size_t written = VW::model_utils::write_model_field(write_buf, original, "duals_test", false);
  write_buf.flush();
  EXPECT_GT(written, 0u);

  // Deserialize
  VW::io_buf read_buf;
  read_buf.add_file(VW::io::create_buffer_view(backing->data(), backing->size()));
  VW::details::Duals restored;
  size_t read_bytes = VW::model_utils::read_model_field(read_buf, restored);
  EXPECT_GT(read_bytes, 0u);

  EXPECT_EQ(restored.unbounded, false);
  EXPECT_DOUBLE_EQ(restored.kappa, 1.5);
  EXPECT_DOUBLE_EQ(restored.gamma, 2.5);
  EXPECT_DOUBLE_EQ(restored.beta, 3.5);
  EXPECT_DOUBLE_EQ(restored.n, 100.0);
}

TEST(CoverageGap, ChiSquaredModelSerializationRoundTrip)
{
  VW::estimators::chi_squared original(0.05, 0.999, 0.0, 10.0, 0.0, 1.0);
  original.update(1.0, 0.5);
  original.update(2.0, 0.3);
  original.update(0.5, 0.8);

  // Serialize
  VW::io_buf write_buf;
  auto backing = std::make_shared<std::vector<char>>();
  write_buf.add_file(VW::io::create_vector_writer(backing));
  size_t written = VW::model_utils::write_model_field(write_buf, original, "chisq_test", false);
  write_buf.flush();
  EXPECT_GT(written, 0u);

  // Deserialize
  VW::io_buf read_buf;
  read_buf.add_file(VW::io::create_buffer_view(backing->data(), backing->size()));
  VW::estimators::chi_squared restored(0.05, 0.999);
  size_t read_bytes = VW::model_utils::read_model_field(read_buf, restored);
  EXPECT_GT(read_bytes, 0u);

  EXPECT_DOUBLE_EQ(restored._alpha, original._alpha);
  EXPECT_DOUBLE_EQ(restored._tau, original._tau);
  EXPECT_DOUBLE_EQ(restored._n, original._n);
  EXPECT_DOUBLE_EQ(restored._sumw, original._sumw);
  EXPECT_DOUBLE_EQ(restored._sumwsq, original._sumwsq);
  EXPECT_DOUBLE_EQ(restored._rmin, original._rmin);
  EXPECT_DOUBLE_EQ(restored._rmax, original._rmax);
}

// --- ccb_label cache round-trip via label_parser ---

TEST(CoverageGap, CcbLabelCacheRoundTrip)
{
  auto lp = VW::ccb_label_parser_global;

  VW::polylabel original;
  lp.default_label(original);
  original.conditional_contextual_bandit.type = VW::ccb_example_type::SLOT;
  original.conditional_contextual_bandit.outcome = new VW::ccb_outcome();
  original.conditional_contextual_bandit.outcome->cost = 1.5f;
  original.conditional_contextual_bandit.outcome->probabilities.push_back({0, 0.9f});
  original.conditional_contextual_bandit.weight = 2.0f;

  // Cache
  VW::io_buf write_buf;
  auto backing = std::make_shared<std::vector<char>>();
  write_buf.add_file(VW::io::create_vector_writer(backing));
  VW::reduction_features red_fts;
  lp.cache_label(original, red_fts, write_buf, "test", false);
  write_buf.flush();

  // Read back
  VW::io_buf read_buf;
  read_buf.add_file(VW::io::create_buffer_view(backing->data(), backing->size()));
  VW::polylabel restored;
  lp.default_label(restored);
  lp.read_cached_label(restored, red_fts, read_buf);

  EXPECT_EQ(restored.conditional_contextual_bandit.type, VW::ccb_example_type::SLOT);
  EXPECT_NE(restored.conditional_contextual_bandit.outcome, nullptr);
  EXPECT_FLOAT_EQ(restored.conditional_contextual_bandit.outcome->cost, 1.5f);
  EXPECT_FLOAT_EQ(restored.conditional_contextual_bandit.weight, 2.0f);
}
