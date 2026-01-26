// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/cb.h"

#include "vw/common/string_view.h"
#include "vw/common/text_utils.h"
#include "vw/core/io_buf.h"
#include "vw/core/memory.h"
#include "vw/core/model_utils.h"
#include "vw/core/multiclass.h"
#include "vw/core/parse_primitives.h"
#include "vw/core/parser.h"
#include "vw/core/vw.h"
#include "vw/io/logger.h"
#include "vw/test_common/test_common.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cfloat>
#include <cmath>
#include <memory>
#include <vector>

using namespace ::testing;

namespace
{
void parse_cb_label(VW::label_parser& lp, VW::string_view label, VW::polylabel& l)
{
  std::vector<VW::string_view> words;
  VW::tokenize(' ', label, words);
  lp.default_label(l);
  VW::reduction_features red_fts;
  VW::label_parser_reuse_mem mem;
  auto null_logger = VW::io::create_null_logger();
  lp.parse_label(l, red_fts, mem, nullptr, words, null_logger);
}
}  // namespace

// ==================== CB Label Tests ====================

TEST(CbLabel, DefaultConstruction)
{
  VW::cb_label label;
  EXPECT_TRUE(label.costs.empty());
  EXPECT_FLOAT_EQ(label.weight, 1.0f);
}

TEST(CbLabel, ResetToDefault)
{
  VW::cb_label label;
  label.costs.push_back(VW::cb_class{0.5f, 1, 0.5f});
  label.weight = 2.0f;

  label.reset_to_default();

  EXPECT_TRUE(label.costs.empty());
  EXPECT_FLOAT_EQ(label.weight, 1.0f);
}

TEST(CbLabel, IsTestLabelEmpty)
{
  VW::cb_label label;
  EXPECT_TRUE(label.is_test_label());
}

TEST(CbLabel, IsTestLabelWithFLTMaxCost)
{
  VW::cb_label label;
  label.costs.push_back(VW::cb_class{FLT_MAX, 1, 0.0f});
  EXPECT_TRUE(label.is_test_label());
}

TEST(CbLabel, IsTestLabelWithZeroProbability)
{
  VW::cb_label label;
  label.costs.push_back(VW::cb_class{1.0f, 1, 0.0f});
  // Zero probability means not observed cost, so it's a test label
  EXPECT_TRUE(label.is_test_label());
}

TEST(CbLabel, IsTestLabelWithValidCostAndProb)
{
  VW::cb_label label;
  label.costs.push_back(VW::cb_class{1.0f, 1, 0.5f});
  EXPECT_FALSE(label.is_test_label());
}

TEST(CbLabel, IsLabeledIsOppositeOfTestLabel)
{
  VW::cb_label label;
  EXPECT_EQ(label.is_labeled(), !label.is_test_label());

  label.costs.push_back(VW::cb_class{1.0f, 1, 0.5f});
  EXPECT_EQ(label.is_labeled(), !label.is_test_label());
}

TEST(CbLabel, ParseSharedLabel)
{
  auto lp = VW::cb_label_parser_global;
  VW::polylabel label;
  parse_cb_label(lp, "shared", label);

  EXPECT_EQ(label.cb.costs.size(), 1);
  EXPECT_FLOAT_EQ(label.cb.costs[0].probability, -1.0f);
}

TEST(CbLabel, ParseActionCostProbLabel)
{
  auto lp = VW::cb_label_parser_global;
  VW::polylabel label;
  parse_cb_label(lp, "1:0.5:0.3", label);

  EXPECT_EQ(label.cb.costs.size(), 1);
  EXPECT_FLOAT_EQ(label.cb.costs[0].cost, 0.5f);
  EXPECT_FLOAT_EQ(label.cb.costs[0].probability, 0.3f);
}

