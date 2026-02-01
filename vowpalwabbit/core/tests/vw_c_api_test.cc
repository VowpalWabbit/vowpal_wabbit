// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

// Tests for C-style API functions in vw.cc that are typically used by
// language bindings (Python, Java, C#) and the VWDLL interface.

#include "vw/core/example.h"
#include "vw/core/global_data.h"
#include "vw/core/simple_label.h"
#include "vw/core/vw.h"
#include "vw/test_common/test_common.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstring>
#include <sstream>

using namespace ::testing;

// --- C API getter tests (vw.cc lines 818-859) ---

TEST(VwCApi, GetLabel)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* ex = VW::read_example(*vw, "0.5 | a b");
  EXPECT_FLOAT_EQ(VW::get_label(ex), 0.5f);
  VW::finish_example(*vw, *ex);
}

TEST(VwCApi, GetPrediction)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);

  auto* ex2 = VW::read_example(*vw, "| a b c");
  vw->predict(*ex2);
  float pred = VW::get_prediction(ex2);
  // After learning on label=1, prediction should be non-zero
  EXPECT_NE(pred, 0.0f);
  VW::finish_example(*vw, *ex2);
}

TEST(VwCApi, GetImportance)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* ex = VW::read_example(*vw, "1 2.5 | a b");
  EXPECT_FLOAT_EQ(VW::get_importance(ex), 2.5f);
  VW::finish_example(*vw, *ex);
}

TEST(VwCApi, GetInitial)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* ex = VW::read_example(*vw, "1 1 0.3 | a b");
  EXPECT_FLOAT_EQ(VW::get_initial(ex), 0.3f);
  VW::finish_example(*vw, *ex);
}

TEST(VwCApi, GetCostSensitivePrediction)
{
  auto vw = VW::initialize(vwtest::make_args("--csoaa", "3", "--quiet"));
  auto* ex = VW::read_example(*vw, "1:0.5 2:1.0 3:0.25 | a b c");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);

  auto* ex2 = VW::read_example(*vw, "| a b c");
  vw->predict(*ex2);
  float cs_pred = VW::get_cost_sensitive_prediction(ex2);
  EXPECT_GE(cs_pred, 1.0f);
  EXPECT_LE(cs_pred, 3.0f);
  VW::finish_example(*vw, *ex2);
}

TEST(VwCApi, GetActionScore)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--quiet"));

  auto* shared = VW::read_example(*vw, "shared | s");
  auto* action1 = VW::read_example(*vw, "0:0.5:0.5 | a");
  auto* action2 = VW::read_example(*vw, "| b");
  auto* empty = VW::read_example(*vw, "");

  VW::multi_ex multi_ex = {shared, action1, action2};
  vw->learn(multi_ex);
  vw->finish_example(multi_ex);
  VW::finish_example(*vw, *empty);

  // Predict
  auto* shared2 = VW::read_example(*vw, "shared | s");
  auto* act1 = VW::read_example(*vw, "| a");
  auto* act2 = VW::read_example(*vw, "| b");
  auto* empty2 = VW::read_example(*vw, "");

  VW::multi_ex multi_ex2 = {shared2, act1, act2};
  vw->predict(multi_ex2);

  size_t as_len = VW::get_action_score_length(shared2);
  EXPECT_GT(as_len, 0u);

  float score = VW::get_action_score(shared2, 0);
  EXPECT_GE(score, 0.0f);
  EXPECT_LE(score, 1.0f);

  // Out of range returns 0.0
  float oob_score = VW::get_action_score(shared2, 999);
  EXPECT_FLOAT_EQ(oob_score, 0.0f);

  vw->finish_example(multi_ex2);
  VW::finish_example(*vw, *empty2);
}

TEST(VwCApi, GetMultilabelPredictions)
{
  auto vw = VW::initialize(vwtest::make_args("--multilabel_oaa", "3", "--quiet"));
  auto* ex = VW::read_example(*vw, "1,2 | a b c");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);

  auto* ex2 = VW::read_example(*vw, "| a b c");
  vw->predict(*ex2);

  size_t len = 0;
  uint32_t* labels = VW::get_multilabel_predictions(ex2, len);
  // Should get at least one prediction
  EXPECT_GT(len, 0u);
  EXPECT_NE(labels, nullptr);
  VW::finish_example(*vw, *ex2);
}