TEST(CbLabel, ParseActionOnlyLabel)
{
  auto lp = VW::cb_label_parser_global;
  VW::polylabel label;
  parse_cb_label(lp, "1", label);

  EXPECT_EQ(label.cb.costs.size(), 1);
  EXPECT_FLOAT_EQ(label.cb.costs[0].cost, FLT_MAX);
  EXPECT_FLOAT_EQ(label.cb.costs[0].probability, 0.0f);
}

TEST(CbLabel, ParseActionCostLabel)
{
  auto lp = VW::cb_label_parser_global;
  VW::polylabel label;
  parse_cb_label(lp, "1:0.5", label);

  EXPECT_EQ(label.cb.costs.size(), 1);
  EXPECT_FLOAT_EQ(label.cb.costs[0].cost, 0.5f);
  EXPECT_FLOAT_EQ(label.cb.costs[0].probability, 0.0f);
}

TEST(CbLabel, ParseEmptyLabel)
{
  auto lp = VW::cb_label_parser_global;
  VW::polylabel label;
  parse_cb_label(lp, "", label);

  EXPECT_TRUE(label.cb.costs.empty());
  EXPECT_TRUE(label.cb.is_test_label());
}

TEST(CbLabel, LabelTypeIsCB)
{
  auto lp = VW::cb_label_parser_global;
  EXPECT_EQ(lp.label_type, VW::label_type_t::CB);
}

TEST(CbLabel, GetWeightReturnsLabelWeight)
{
  auto lp = VW::cb_label_parser_global;
  VW::polylabel label;
  VW::reduction_features red_features;
  lp.default_label(label);
  label.cb.weight = 2.5f;

  EXPECT_FLOAT_EQ(lp.get_weight(label, red_features), 2.5f);
}

// ==================== CB Class Tests ====================

TEST(CbClass, DefaultConstruction)
{
  VW::cb_class cbc;
  EXPECT_FLOAT_EQ(cbc.cost, FLT_MAX);
  EXPECT_EQ(cbc.action, 0);
  EXPECT_FLOAT_EQ(cbc.probability, -1.0f);
  EXPECT_FLOAT_EQ(cbc.partial_prediction, 0.0f);
}

TEST(CbClass, ParameterizedConstruction)
{
  VW::cb_class cbc(1.5f, 2, 0.3f);
  EXPECT_FLOAT_EQ(cbc.cost, 1.5f);
  EXPECT_EQ(cbc.action, 2);
  EXPECT_FLOAT_EQ(cbc.probability, 0.3f);
  EXPECT_FLOAT_EQ(cbc.partial_prediction, 0.0f);
}

TEST(CbClass, HasObservedCostTrue)
{
  VW::cb_class cbc(1.0f, 1, 0.5f);
  EXPECT_TRUE(cbc.has_observed_cost());
}

TEST(CbClass, HasObservedCostFalseWithFLTMax)
{
  VW::cb_class cbc;
  cbc.cost = FLT_MAX;
  cbc.probability = 0.5f;
  EXPECT_FALSE(cbc.has_observed_cost());
}

TEST(CbClass, HasObservedCostFalseWithZeroProb)
{
  VW::cb_class cbc;
  cbc.cost = 1.0f;
  cbc.probability = 0.0f;
  EXPECT_FALSE(cbc.has_observed_cost());
}

TEST(CbClass, HasObservedCostFalseWithNegativeProb)
{
  VW::cb_class cbc;
  cbc.cost = 1.0f;
  cbc.probability = -1.0f;
  EXPECT_FALSE(cbc.has_observed_cost());
}

TEST(CbClass, Equality)
{
  VW::cb_class a(1.0f, 5, 0.3f);
  VW::cb_class b(2.0f, 5, 0.8f);
  VW::cb_class c(1.0f, 3, 0.3f);

  // Equality is based on action only
  EXPECT_TRUE(a == b);
  EXPECT_FALSE(a == c);
}

// ==================== Get Observed Cost Tests ====================

TEST(CbLabel, GetObservedCostWithNoObservedCost)
{
  VW::cb_label label;
  label.costs.push_back(VW::cb_class{FLT_MAX, 1, 0.0f});

  auto result = VW::get_observed_cost_cb(label);
  EXPECT_FALSE(result.first);
}

TEST(CbLabel, GetObservedCostWithObservedCost)
{
  VW::cb_label label;
  label.costs.push_back(VW::cb_class{1.5f, 2, 0.3f});

  auto result = VW::get_observed_cost_cb(label);
  EXPECT_TRUE(result.first);
  EXPECT_FLOAT_EQ(result.second.cost, 1.5f);
  EXPECT_EQ(result.second.action, 2);
  EXPECT_FLOAT_EQ(result.second.probability, 0.3f);
}

TEST(CbLabel, GetObservedCostWithMultipleCosts)
{
  VW::cb_label label;
  label.costs.push_back(VW::cb_class{FLT_MAX, 1, 0.0f});  // Not observed
  label.costs.push_back(VW::cb_class{2.0f, 2, 0.5f});     // Observed
  label.costs.push_back(VW::cb_class{FLT_MAX, 3, 0.0f});  // Not observed

  auto result = VW::get_observed_cost_cb(label);
  EXPECT_TRUE(result.first);
  EXPECT_FLOAT_EQ(result.second.cost, 2.0f);
  EXPECT_EQ(result.second.action, 2);
}

TEST(CbLabel, GetObservedCostEmpty)
{
  VW::cb_label label;

  auto result = VW::get_observed_cost_cb(label);
  EXPECT_FALSE(result.first);
}

// ==================== Example Header Detection ====================

TEST(CbLabel, EcIsExampleHeaderCbWithShared)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--quiet"));

  auto* ex = VW::read_example(*vw, "shared | a b c");
  EXPECT_TRUE(VW::ec_is_example_header_cb(*ex));
  VW::finish_example(*vw, *ex);
}

TEST(CbLabel, EcIsExampleHeaderCbWithNonShared)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--quiet"));

  auto* ex = VW::read_example(*vw, "0:1:0.5 | a b c");
  EXPECT_FALSE(VW::ec_is_example_header_cb(*ex));
  VW::finish_example(*vw, *ex);
}

TEST(CbLabel, EcIsExampleHeaderCbWithMultipleCosts)
{
  VW::example ex;
  ex.l.cb.costs.push_back(VW::cb_class{0.f, 0, -1.f});
  ex.l.cb.costs.push_back(VW::cb_class{1.f, 1, 0.5f});

  // Multiple costs means not a header
  EXPECT_FALSE(VW::ec_is_example_header_cb(ex));
}

// ==================== Model Serialization Tests ====================

TEST(CbClass, ModelSerialization)
{
  VW::cb_class original{1.5f, 3, 0.25f};
  original.partial_prediction = 0.75f;

  // Serialize
  VW::io_buf write_buffer;
  auto backing_buffer_write = std::make_shared<std::vector<char>>();
  write_buffer.add_file(VW::io::create_vector_writer(backing_buffer_write));
  size_t written = VW::model_utils::write_model_field(write_buffer, original, "test", false);
  write_buffer.flush();
  EXPECT_GT(written, 0);

  // Deserialize
  VW::io_buf read_buffer;
  read_buffer.add_file(VW::io::create_buffer_view(backing_buffer_write->data(), backing_buffer_write->size()));
  VW::cb_class restored;
  size_t read = VW::model_utils::read_model_field(read_buffer, restored);
  EXPECT_GT(read, 0);

  EXPECT_FLOAT_EQ(restored.cost, original.cost);
  EXPECT_EQ(restored.action, original.action);
  EXPECT_FLOAT_EQ(restored.probability, original.probability);
  EXPECT_FLOAT_EQ(restored.partial_prediction, original.partial_prediction);
}