TEST(VwCApi, GetTagAndLength)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* ex = VW::read_example(*vw, "1 my_tag| a b c");

  size_t tag_len = VW::get_tag_length(ex);
  EXPECT_EQ(tag_len, 6u);

  const char* tag = VW::get_tag(ex);
  EXPECT_EQ(std::string(tag, tag_len), "my_tag");

  VW::finish_example(*vw, *ex);
}

TEST(VwCApi, GetTagEmpty)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");

  size_t tag_len = VW::get_tag_length(ex);
  EXPECT_EQ(tag_len, 0u);

  VW::finish_example(*vw, *ex);
}

TEST(VwCApi, GetFeatureNumber)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");

  size_t num_features = VW::get_feature_number(ex);
  // a, b, c + constant = 4 features
  EXPECT_GE(num_features, 3u);

  VW::finish_example(*vw, *ex);
}

TEST(VwCApi, GetConfidence)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");
  // Default confidence is 0
  EXPECT_FLOAT_EQ(VW::get_confidence(ex), 0.0f);
  VW::finish_example(*vw, *ex);
}

// --- add_label, add_constant_feature ---

TEST(VwCApi, AddLabel)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* ex = VW::new_unused_example(*vw);

  VW::add_label(ex, 2.0f, 3.0f, 0.5f);
  EXPECT_FLOAT_EQ(VW::get_label(ex), 2.0f);
  EXPECT_FLOAT_EQ(VW::get_importance(ex), 3.0f);
  EXPECT_FLOAT_EQ(VW::get_initial(ex), 0.5f);

  VW::finish_example(*vw, *ex);
}

TEST(VwCApi, AddConstantFeature)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--noconstant"));
  auto* ex = VW::read_example(*vw, "1 | a b");
  size_t before = VW::get_feature_number(ex);

  VW::add_constant_feature(*vw, ex);
  EXPECT_EQ(ex->num_features, before + 1);

  VW::finish_example(*vw, *ex);
}

// --- parse_example_label ---

TEST(VwCApi, ParseExampleLabel)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* ex = VW::new_unused_example(*vw);

  VW::parse_example_label(*vw, *ex, "3.5");
  EXPECT_FLOAT_EQ(ex->l.simple.label, 3.5f);

  VW::finish_example(*vw, *ex);
}

TEST(VwCApi, ParseExampleLabelWithWeightAndInitial)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* ex = VW::new_unused_example(*vw);

  VW::parse_example_label(*vw, *ex, "3.5 2.0 0.1");
  EXPECT_FLOAT_EQ(ex->l.simple.label, 3.5f);
  // Weight and initial are stored in reduction features, not ec->weight
  auto& red_features = ex->ex_reduction_features.template get<VW::simple_label_reduction_features>();
  EXPECT_FLOAT_EQ(red_features.weight, 2.0f);
  EXPECT_FLOAT_EQ(red_features.initial, 0.1f);

  VW::finish_example(*vw, *ex);
}

// --- get_features / return_features ---

TEST(VwCApi, GetAndReturnFeatures)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* ex = VW::read_example(*vw, "1 | a b c");

  size_t feature_number = 0;
  VW::feature* features = VW::get_features(*vw, ex, feature_number);
  EXPECT_GT(feature_number, 0u);
  EXPECT_NE(features, nullptr);

  // Each feature should have a non-zero weight index
  for (size_t i = 0; i < feature_number; ++i) { EXPECT_NE(features[i].weight_index, 0u); }

  VW::return_features(features);
  VW::finish_example(*vw, *ex);
}

// --- export_example / release_feature_space ---

TEST(VwCApi, ExportAndReleaseExample)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* ex = VW::read_example(*vw, "1 |ns a b c");

  size_t len = 0;
  VW::primitive_feature_space* fs = VW::export_example(*vw, ex, len);
  EXPECT_GT(len, 0u);
  EXPECT_NE(fs, nullptr);

  // Verify we got features exported
  size_t total_features = 0;
  for (size_t i = 0; i < len; ++i) { total_features += fs[i].len; }
  EXPECT_GT(total_features, 0u);

  VW::release_feature_space(fs, len);
  VW::finish_example(*vw, *ex);
}