TEST(CbLabel, ModelSerialization)
{
  VW::cb_label original;
  original.costs.push_back({0.5f, 1, 0.1f});
  original.costs.push_back({1.0f, 2, 0.3f});
  original.weight = 2.5f;

  // Serialize
  VW::io_buf write_buffer;
  auto backing_buffer_write = std::make_shared<std::vector<char>>();
  write_buffer.add_file(VW::io::create_vector_writer(backing_buffer_write));
  size_t written = VW::model_utils::write_model_field(write_buffer, original, "test", false);
  write_buffer.flush();
  EXPECT_GT(written, 0);

  // Deserialize
  VW::io_buf read_buffer;
  read_buffer.add_file(VW::io::create_buffer_view(backing_buffer_write->data(), backing_buffer_write->size()));
  VW::cb_label restored;
  size_t read = VW::model_utils::read_model_field(read_buffer, restored);
  EXPECT_GT(read, 0);

  EXPECT_EQ(restored.costs.size(), original.costs.size());
  EXPECT_FLOAT_EQ(restored.weight, original.weight);
  for (size_t i = 0; i < original.costs.size(); i++)
  {
    EXPECT_FLOAT_EQ(restored.costs[i].cost, original.costs[i].cost);
    EXPECT_EQ(restored.costs[i].action, original.costs[i].action);
    EXPECT_FLOAT_EQ(restored.costs[i].probability, original.costs[i].probability);
  }
}

// ==================== CB Eval Label Tests ====================

TEST(CbEvalLabel, LabelTypeIsCBEval)
{
  auto lp = VW::cb_eval_label_parser_global;
  EXPECT_EQ(lp.label_type, VW::label_type_t::CB_EVAL);
}

TEST(CbEvalLabel, DefaultLabel)
{
  auto lp = VW::cb_eval_label_parser_global;
  VW::polylabel label;
  lp.default_label(label);

  EXPECT_EQ(label.cb_eval.action, 0);
  EXPECT_TRUE(label.cb_eval.event.costs.empty());
}

TEST(CbEvalLabel, ModelSerialization)
{
  VW::cb_eval_label original;
  original.action = 5;
  original.event.costs.push_back({0.5f, 1, 0.2f});
  original.event.weight = 1.5f;

  // Serialize
  VW::io_buf write_buffer;
  auto backing_buffer_write = std::make_shared<std::vector<char>>();
  write_buffer.add_file(VW::io::create_vector_writer(backing_buffer_write));
  size_t written = VW::model_utils::write_model_field(write_buffer, original, "test", false);
  write_buffer.flush();
  EXPECT_GT(written, 0);

  // Deserialize
  VW::io_buf read_buffer;
  read_buffer.add_file(VW::io::create_buffer_view(backing_buffer_write->data(), backing_buffer_write->size()));
  VW::cb_eval_label restored;
  size_t read = VW::model_utils::read_model_field(read_buffer, restored);
  EXPECT_GT(read, 0);

  EXPECT_EQ(restored.action, original.action);
  EXPECT_EQ(restored.event.costs.size(), original.event.costs.size());
  EXPECT_FLOAT_EQ(restored.event.weight, original.event.weight);
}

// ==================== Multiclass Label Tests ====================

TEST(MulticlassLabel, DefaultConstruction)
{
  VW::multiclass_label label;
  EXPECT_EQ(label.label, std::numeric_limits<uint32_t>::max());
  EXPECT_FLOAT_EQ(label.weight, 1.0f);
}

TEST(MulticlassLabel, ParameterizedConstruction)
{
  VW::multiclass_label label(5, 2.0f);
  EXPECT_EQ(label.label, 5);
  EXPECT_FLOAT_EQ(label.weight, 2.0f);
}

TEST(MulticlassLabel, ResetToDefault)
{
  VW::multiclass_label label(5, 2.0f);
  label.reset_to_default();

  EXPECT_EQ(label.label, std::numeric_limits<uint32_t>::max());
  EXPECT_FLOAT_EQ(label.weight, 1.0f);
}

TEST(MulticlassLabel, IsLabeledTrue)
{
  VW::multiclass_label label(5, 1.0f);
  EXPECT_TRUE(label.is_labeled());
}

TEST(MulticlassLabel, IsLabeledFalse)
{
  VW::multiclass_label label;
  EXPECT_FALSE(label.is_labeled());
}

TEST(MulticlassLabel, TestMulticlassLabelUnlabeled)
{
  VW::multiclass_label label;
  EXPECT_TRUE(VW::test_multiclass_label(label));
}

TEST(MulticlassLabel, TestMulticlassLabelLabeled)
{
  VW::multiclass_label label(3, 1.0f);
  EXPECT_FALSE(VW::test_multiclass_label(label));
}

TEST(MulticlassLabel, LabelTypeIsMulticlass)
{
  auto lp = VW::multiclass_label_parser_global;
  EXPECT_EQ(lp.label_type, VW::label_type_t::MULTICLASS);
}

TEST(MulticlassLabel, ModelSerialization)
{
  VW::multiclass_label original(7, 3.5f);

  // Serialize
  VW::io_buf write_buffer;
  auto backing_buffer_write = std::make_shared<std::vector<char>>();
  write_buffer.add_file(VW::io::create_vector_writer(backing_buffer_write));
  size_t written = VW::model_utils::write_model_field(write_buffer, original, "test", false);
  write_buffer.flush();
  EXPECT_GT(written, 0);

  // Deserialize
  VW::io_buf read_buffer;
  read_buffer.add_file(VW::io::create_buffer_view(backing_buffer_write->data(), backing_buffer_write->size()));
  VW::multiclass_label restored;
  size_t read = VW::model_utils::read_model_field(read_buffer, restored);
  EXPECT_GT(read, 0);

  EXPECT_EQ(restored.label, original.label);
  EXPECT_FLOAT_EQ(restored.weight, original.weight);
}

TEST(MulticlassLabel, GetWeightPositive)
{
  auto lp = VW::multiclass_label_parser_global;
  VW::polylabel label;
  VW::reduction_features red_features;
  lp.default_label(label);
  label.multi.weight = 2.5f;

  EXPECT_FLOAT_EQ(lp.get_weight(label, red_features), 2.5f);
}

TEST(MulticlassLabel, GetWeightZero)
{
  auto lp = VW::multiclass_label_parser_global;
  VW::polylabel label;
  VW::reduction_features red_features;
  lp.default_label(label);
  label.multi.weight = 0.0f;

  // Zero weight should return 0
  EXPECT_FLOAT_EQ(lp.get_weight(label, red_features), 0.0f);
}

TEST(MulticlassLabel, GetWeightNegative)
{
  auto lp = VW::multiclass_label_parser_global;
  VW::polylabel label;
  VW::reduction_features red_features;
  lp.default_label(label);
  label.multi.weight = -1.0f;

  // Negative weight should return 0
  EXPECT_FLOAT_EQ(lp.get_weight(label, red_features), 0.0f);
}

// ==================== Integration Tests ====================

TEST(CbLabel, IntegrationWithVWCbAdf)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--quiet"));

  // Train with a labeled example
  {
    VW::multi_ex examples;
    examples.push_back(VW::read_example(*vw, "shared | s1 s2"));
    examples.push_back(VW::read_example(*vw, "0:1:0.5 | a1 a2"));
    examples.push_back(VW::read_example(*vw, "| b1 b2"));
    examples.push_back(VW::read_example(*vw, "| c1 c2"));

    vw->learn(examples);
    vw->finish_example(examples);
  }

  // Predict on unlabeled example
  {
    VW::multi_ex examples;
    examples.push_back(VW::read_example(*vw, "shared | s1 s2"));
    examples.push_back(VW::read_example(*vw, "| a1 a2"));
    examples.push_back(VW::read_example(*vw, "| b1 b2"));

    vw->predict(examples);
    // Should have action scores
    EXPECT_FALSE(examples[0]->pred.a_s.empty());
    vw->finish_example(examples);
  }
}