// --- import_example ---

TEST(VwCApi, ImportExample)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));

  // Create a primitive feature space
  VW::primitive_feature_space pfs;
  pfs.name = 'a';
  pfs.len = 2;
  pfs.fs = new VW::feature[2];
  pfs.fs[0] = VW::feature(1.0f, 100);
  pfs.fs[1] = VW::feature(2.0f, 200);

  auto* ex = VW::import_example(*vw, "1.0", &pfs, 1);
  EXPECT_NE(ex, nullptr);
  EXPECT_FLOAT_EQ(VW::get_label(ex), 1.0f);
  EXPECT_GT(VW::get_feature_number(ex), 0u);

  VW::finish_example(*vw, *ex);
  delete[] pfs.fs;
}

// --- new_unused_example ---

TEST(VwCApi, NewUnusedExample)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* ex = VW::new_unused_example(*vw);
  EXPECT_NE(ex, nullptr);
  VW::finish_example(*vw, *ex);
}

// --- move_feature_namespace ---

TEST(VwCApi, MoveFeatureNamespace)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* src = VW::read_example(*vw, "| a b c");
  auto* dst = VW::new_unused_example(*vw);

  size_t src_features_before = src->num_features;
  EXPECT_GT(src_features_before, 0u);

  // Move the default namespace from src to dst
  VW::namespace_index ns = ' ';
  VW::move_feature_namespace(dst, src, ns);

  // src should have lost the features, dst should have gained them
  EXPECT_GT(dst->num_features, 0u);

  VW::finish_example(*vw, *src);
  VW::finish_example(*vw, *dst);
}

TEST(VwCApi, MoveFeatureNamespaceNonexistent)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* src = VW::read_example(*vw, "|a x y");
  auto* dst = VW::new_unused_example(*vw);

  // Trying to move a namespace that doesn't exist in src should be a no-op
  VW::move_feature_namespace(dst, src, 'z');
  EXPECT_EQ(dst->num_features, 0u);

  VW::finish_example(*vw, *src);
  VW::finish_example(*vw, *dst);
}

// --- copy_example_metadata, copy_example_data, copy_example_data_with_label ---

TEST(VwCApi, CopyExampleMetadata)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* src = VW::read_example(*vw, "1 my_tag| a b c");
  src->example_counter = 42;
  src->confidence = 0.75f;
  src->test_only = true;

  auto* dst = VW::new_unused_example(*vw);

  VW::copy_example_metadata(dst, src);
  EXPECT_EQ(dst->example_counter, 42u);
  EXPECT_FLOAT_EQ(dst->confidence, 0.75f);
  EXPECT_TRUE(dst->test_only);
  EXPECT_EQ(VW::get_tag_length(dst), VW::get_tag_length(src));

  VW::finish_example(*vw, *src);
  VW::finish_example(*vw, *dst);
}

TEST(VwCApi, CopyExampleData)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* src = VW::read_example(*vw, "1 | a b c");

  auto* dst = VW::new_unused_example(*vw);

  VW::copy_example_data(dst, src);
  EXPECT_EQ(dst->num_features, src->num_features);
  EXPECT_EQ(dst->indices.size(), src->indices.size());

  VW::finish_example(*vw, *src);
  VW::finish_example(*vw, *dst);
}

TEST(VwCApi, CopyExampleDataWithLabel)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* src = VW::read_example(*vw, "2.5 | a b c");

  auto* dst = VW::new_unused_example(*vw);

  VW::copy_example_data_with_label(dst, src);
  EXPECT_FLOAT_EQ(dst->l.simple.label, 2.5f);
  EXPECT_EQ(dst->num_features, src->num_features);

  VW::finish_example(*vw, *src);
  VW::finish_example(*vw, *dst);
}

// --- empty_example ---