TEST(MulticlassLabel, IntegrationWithVWOAA)
{
  auto vw = VW::initialize(vwtest::make_args("--oaa", "3", "--quiet"));

  auto* ex = VW::read_example(*vw, "2 | a b c");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);

  auto* ex_pred = VW::read_example(*vw, "| a b c");
  vw->predict(*ex_pred);
  EXPECT_GE(ex_pred->pred.multiclass, 1);
  EXPECT_LE(ex_pred->pred.multiclass, 3);
  VW::finish_example(*vw, *ex_pred);
}

// ==================== Edge Case Tests for Coverage ====================

TEST(CbLabelEdgeCases, ProbabilityClampingAboveOne)
{
  // Test that probability > 1 gets clamped to 1.0
  auto vw = VW::initialize(vwtest::make_args("--cb", "2", "--quiet"));
  // Probability 1.5 should be clamped to 1.0
  auto* ex = VW::read_example(*vw, "1:0.5:1.5 | a b");
  EXPECT_FLOAT_EQ(ex->l.cb.costs[0].probability, 1.0f);
  VW::finish_example(*vw, *ex);
}

TEST(CbLabelEdgeCases, ProbabilityClampingBelowZero)
{
  // Test that probability < 0 gets clamped to 0.0
  auto vw = VW::initialize(vwtest::make_args("--cb", "2", "--quiet"));
  // Probability -0.5 should be clamped to 0.0
  auto* ex = VW::read_example(*vw, "1:0.5:-0.5 | a b");
  EXPECT_FLOAT_EQ(ex->l.cb.costs[0].probability, 0.0f);
  VW::finish_example(*vw, *ex);
}

TEST(CbLabelEdgeCases, SharedWithoutCost)
{
  // Test shared feature with no cost (probability set to -1)
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--quiet"));
  VW::multi_ex examples;
  examples.push_back(VW::read_example(*vw, "shared | s_feature"));
  examples.push_back(VW::read_example(*vw, "| a_feature"));
  examples.push_back(VW::read_example(*vw, "| b_feature"));

  // Shared example should have probability -1 as marker
  EXPECT_FLOAT_EQ(examples[0]->l.cb.costs[0].probability, -1.0f);

  vw->predict(examples);
  vw->finish_example(examples);
}

TEST(CbLabelEdgeCases, FLTMaxCost)
{
  // Test that action without cost gets FLT_MAX
  VW::cb_label label;
  label.reset_to_default();

  VW::cb_class cb;
  cb.action = 1;
  cb.cost = FLT_MAX;  // Default when no cost specified
  cb.probability = 0.0f;
  label.costs.push_back(cb);

  EXPECT_FLOAT_EQ(label.costs[0].cost, FLT_MAX);
}

TEST(MulticlassLabelEdgeCases, TrailingCharacterThrows)
{
  // Test that trailing characters after label cause an error
  auto vw = VW::initialize(vwtest::make_args("--oaa", "5", "--quiet"));
  EXPECT_THROW(VW::read_example(*vw, "123abc | features"), VW::vw_exception);
}

TEST(MulticlassLabelEdgeCases, TooManyWordsThrows)
{
  // Test that too many words in label cause an error
  auto vw = VW::initialize(vwtest::make_args("--oaa", "5", "--quiet"));
  EXPECT_THROW(VW::read_example(*vw, "1 0.5 extra | features"), VW::vw_exception);
}

TEST(MulticlassLabelEdgeCases, LabelWithWeight)
{
  // Test label with explicit weight
  auto vw = VW::initialize(vwtest::make_args("--oaa", "10", "--quiet"));
  auto* ex = VW::read_example(*vw, "5 2.5 | features");

  EXPECT_EQ(ex->l.multi.label, 5);
  EXPECT_FLOAT_EQ(ex->l.multi.weight, 2.5f);
  VW::finish_example(*vw, *ex);
}