TEST(VwCApi, EmptyExample)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* ex = VW::read_example(*vw, "1 tag| a b c");
  EXPECT_GT(ex->indices.size(), 0u);
  EXPECT_GT(VW::get_tag_length(ex), 0u);

  VW::empty_example(*vw, *ex);
  EXPECT_EQ(ex->indices.size(), 0u);
  EXPECT_EQ(ex->tag.size(), 0u);
  EXPECT_FALSE(ex->sorted);
  EXPECT_FALSE(ex->end_pass);
  EXPECT_FALSE(ex->is_newline);

  VW::finish_example(*vw, *ex);
}

// --- cmd_string_replace_value ---

VW_WARNING_STATE_PUSH
VW_WARNING_DISABLE_DEPRECATED_USAGE

TEST(VwCApi, CmdStringReplaceValueExistingFlag)
{
  auto* ss = new std::stringstream();
  *ss << "--learning_rate 0.5 --power_t 0.5";

  VW::cmd_string_replace_value(ss, "--learning_rate", "1.0");

  std::string result = ss->str();
  EXPECT_THAT(result, HasSubstr("--learning_rate 1.0"));
  EXPECT_THAT(result, HasSubstr("--power_t 0.5"));
  delete ss;
}

TEST(VwCApi, CmdStringReplaceValueNewFlag)
{
  auto* ss = new std::stringstream();
  *ss << "--learning_rate 0.5";

  VW::cmd_string_replace_value(ss, "--power_t", "0.3");

  std::string result = ss->str();
  EXPECT_THAT(result, HasSubstr("--power_t 0.3"));
  EXPECT_THAT(result, HasSubstr("--learning_rate 0.5"));
  delete ss;
}

TEST(VwCApi, CmdStringReplaceValueEndOfString)
{
  auto* ss = new std::stringstream();
  *ss << "--passes 3 --learning_rate 0.5";

  // Replace the last flag (value at end of string)
  VW::cmd_string_replace_value(ss, "--learning_rate", "2.0");

  std::string result = ss->str();
  EXPECT_THAT(result, HasSubstr("--learning_rate 2.0"));
  delete ss;
}

VW_WARNING_STATE_POP

// --- seed_vw_model ---

TEST(VwCApi, SeedVwModel)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));

  // Train a bit
  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);

  // Seed a new model from the trained one
  auto seeded = VW::seed_vw_model(*vw, {"-t"});
  EXPECT_NE(seeded, nullptr);

  // The seeded model should share weights, so predictions should be similar
  auto* ex2 = VW::read_example(*seeded, "| a b c");
  seeded->predict(*ex2);
  float seeded_pred = VW::get_prediction(ex2);
  VW::finish_example(*seeded, *ex2);

  auto* ex3 = VW::read_example(*vw, "| a b c");
  vw->predict(*ex3);
  float orig_pred = VW::get_prediction(ex3);
  VW::finish_example(*vw, *ex3);

  EXPECT_FLOAT_EQ(seeded_pred, orig_pred);
}

// --- Topic prediction (LDA-style) ---

TEST(VwCApi, GetTopicPrediction)
{
  auto vw = VW::initialize(vwtest::make_args("--lda", "3", "--quiet", "-b", "10"));
  auto* ex = VW::read_example(*vw, "| word1 word2 word3");
  vw->learn(*ex);

  // After learning, topic predictions should be set in the scalars
  // get_topic_prediction accesses pred.scalars[i]
  float topic0 = VW::get_topic_prediction(ex, 0);
  float topic1 = VW::get_topic_prediction(ex, 1);
  float topic2 = VW::get_topic_prediction(ex, 2);

  // At least one should be non-negative (LDA outputs non-negative)
  EXPECT_GE(topic0, 0.0f);
  EXPECT_GE(topic1, 0.0f);
  EXPECT_GE(topic2, 0.0f);

  VW::finish_example(*vw, *ex);
}

// --- alloc/dealloc examples ---

VW_WARNING_STATE_PUSH
VW_WARNING_DISABLE_DEPRECATED_USAGE

TEST(VwCApi, AllocDeallocExamples)
{
  VW::example* examples = VW::alloc_examples(3);
  EXPECT_NE(examples, nullptr);
  VW::dealloc_examples(examples, 3);
}

VW_WARNING_STATE_POP