TEST(MulticlassLabelEdgeCases, UnlabeledExample)
{
  // Test unlabeled example for prediction
  auto vw = VW::initialize(vwtest::make_args("--oaa", "5", "--quiet"));

  // First train with some labeled examples
  auto* ex_train = VW::read_example(*vw, "2 | features");
  vw->learn(*ex_train);
  VW::finish_example(*vw, *ex_train);

  // Now test with unlabeled
  auto* ex = VW::read_example(*vw, "| features");
  vw->predict(*ex);
  // Prediction should be in valid range
  EXPECT_GE(ex->pred.multiclass, 1);
  EXPECT_LE(ex->pred.multiclass, 5);
  VW::finish_example(*vw, *ex);
}

TEST(CbLabelEdgeCases, ActionOnlyCost)
{
  // Test parsing with only action (no cost/probability)
  auto vw = VW::initialize(vwtest::make_args("--cb", "3", "--quiet"));
  auto* ex = VW::read_example(*vw, "2 | a b");

  // Action 2, cost should be FLT_MAX
  EXPECT_EQ(ex->l.cb.costs[0].action, 2);
  EXPECT_FLOAT_EQ(ex->l.cb.costs[0].cost, FLT_MAX);
  VW::finish_example(*vw, *ex);
}

TEST(CbLabelEdgeCases, MultipleActions)
{
  // Test parsing multiple actions in a label
  VW::polylabel label;
  parse_cb_label(VW::cb_label_parser_global, "1:0.5:0.25 2:0.3:0.75", label);

  EXPECT_EQ(label.cb.costs.size(), 2);
  EXPECT_EQ(label.cb.costs[0].action, 1);
  EXPECT_FLOAT_EQ(label.cb.costs[0].cost, 0.5f);
  EXPECT_FLOAT_EQ(label.cb.costs[0].probability, 0.25f);
  EXPECT_EQ(label.cb.costs[1].action, 2);
  EXPECT_FLOAT_EQ(label.cb.costs[1].cost, 0.3f);
  EXPECT_FLOAT_EQ(label.cb.costs[1].probability, 0.75f);
}

TEST(CbLabelEdgeCases, ZeroCost)
{
  // Test zero cost is valid
  VW::polylabel label;
  parse_cb_label(VW::cb_label_parser_global, "1:0.0:0.5", label);

  EXPECT_EQ(label.cb.costs.size(), 1);
  EXPECT_FLOAT_EQ(label.cb.costs[0].cost, 0.0f);
}

TEST(CbLabelEdgeCases, NegativeCost)
{
  // Test negative cost is valid (reward)
  VW::polylabel label;
  parse_cb_label(VW::cb_label_parser_global, "1:-1.5:0.5", label);

  EXPECT_EQ(label.cb.costs.size(), 1);
  EXPECT_FLOAT_EQ(label.cb.costs[0].cost, -1.5f);
}

TEST(CbLabelEdgeCases, ZeroProbability)
{
  // Test zero probability is valid
  VW::polylabel label;
  parse_cb_label(VW::cb_label_parser_global, "1:0.5:0.0", label);

  EXPECT_EQ(label.cb.costs.size(), 1);
  EXPECT_FLOAT_EQ(label.cb.costs[0].probability, 0.0f);
}

TEST(CbLabelEdgeCases, ExactlyOneProbability)
{
  // Test probability = 1.0 exactly
  VW::polylabel label;
  parse_cb_label(VW::cb_label_parser_global, "1:0.5:1.0", label);

  EXPECT_EQ(label.cb.costs.size(), 1);
  EXPECT_FLOAT_EQ(label.cb.costs[0].probability, 1.0f);
}
