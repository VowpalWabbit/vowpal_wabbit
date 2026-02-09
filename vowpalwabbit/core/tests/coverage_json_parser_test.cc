// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

// Batch 16: JSON/Slates JSON parser coverage tests targeting
// parse_example_json.cc and parse_example_slates_json.cc

#include "vw/core/reductions/conditional_contextual_bandit.h"
#include "vw/core/slates_label.h"
#include "vw/core/vw.h"
#include "vw/json_parser/parse_example_json.h"
#include "vw/test_common/test_common.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cfloat>
#include <cmath>
#include <limits>
#include <string>
#include <vector>

// ============================================================
// Section 1: Simple label JSON tests (~15 tests)
// ============================================================

TEST(CoverageJsonParser, SimpleJsonLabelFloat)
{
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--no_stdin", "--quiet"));
  std::string json_text = R"({"_label": 2.5, "ns": {"f1": 1.0}})";
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 1);
  EXPECT_FLOAT_EQ(examples[0]->l.simple.label, 2.5f);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, SimpleJsonLabelUint)
{
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--no_stdin", "--quiet"));
  std::string json_text = R"({"_label": 3, "ns": {"f1": 1.0}})";
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 1);
  EXPECT_FLOAT_EQ(examples[0]->l.simple.label, 3.0f);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, SimpleJsonLabelNegative)
{
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--no_stdin", "--quiet"));
  std::string json_text = R"({"_label": -1.0, "ns": {"f1": 1.0}})";
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 1);
  EXPECT_FLOAT_EQ(examples[0]->l.simple.label, -1.0f);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, SimpleJsonLabelObjectWithWeight)
{
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--no_stdin", "--quiet"));
  std::string json_text = R"({"_label": {"Label": 1.0, "Weight": 0.5}, "ns": {"f1": 1.0}})";
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 1);
  EXPECT_FLOAT_EQ(examples[0]->l.simple.label, 1.0f);
  EXPECT_FLOAT_EQ(examples[0]->weight, 0.5f);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, SimpleJsonLabelObjectWithInitial)
{
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--no_stdin", "--quiet"));
  std::string json_text = R"({"_label": {"Label": 2.0, "Initial": 0.1}, "ns": {"f1": 1.0}})";
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 1);
  EXPECT_FLOAT_EQ(examples[0]->l.simple.label, 2.0f);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, SimpleJsonLabelObjectNaN)
{
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--no_stdin", "--quiet"));
  std::string json_text = R"({"_label": {"Label": "NaN"}, "ns": {"f1": 1.0}})";
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 1);
  EXPECT_TRUE(std::isnan(examples[0]->l.simple.label));
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, SimpleJsonLabelObjectNaNWeight)
{
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--no_stdin", "--quiet"));
  std::string json_text = R"({"_label": {"Label": 1.0, "Weight": "NaN"}, "ns": {"f1": 1.0}})";
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 1);
  EXPECT_FLOAT_EQ(examples[0]->l.simple.label, 1.0f);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, SimpleJsonLabelObjectNaNInitial)
{
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--no_stdin", "--quiet"));
  std::string json_text = R"({"_label": {"Label": 1.0, "Initial": "NaN"}, "ns": {"f1": 1.0}})";
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 1);
  EXPECT_FLOAT_EQ(examples[0]->l.simple.label, 1.0f);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, SimpleJsonLabelString)
{
  // _label as string uses VW::parse_example_label
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--no_stdin", "--quiet"));
  std::string json_text = R"({"_label": "1.0", "ns": {"f1": 1.0}})";
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 1);
  EXPECT_FLOAT_EQ(examples[0]->l.simple.label, 1.0f);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, SimpleJsonLabelZero)
{
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--no_stdin", "--quiet"));
  std::string json_text = R"({"_label": 0, "ns": {"f1": 1.0}})";
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 1);
  EXPECT_FLOAT_EQ(examples[0]->l.simple.label, 0.0f);
  VW::finish_example(*vw, examples);
}

// ============================================================
// Section 2: Feature parsing tests (~15 tests)
// ============================================================

TEST(CoverageJsonParser, JsonSingleNamespace)
{
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--no_stdin", "--quiet"));
  std::string json_text = R"({"ns": {"f1": 1.0, "f2": 2.0, "f3": 0.5}})";
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 1);
  EXPECT_EQ(examples[0]->feature_space['n'].size(), 3);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, JsonMultipleNamespaces)
{
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--no_stdin", "--quiet"));
  std::string json_text = R"({"alpha": {"a1": 1.0}, "beta": {"b1": 2.0}})";
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 1);
  EXPECT_EQ(examples[0]->feature_space['a'].size(), 1);
  EXPECT_EQ(examples[0]->feature_space['b'].size(), 1);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, JsonDefaultNamespace)
{
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--no_stdin", "--quiet"));
  std::string json_text = R"({"feat1": 1.0, "feat2": 2.0})";
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 1);
  // Top-level features go to default namespace ' '
  EXPECT_EQ(examples[0]->feature_space[' '].size(), 2);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, JsonNestedNamespaces)
{
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--no_stdin", "--quiet"));
  std::string json_text = R"({"outer": {"inner": {"deep_feat": 1.0}, "top_feat": 2.0}})";
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 1);
  // Both features should be in 'o' (first char of outer), inner/deep nesting
  EXPECT_GE(examples[0]->feature_space['o'].size(), 1);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, JsonStringFeatureChainHash)
{
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--no_stdin", "--quiet"));
  std::string json_text = R"({"ns": {"color": "red"}})";
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 1);
  EXPECT_EQ(examples[0]->feature_space['n'].size(), 1);
  // Audit info should have key=color, value=red
  EXPECT_EQ(examples[0]->feature_space['n'].space_names[0].name, "color");
  EXPECT_EQ(examples[0]->feature_space['n'].space_names[0].str_value, "red");
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, JsonStringFeatureNoChainHash)
{
  auto vw = VW::initialize(vwtest::make_args("--json", "--no_stdin", "--quiet"));
  std::string json_text = R"({"ns": {"color": "red"}})";
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 1);
  EXPECT_EQ(examples[0]->feature_space['n'].size(), 1);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, JsonStringFeatureEscapeSpecialChars)
{
  // Spaces, tabs, pipes, colons in string values get replaced with _
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--no_stdin", "--quiet"));
  std::string json_text = R"({"ns": {"key": "has space|pipe:colon"}})";
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 1);
  EXPECT_EQ(examples[0]->feature_space['n'].size(), 1);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, JsonBoolFeatureTrue)
{
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--no_stdin", "--quiet"));
  std::string json_text = R"({"ns": {"is_active": true}})";
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 1);
  EXPECT_EQ(examples[0]->feature_space['n'].size(), 1);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, JsonBoolFeatureFalse)
{
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--no_stdin", "--quiet"));
  std::string json_text = R"({"ns": {"is_active": false}})";
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 1);
  // false bool features are not added
  EXPECT_EQ(examples[0]->feature_space['n'].size(), 0);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, JsonArrayFeatures)
{
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--no_stdin", "--quiet"));
  std::string json_text = R"({"ns": {"arr": [1.0, 2.0, 3.0]}})";
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 1);
  // Array features: 3 numeric values
  EXPECT_EQ(examples[0]->feature_space['a'].size(), 3);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, JsonArrayUintFeatures)
{
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--no_stdin", "--quiet"));
  std::string json_text = R"({"ns": {"arr": [1, 2, 3]}})";
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 1);
  EXPECT_EQ(examples[0]->feature_space['a'].size(), 3);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, JsonArrayWithObjectsFeatures)
{
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--no_stdin", "--quiet"));
  std::string json_text = R"({"items": [{"f1": 1.0}, {"f2": 2.0}]})";
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 1);
  // Array of objects expands features under 'i' namespace
  EXPECT_GE(examples[0]->feature_space['i'].size(), 2);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, JsonArrayNullIgnored)
{
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--no_stdin", "--quiet"));
  std::string json_text = R"({"ns": {"arr": [1.0, null, 3.0]}})";
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 1);
  // null values are ignored in arrays
  EXPECT_EQ(examples[0]->feature_space['a'].size(), 2);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, JsonZeroValueFeatureFiltered)
{
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--no_stdin", "--quiet"));
  std::string json_text = R"({"ns": {"nonzero": 1.0, "zero": 0.0}})";
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 1);
  // Zero-value features are filtered out by namespace_builder::add_feature
  EXPECT_EQ(examples[0]->feature_space['n'].size(), 1);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, JsonUintFeature)
{
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--no_stdin", "--quiet"));
  std::string json_text = R"({"ns": {"count": 42}})";
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 1);
  EXPECT_EQ(examples[0]->feature_space['n'].size(), 1);
  EXPECT_FLOAT_EQ(examples[0]->feature_space['n'].values[0], 42.0f);
  VW::finish_example(*vw, examples);
}

// ============================================================
// Section 3: _text field tests (~5 tests)
// ============================================================

TEST(CoverageJsonParser, JsonTextFieldSingleWord)
{
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--no_stdin", "--quiet"));
  std::string json_text = R"({"_text": "hello"})";
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 1);
  EXPECT_EQ(examples[0]->feature_space[' '].size(), 1);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, JsonTextFieldMultipleWords)
{
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--no_stdin", "--quiet"));
  std::string json_text = R"({"_text": "the quick brown fox"})";
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 1);
  EXPECT_EQ(examples[0]->feature_space[' '].size(), 4);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, JsonTextFieldWithSpecialChars)
{
  // colons and pipes get replaced with _ in text
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--no_stdin", "--quiet"));
  std::string json_text = R"({"_text": "word1|word2:word3"})";
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 1);
  // Single combined token after special chars replaced
  EXPECT_GE(examples[0]->feature_space[' '].size(), 1);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, JsonTextFieldWithTabs)
{
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--no_stdin", "--quiet"));
  std::string json_text = "{\"_text\": \"word1\\tword2\"}";
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 1);
  EXPECT_EQ(examples[0]->feature_space[' '].size(), 2);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, JsonTextFieldEmpty)
{
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--no_stdin", "--quiet"));
  std::string json_text = R"({"_text": ""})";
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 1);
  EXPECT_EQ(examples[0]->feature_space[' '].size(), 0);
  VW::finish_example(*vw, examples);
}

// ============================================================
// Section 4: _tag field tests (~3 tests)
// ============================================================

TEST(CoverageJsonParser, JsonTagField)
{
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--no_stdin", "--quiet"));
  std::string json_text = R"({"_tag": "my_tag", "ns": {"f1": 1.0}})";
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 1);
  std::string tag(examples[0]->tag.begin(), examples[0]->tag.end());
  EXPECT_EQ(tag, "my_tag");
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, JsonTagFieldEmpty)
{
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--no_stdin", "--quiet"));
  std::string json_text = R"({"_tag": "", "ns": {"f1": 1.0}})";
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 1);
  EXPECT_EQ(examples[0]->tag.size(), 0);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, JsonTagFieldLong)
{
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--no_stdin", "--quiet"));
  std::string long_tag = "tag_" + std::string(100, 'x');
  std::string json_text = R"({"_tag": ")" + long_tag + R"(", "ns": {"f1": 1.0}})";
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 1);
  std::string tag(examples[0]->tag.begin(), examples[0]->tag.end());
  EXPECT_EQ(tag, long_tag);
  VW::finish_example(*vw, examples);
}

// ============================================================
// Section 5: CB label JSON tests (~10 tests)
// ============================================================

TEST(CoverageJsonParser, CbJsonBasicLabel)
{
  std::string json_text = R"(
  {
    "_labelIndex": 0,
    "_label_Action": 1,
    "_label_Cost": 0.5,
    "_label_Probability": 0.25,
    "_multi": [
      {"a_": {"f1": 1.0}},
      {"a_": {"f2": 1.0}}
    ]
  })";
  auto vw = VW::initialize(vwtest::make_args("--cb_adf", "--json", "--chain_hash", "--no_stdin", "--quiet"));
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 3);
  // _labelIndex=0 => label goes to examples[0+1]=examples[1] (first action)
  EXPECT_EQ(examples[1]->l.cb.costs.size(), 1);
  EXPECT_FLOAT_EQ(examples[1]->l.cb.costs[0].cost, 0.5f);
  EXPECT_FLOAT_EQ(examples[1]->l.cb.costs[0].probability, 0.25f);
  EXPECT_EQ(examples[1]->l.cb.costs[0].action, 1);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, CbJsonLabelIndex)
{
  std::string json_text = R"(
  {
    "_labelIndex": 1,
    "_label_Action": 2,
    "_label_Cost": -1.0,
    "_label_Probability": 0.8,
    "_multi": [
      {"a_": {"f1": 1.0}},
      {"a_": {"f2": 1.0}},
      {"a_": {"f3": 1.0}}
    ]
  })";
  auto vw = VW::initialize(vwtest::make_args("--cb_adf", "--json", "--chain_hash", "--no_stdin", "--quiet"));
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 4);
  // Label should be on example at index 1+1=2 (skip shared)
  EXPECT_EQ(examples[2]->l.cb.costs.size(), 1);
  EXPECT_FLOAT_EQ(examples[2]->l.cb.costs[0].cost, -1.0f);
  EXPECT_FLOAT_EQ(examples[2]->l.cb.costs[0].probability, 0.8f);
  EXPECT_EQ(examples[2]->l.cb.costs[0].action, 2);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, CbJsonSharedFeatures)
{
  std::string json_text = R"(
  {
    "shared_ns": {"sf1": 1.0},
    "_multi": [
      {"action_ns": {"af1": 1.0}}
    ]
  })";
  auto vw = VW::initialize(vwtest::make_args("--cb_adf", "--json", "--chain_hash", "--no_stdin", "--quiet"));
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 2);
  // Shared example has shared_ns
  EXPECT_EQ(examples[0]->feature_space['s'].size(), 1);
  // Action example has action_ns
  EXPECT_EQ(examples[1]->feature_space['a'].size(), 1);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, CbJsonMultipleActions)
{
  std::string json_text = R"(
  {
    "_multi": [
      {"a_": {"f1": 1.0}},
      {"b_": {"f2": 2.0}},
      {"c_": {"f3": 3.0}},
      {"d_": {"f4": 4.0}}
    ]
  })";
  auto vw = VW::initialize(vwtest::make_args("--cb_adf", "--json", "--chain_hash", "--no_stdin", "--quiet"));
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 5);  // shared + 4 actions
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, CbJsonWithTagOnAction)
{
  std::string json_text = R"(
  {
    "_multi": [
      {"_tag": "action1", "a_": {"f1": 1.0}},
      {"_tag": "action2", "a_": {"f2": 1.0}}
    ]
  })";
  auto vw = VW::initialize(vwtest::make_args("--cb_adf", "--json", "--chain_hash", "--no_stdin", "--quiet"));
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 3);
  std::string tag1(examples[1]->tag.begin(), examples[1]->tag.end());
  std::string tag2(examples[2]->tag.begin(), examples[2]->tag.end());
  EXPECT_EQ(tag1, "action1");
  EXPECT_EQ(tag2, "action2");
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, CbJsonNoLabel)
{
  std::string json_text = R"(
  {
    "shared_ns": {"sf1": 1.0},
    "_multi": [
      {"a_": {"f1": 1.0}},
      {"a_": {"f2": 1.0}}
    ]
  })";
  auto vw = VW::initialize(vwtest::make_args("--cb_adf", "--json", "--chain_hash", "--no_stdin", "--quiet", "-t"));
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 3);
  // No label on any action
  EXPECT_EQ(examples[1]->l.cb.costs.size(), 0);
  EXPECT_EQ(examples[2]->l.cb.costs.size(), 0);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, CbJsonNaNCost)
{
  std::string json_text = R"(
  {
    "_labelIndex": 0,
    "_label_Action": 1,
    "_label_Cost": "NaN",
    "_label_Probability": 0.5,
    "_multi": [
      {"a_": {"f1": 1.0}},
      {"a_": {"f2": 1.0}}
    ]
  })";
  auto vw = VW::initialize(vwtest::make_args("--cb_adf", "--json", "--chain_hash", "--no_stdin", "--quiet"));
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 3);
  EXPECT_EQ(examples[1]->l.cb.costs.size(), 1);
  EXPECT_TRUE(std::isnan(examples[1]->l.cb.costs[0].cost));
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, CbJsonNaNProbability)
{
  std::string json_text = R"(
  {
    "_labelIndex": 0,
    "_label_Action": 1,
    "_label_Cost": 1.0,
    "_label_Probability": "NaN",
    "_multi": [
      {"a_": {"f1": 1.0}},
      {"a_": {"f2": 1.0}}
    ]
  })";
  auto vw = VW::initialize(vwtest::make_args("--cb_adf", "--json", "--chain_hash", "--no_stdin", "--quiet"));
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 3);
  EXPECT_EQ(examples[1]->l.cb.costs.size(), 1);
  EXPECT_TRUE(std::isnan(examples[1]->l.cb.costs[0].probability));
  VW::finish_example(*vw, examples);
}

// ============================================================
// Section 6: CCB label JSON tests (~8 tests)
// ============================================================

TEST(CoverageJsonParser, CcbJsonBasic)
{
  std::string json_text = R"(
  {
    "shared_ns": {"sf": 1.0},
    "_multi": [
      {"a_": {"f1": 1.0}},
      {"a_": {"f2": 1.0}}
    ],
    "_slots": [
      {"slot_ns": {"s1": 1.0}},
      {"slot_ns": {"s2": 1.0}}
    ]
  })";
  auto vw = VW::initialize(vwtest::make_args("--ccb_explore_adf", "--json", "--chain_hash", "--no_stdin", "--quiet"));
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 5);
  EXPECT_EQ(examples[0]->l.conditional_contextual_bandit.type, VW::ccb_example_type::SHARED);
  EXPECT_EQ(examples[1]->l.conditional_contextual_bandit.type, VW::ccb_example_type::ACTION);
  EXPECT_EQ(examples[2]->l.conditional_contextual_bandit.type, VW::ccb_example_type::ACTION);
  EXPECT_EQ(examples[3]->l.conditional_contextual_bandit.type, VW::ccb_example_type::SLOT);
  EXPECT_EQ(examples[4]->l.conditional_contextual_bandit.type, VW::ccb_example_type::SLOT);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, CcbJsonSlotWithIncludedActions)
{
  std::string json_text = R"(
  {
    "_multi": [
      {"a_": {"f1": 1.0}},
      {"a_": {"f2": 1.0}},
      {"a_": {"f3": 1.0}}
    ],
    "_slots": [
      {"_inc": [0, 2], "slot_ns": {"s1": 1.0}},
      {"slot_ns": {"s2": 1.0}}
    ]
  })";
  auto vw = VW::initialize(vwtest::make_args("--ccb_explore_adf", "--json", "--chain_hash", "--no_stdin", "--quiet"));
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 6);
  auto& slot_label = examples[4]->l.conditional_contextual_bandit;
  EXPECT_EQ(slot_label.explicit_included_actions.size(), 2);
  EXPECT_EQ(slot_label.explicit_included_actions[0], 0);
  EXPECT_EQ(slot_label.explicit_included_actions[1], 2);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, CcbJsonSlotWithOutcome)
{
  std::string json_text = R"(
  {
    "_multi": [
      {"a_": {"f1": 1.0}},
      {"a_": {"f2": 1.0}}
    ],
    "_slots": [
      {
        "_label_cost": 3.0,
        "_a": [1, 0],
        "_p": [0.6, 0.4],
        "slot_ns": {"s1": 1.0}
      }
    ]
  })";
  auto vw = VW::initialize(vwtest::make_args("--ccb_explore_adf", "--json", "--chain_hash", "--no_stdin", "--quiet"));
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 4);  // shared + 2 actions + 1 slot
  auto& slot_label = examples[3]->l.conditional_contextual_bandit;
  EXPECT_NE(slot_label.outcome, nullptr);
  EXPECT_FLOAT_EQ(slot_label.outcome->cost, 3.0f);
  EXPECT_EQ(slot_label.outcome->probabilities.size(), 2);
  EXPECT_EQ(slot_label.outcome->probabilities[0].action, 1);
  EXPECT_FLOAT_EQ(slot_label.outcome->probabilities[0].score, 0.6f);
  EXPECT_EQ(slot_label.outcome->probabilities[1].action, 0);
  EXPECT_FLOAT_EQ(slot_label.outcome->probabilities[1].score, 0.4f);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, CcbJsonSlotSingleActionProb)
{
  std::string json_text = R"(
  {
    "_multi": [
      {"a_": {"f1": 1.0}},
      {"a_": {"f2": 1.0}}
    ],
    "_slots": [
      {
        "_label_cost": 1.5,
        "_a": 0,
        "_p": 0.9,
        "slot_ns": {"s1": 1.0}
      }
    ]
  })";
  auto vw = VW::initialize(vwtest::make_args("--ccb_explore_adf", "--json", "--chain_hash", "--no_stdin", "--quiet"));
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 4);
  auto& slot_label = examples[3]->l.conditional_contextual_bandit;
  EXPECT_NE(slot_label.outcome, nullptr);
  EXPECT_FLOAT_EQ(slot_label.outcome->cost, 1.5f);
  EXPECT_EQ(slot_label.outcome->probabilities.size(), 1);
  EXPECT_EQ(slot_label.outcome->probabilities[0].action, 0);
  EXPECT_FLOAT_EQ(slot_label.outcome->probabilities[0].score, 0.9f);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, CcbJsonSlotNoOutcome)
{
  std::string json_text = R"(
  {
    "_multi": [
      {"a_": {"f1": 1.0}}
    ],
    "_slots": [
      {"slot_ns": {"s1": 1.0}}
    ]
  })";
  auto vw = VW::initialize(vwtest::make_args("--ccb_explore_adf", "--json", "--chain_hash", "--no_stdin", "--quiet"));
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 3);
  auto& slot_label = examples[2]->l.conditional_contextual_bandit;
  EXPECT_EQ(slot_label.outcome, nullptr);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, CcbJsonMultipleSlots)
{
  std::string json_text = R"(
  {
    "_multi": [
      {"a_": {"f1": 1.0}},
      {"a_": {"f2": 1.0}},
      {"a_": {"f3": 1.0}}
    ],
    "_slots": [
      {"slot_ns": {"s1": 1.0}},
      {"slot_ns": {"s2": 1.0}},
      {"slot_ns": {"s3": 1.0}}
    ]
  })";
  auto vw = VW::initialize(vwtest::make_args("--ccb_explore_adf", "--json", "--chain_hash", "--no_stdin", "--quiet"));
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 7);  // shared + 3 actions + 3 slots
  for (int i = 4; i <= 6; i++)
  {
    EXPECT_EQ(examples[i]->l.conditional_contextual_bandit.type, VW::ccb_example_type::SLOT);
  }
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, CcbJsonCbAsCcbConversion)
{
  // When CB label fields are present without _slots, CCB auto-generates a single slot
  std::string json_text = R"(
  {
    "_label_Action": 2,
    "_label_Cost": 0.5,
    "_label_Probability": 0.25,
    "_labelIndex": 0,
    "_multi": [
      {"a_": {"f1": 1.0}},
      {"a_": {"f2": 1.0}}
    ]
  })";
  auto vw = VW::initialize(vwtest::make_args("--ccb_explore_adf", "--json", "--chain_hash", "--no_stdin", "--quiet"));
  auto examples = vwtest::parse_json(*vw, json_text);
  // shared + 2 actions + 1 auto-generated slot
  EXPECT_EQ(examples.size(), 4);
  EXPECT_EQ(examples[3]->l.conditional_contextual_bandit.type, VW::ccb_example_type::SLOT);
  EXPECT_NE(examples[3]->l.conditional_contextual_bandit.outcome, nullptr);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, CcbJsonSlotWithId)
{
  std::string json_text = R"(
  {
    "_multi": [
      {"a_": {"f1": 1.0}}
    ],
    "_slots": [
      {"_id": "slot_abc", "slot_ns": {"s1": 1.0}}
    ]
  })";
  auto vw = VW::initialize(vwtest::make_args("--ccb_explore_adf", "--json", "--chain_hash", "--no_stdin", "--quiet"));
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 3);
  EXPECT_EQ(examples[2]->l.conditional_contextual_bandit.type, VW::ccb_example_type::SLOT);
  VW::finish_example(*vw, examples);
}

// ============================================================
// Section 7: Continuous actions (CATS) JSON tests (~5 tests)
// ============================================================

TEST(CoverageJsonParser, CatsJsonLabelFields)
{
  std::string json_text = R"(
  {
    "_label_ca": {
      "cost": 1.5,
      "pdf_value": 0.001,
      "action": 200.0
    },
    "f1": 1, "f2": 1
  })";
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--cats", "4", "--min_value=185",
      "--max_value=23959", "--bandwidth", "1", "--no_stdin", "--quiet"));
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 1);
  EXPECT_EQ(examples[0]->l.cb_cont.costs.size(), 1);
  EXPECT_FLOAT_EQ(examples[0]->l.cb_cont.costs[0].cost, 1.5f);
  EXPECT_FLOAT_EQ(examples[0]->l.cb_cont.costs[0].pdf_value, 0.001f);
  EXPECT_FLOAT_EQ(examples[0]->l.cb_cont.costs[0].action, 200.0f);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, CatsJsonNoLabel)
{
  std::string json_text = R"({"f1": 1, "f2": 1})";
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "-t", "--cats", "4", "--min_value=185",
      "--max_value=23959", "--bandwidth", "1", "--no_stdin", "--quiet"));
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 1);
  EXPECT_EQ(examples[0]->l.cb_cont.costs.size(), 0);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, CatsJsonNaNPdfValue)
{
  std::string json_text = R"(
  {
    "_label_ca": {
      "cost": 1.0,
      "pdf_value": "NaN",
      "action": 200.0
    },
    "f1": 1
  })";
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--cats", "4", "--min_value=185",
      "--max_value=23959", "--bandwidth", "1", "--no_stdin", "--quiet"));
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 1);
  EXPECT_EQ(examples[0]->l.cb_cont.costs.size(), 1);
  EXPECT_TRUE(std::isnan(examples[0]->l.cb_cont.costs[0].pdf_value));
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, CatsJsonUintFields)
{
  std::string json_text = R"(
  {
    "_label_ca": {
      "cost": 1,
      "pdf_value": 0,
      "action": 200
    },
    "f1": 1
  })";
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--cats", "4", "--min_value=185",
      "--max_value=23959", "--bandwidth", "1", "--no_stdin", "--quiet"));
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 1);
  // Uint cost=1 gets through Uint->Float conversion, but pdf_value=0 may cause empty due to float cast
  EXPECT_EQ(examples[0]->l.cb_cont.costs.size(), 1);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, CatsJsonMultipleFeatures)
{
  std::string json_text = R"(
  {
    "_label_ca": {
      "cost": 0.5,
      "pdf_value": 0.01,
      "action": 190.0
    },
    "age": 1, "gender": 1, "location": 1, "device": 1
  })";
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--cats", "4", "--min_value=185",
      "--max_value=23959", "--bandwidth", "1", "--no_stdin", "--quiet"));
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 1);
  EXPECT_EQ(examples[0]->feature_space[' '].size(), 4);
  VW::finish_example(*vw, examples);
}

// ============================================================
// Section 8: DSJson tests (~10 tests)
// ============================================================

TEST(CoverageJsonParser, DsJsonCbBasic)
{
  std::string json_text = R"(
  {
    "_label_cost": -0.5,
    "_label_probability": 0.7,
    "_label_Action": 1,
    "_labelIndex": 0,
    "EventId": "evt1",
    "Timestamp": "2023-01-01T00:00:00Z",
    "a": [1, 2],
    "c": {
      "shared": {"sf": 1},
      "_multi": [
        {"ns": {"f1": 1}},
        {"ns": {"f2": 1}}
      ]
    },
    "p": [0.7, 0.3]
  })";
  auto vw = VW::initialize(vwtest::make_args("--dsjson", "--chain_hash", "--cb_adf", "--no_stdin", "--quiet"));
  VW::parsers::json::decision_service_interaction interaction;
  auto examples = vwtest::parse_dsjson(*vw, json_text, &interaction);
  EXPECT_EQ(examples.size(), 3);
  EXPECT_EQ(interaction.event_id, "evt1");
  EXPECT_EQ(interaction.timestamp, "2023-01-01T00:00:00Z");
  EXPECT_EQ(interaction.actions.size(), 2);
  EXPECT_EQ(interaction.probabilities.size(), 2);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, DsJsonCbSkipLearn)
{
  std::string json_text = R"(
  {
    "_skipLearn": true,
    "EventId": "skip_evt",
    "c": {
      "_multi": [
        {"ns": {"f1": 1}}
      ]
    },
    "p": [1.0],
    "a": [1]
  })";
  auto vw = VW::initialize(vwtest::make_args("--dsjson", "--chain_hash", "--cb_adf", "--no_stdin", "--quiet"));
  VW::parsers::json::decision_service_interaction interaction;
  auto examples = vwtest::parse_dsjson(*vw, json_text, &interaction);
  EXPECT_EQ(interaction.skip_learn, true);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, DsJsonCbPdrop)
{
  std::string json_text = R"(
  {
    "pdrop": 0.3,
    "EventId": "pdrop_evt",
    "c": {
      "_multi": [
        {"ns": {"f1": 1}}
      ]
    },
    "p": [1.0],
    "a": [1]
  })";
  auto vw = VW::initialize(vwtest::make_args("--dsjson", "--chain_hash", "--cb_adf", "--no_stdin", "--quiet"));
  VW::parsers::json::decision_service_interaction interaction;
  auto examples = vwtest::parse_dsjson(*vw, json_text, &interaction);
  EXPECT_FLOAT_EQ(interaction.probability_of_drop, 0.3f);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, DsJsonCcbOutcomes)
{
  std::string json_text = R"(
  {
    "EventId": "ccb_evt",
    "Timestamp": "2023-06-01T00:00:00Z",
    "c": {
      "shared_ns": {"sf": 1.0},
      "_multi": [
        {"a_": {"f1": 1}},
        {"a_": {"f2": 1}}
      ],
      "_slots": [
        {"_id": "slot1", "_inc": [0, 1]},
        {"_id": "slot2"}
      ]
    },
    "_outcomes": [
      {
        "_label_cost": 1.0,
        "_a": 0,
        "_p": 0.75
      },
      {
        "_label_cost": 2.0,
        "_a": [1, 0],
        "_p": [0.6, 0.4]
      }
    ]
  })";
  auto vw = VW::initialize(vwtest::make_args("--ccb_explore_adf", "--dsjson", "--chain_hash", "--no_stdin", "--quiet"));
  VW::parsers::json::decision_service_interaction interaction;
  auto examples = vwtest::parse_dsjson(*vw, json_text, &interaction);
  EXPECT_EQ(examples.size(), 5);
  EXPECT_EQ(interaction.event_id, "ccb_evt");

  auto& slot1 = examples[3]->l.conditional_contextual_bandit;
  EXPECT_NE(slot1.outcome, nullptr);
  EXPECT_FLOAT_EQ(slot1.outcome->cost, 1.0f);
  EXPECT_EQ(slot1.outcome->probabilities.size(), 1);

  auto& slot2 = examples[4]->l.conditional_contextual_bandit;
  EXPECT_NE(slot2.outcome, nullptr);
  EXPECT_FLOAT_EQ(slot2.outcome->cost, 2.0f);
  EXPECT_EQ(slot2.outcome->probabilities.size(), 2);

  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, DsJsonBaselineActions)
{
  std::string json_text = R"(
  {
    "EventId": "ba_evt",
    "c": {
      "_multi": [
        {"ns": {"f1": 1}},
        {"ns": {"f2": 1}}
      ]
    },
    "a": [1, 2],
    "p": [0.5, 0.5],
    "_ba": [1, 2]
  })";
  auto vw = VW::initialize(vwtest::make_args("--dsjson", "--chain_hash", "--cb_adf", "--no_stdin", "--quiet"));
  VW::parsers::json::decision_service_interaction interaction;
  auto examples = vwtest::parse_dsjson(*vw, json_text, &interaction);
  EXPECT_EQ(interaction.baseline_actions.size(), 2);
  EXPECT_EQ(interaction.baseline_actions[0], 1);
  EXPECT_EQ(interaction.baseline_actions[1], 2);
  VW::finish_example(*vw, examples);
}

// ============================================================
// Section 9: Slates JSON tests (~15 tests)
// ============================================================

TEST(CoverageJsonParser, SlatesJsonBasicStructure)
{
  std::string json_text = R"(
  {
    "GUser": {"id": "user1"},
    "_multi": [
      {"_slot_id": 0, "TAction": {"topic": "sports"}},
      {"_slot_id": 0, "TAction": {"topic": "news"}},
      {"_slot_id": 1, "TAction": {"topic": "weather"}}
    ],
    "_slots": [
      {"slot_feature": 1},
      {"slot_feature": 2}
    ]
  })";
  auto vw = VW::initialize(
      vwtest::make_args("--slates", "--dsjson", "--chain_hash", "--no_stdin", "--noconstant", "--quiet"));
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 6);  // shared + 3 actions + 2 slots
  EXPECT_EQ(examples[0]->l.slates.type, VW::slates::example_type::SHARED);
  EXPECT_EQ(examples[1]->l.slates.type, VW::slates::example_type::ACTION);
  EXPECT_EQ(examples[2]->l.slates.type, VW::slates::example_type::ACTION);
  EXPECT_EQ(examples[3]->l.slates.type, VW::slates::example_type::ACTION);
  EXPECT_EQ(examples[4]->l.slates.type, VW::slates::example_type::SLOT);
  EXPECT_EQ(examples[5]->l.slates.type, VW::slates::example_type::SLOT);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, SlatesJsonSlotIds)
{
  std::string json_text = R"(
  {
    "_multi": [
      {"_slot_id": 0, "TAction": {"f": 1}},
      {"_slot_id": 1, "TAction": {"f": 1}},
      {"_slot_id": 2, "TAction": {"f": 1}}
    ],
    "_slots": [
      {"sf": 1},
      {"sf": 2},
      {"sf": 3}
    ]
  })";
  auto vw = VW::initialize(
      vwtest::make_args("--slates", "--dsjson", "--chain_hash", "--no_stdin", "--noconstant", "--quiet"));
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 7);
  EXPECT_EQ(examples[1]->l.slates.slot_id, 0);
  EXPECT_EQ(examples[2]->l.slates.slot_id, 1);
  EXPECT_EQ(examples[3]->l.slates.slot_id, 2);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, SlatesJsonSharedFeatures)
{
  std::string json_text = R"(
  {
    "User": {"age": 25, "gender": 1},
    "_multi": [
      {"_slot_id": 0, "A": {"f": 1}}
    ],
    "_slots": [
      {"S": {"s_f": 1}}
    ]
  })";
  auto vw = VW::initialize(
      vwtest::make_args("--slates", "--dsjson", "--chain_hash", "--no_stdin", "--noconstant", "--quiet"));
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 3);
  EXPECT_EQ(examples[0]->l.slates.type, VW::slates::example_type::SHARED);
  EXPECT_GE(examples[0]->feature_space['U'].size(), 1);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, SlatesJsonNoMulti)
{
  // Shared example with _slots but no _multi
  std::string json_text = R"(
  {
    "User": {"age": 25},
    "_slots": [
      {"S": {"s_f": 1}}
    ]
  })";
  auto vw = VW::initialize(
      vwtest::make_args("--slates", "--dsjson", "--chain_hash", "--no_stdin", "--noconstant", "--quiet"));
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 2);  // shared + 1 slot
  EXPECT_EQ(examples[0]->l.slates.type, VW::slates::example_type::SHARED);
  EXPECT_EQ(examples[1]->l.slates.type, VW::slates::example_type::SLOT);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, SlatesJsonNoSlots)
{
  // Shared example with _multi but no _slots
  std::string json_text = R"(
  {
    "User": {"age": 25},
    "_multi": [
      {"_slot_id": 0, "A": {"f": 1}}
    ]
  })";
  auto vw = VW::initialize(
      vwtest::make_args("--slates", "--dsjson", "--chain_hash", "--no_stdin", "--noconstant", "--quiet"));
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 2);  // shared + 1 action
  EXPECT_EQ(examples[0]->l.slates.type, VW::slates::example_type::SHARED);
  EXPECT_EQ(examples[1]->l.slates.type, VW::slates::example_type::ACTION);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, SlatesJsonActionFeatures)
{
  std::string json_text = R"(
  {
    "_multi": [
      {"_slot_id": 0, "Action": {"topic": "sports", "relevance": 0.9}}
    ],
    "_slots": [
      {"Slot": {"position": 1}}
    ]
  })";
  auto vw = VW::initialize(
      vwtest::make_args("--slates", "--dsjson", "--chain_hash", "--no_stdin", "--noconstant", "--quiet"));
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 3);
  // Action example should have features in namespace 'A'
  EXPECT_GE(examples[1]->feature_space['A'].size(), 1);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, SlatesDsJsonWithOutcomes)
{
  std::string json_text = R"(
  {
    "_label_cost": 2.5,
    "_outcomes": [
      {
        "_a": 0,
        "_p": 0.7
      },
      {
        "_a": [1, 0],
        "_p": [0.5, 0.5]
      }
    ],
    "EventId": "slates_evt",
    "c": {
      "_multi": [
        {"_slot_id": 0, "A": {"f": 1}},
        {"_slot_id": 0, "A": {"f": 2}},
        {"_slot_id": 1, "A": {"f": 3}},
        {"_slot_id": 1, "A": {"f": 4}}
      ],
      "_slots": [
        {"S": {"s": 1}},
        {"S": {"s": 2}}
      ]
    }
  })";
  auto vw = VW::initialize(vwtest::make_args("--slates", "--dsjson", "--chain_hash", "--no_stdin", "--quiet"));
  VW::parsers::json::decision_service_interaction interaction;
  auto examples = vwtest::parse_dsjson(*vw, json_text, &interaction);
  EXPECT_EQ(examples.size(), 7);  // shared + 4 actions + 2 slots
  // Check shared label cost
  EXPECT_FLOAT_EQ(examples[0]->l.slates.cost, 2.5f);
  EXPECT_EQ(examples[0]->l.slates.labeled, true);
  // Slot 1 probabilities
  EXPECT_EQ(examples[5]->l.slates.probabilities.size(), 1);
  EXPECT_EQ(examples[5]->l.slates.probabilities[0].action, 0);
  EXPECT_FLOAT_EQ(examples[5]->l.slates.probabilities[0].score, 0.7f);
  // Slot 2 probabilities
  EXPECT_EQ(examples[6]->l.slates.probabilities.size(), 2);
  EXPECT_EQ(examples[6]->l.slates.probabilities[0].action, 1);
  EXPECT_FLOAT_EQ(examples[6]->l.slates.probabilities[0].score, 0.5f);
  EXPECT_EQ(examples[6]->l.slates.probabilities[1].action, 0);
  EXPECT_FLOAT_EQ(examples[6]->l.slates.probabilities[1].score, 0.5f);

  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, SlatesDsJsonEventFields)
{
  std::string json_text = R"(
  {
    "EventId": "slates_event_123",
    "Timestamp": "2023-06-15T12:00:00Z",
    "_skipLearn": false,
    "pdrop": 0.05,
    "c": {
      "_multi": [
        {"_slot_id": 0, "A": {"f": 1}}
      ],
      "_slots": [
        {"S": {"s": 1}}
      ]
    }
  })";
  auto vw = VW::initialize(vwtest::make_args("--slates", "--dsjson", "--chain_hash", "--no_stdin", "--quiet"));
  VW::parsers::json::decision_service_interaction interaction;
  auto examples = vwtest::parse_dsjson(*vw, json_text, &interaction);
  EXPECT_EQ(interaction.event_id, "slates_event_123");
  EXPECT_EQ(interaction.timestamp, "2023-06-15T12:00:00Z");
  EXPECT_EQ(interaction.skip_learn, false);
  EXPECT_FLOAT_EQ(interaction.probability_of_drop, 0.05f);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, SlatesJsonManyActionsPerSlot)
{
  std::string json_text = R"(
  {
    "_multi": [
      {"_slot_id": 0, "A": {"f1": 1}},
      {"_slot_id": 0, "A": {"f2": 1}},
      {"_slot_id": 0, "A": {"f3": 1}},
      {"_slot_id": 0, "A": {"f4": 1}},
      {"_slot_id": 0, "A": {"f5": 1}}
    ],
    "_slots": [
      {"S": {"s": 1}}
    ]
  })";
  auto vw = VW::initialize(
      vwtest::make_args("--slates", "--dsjson", "--chain_hash", "--no_stdin", "--noconstant", "--quiet"));
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 7);  // shared + 5 actions + 1 slot
  for (int i = 1; i <= 5; i++)
  {
    EXPECT_EQ(examples[i]->l.slates.type, VW::slates::example_type::ACTION);
    EXPECT_EQ(examples[i]->l.slates.slot_id, 0);
  }
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, SlatesJsonStringFeatures)
{
  std::string json_text = R"(
  {
    "User": {"name": "alice", "city": "NYC"},
    "_multi": [
      {"_slot_id": 0, "A": {"topic": "music"}}
    ],
    "_slots": [
      {"S": {"type": "banner"}}
    ]
  })";
  auto vw = VW::initialize(
      vwtest::make_args("--slates", "--dsjson", "--chain_hash", "--no_stdin", "--noconstant", "--quiet"));
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 3);
  // Check audit info for string features
  EXPECT_GE(examples[0]->feature_space['U'].space_names.size(), 2);
  VW::finish_example(*vw, examples);
}

// ============================================================
// Section 10: Edge cases and error handling (~15 tests)
// ============================================================

TEST(CoverageJsonParser, JsonEmptyObject)
{
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--no_stdin", "--quiet"));
  std::string json_text = R"({})";
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 1);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, JsonEmptyNamespace)
{
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--no_stdin", "--quiet"));
  std::string json_text = R"({"ns": {}})";
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 1);
  // Empty namespace = no features
  EXPECT_EQ(examples[0]->feature_space['n'].size(), 0);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, JsonMalformedThrows)
{
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--no_stdin", "--quiet"));
  std::string json_text = R"({invalid json)";
  VW::multi_ex examples;
  examples.push_back(&VW::get_unused_example(vw.get()));
  EXPECT_THROW(VW::parsers::json::read_line_json<true>(*vw, examples, (char*)json_text.c_str(), json_text.length(),
                   std::bind(VW::get_unused_example, vw.get())),
      VW::vw_exception);
  for (auto* ex : examples) { VW::finish_example(*vw, *ex); }
}

TEST(CoverageJsonParser, JsonMissingClosingBrace)
{
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--no_stdin", "--quiet"));
  std::string json_text = R"({"ns": {"f1": 1.0})";
  VW::multi_ex examples;
  examples.push_back(&VW::get_unused_example(vw.get()));
  EXPECT_THROW(VW::parsers::json::read_line_json<true>(*vw, examples, (char*)json_text.c_str(), json_text.length(),
                   std::bind(VW::get_unused_example, vw.get())),
      VW::vw_exception);
  for (auto* ex : examples) { VW::finish_example(*vw, *ex); }
}

TEST(CoverageJsonParser, JsonReservedFieldsIgnored)
{
  // Fields starting with _ that are not recognized should be ignored
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--no_stdin", "--quiet"));
  std::string json_text = R"({"_unknown_field": "value", "_another": 42, "ns": {"f1": 1.0}})";
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 1);
  EXPECT_EQ(examples[0]->feature_space['n'].size(), 1);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, JsonNullValueHandled)
{
  // Null at top level should be handled by the state machine
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--no_stdin", "--quiet"));
  std::string json_text = R"({"_some_reserved": null, "ns": {"f1": 1.0}})";
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 1);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, JsonLargeNumberOfFeatures)
{
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--no_stdin", "--quiet"));
  std::string json_text = R"({"ns": {)";
  for (int i = 0; i < 100; i++)
  {
    if (i > 0) { json_text += ","; }
    json_text += "\"f" + std::to_string(i) + "\": " + std::to_string(i + 1);
  }
  json_text += "}}";
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 1);
  EXPECT_EQ(examples[0]->feature_space['n'].size(), 100);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, JsonDeeplyNestedObject)
{
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--no_stdin", "--quiet"));
  std::string json_text = R"({"a": {"b": {"c": {"d": {"e": {"f": 1.0}}}}}})";
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 1);
  // Deep nesting should still produce features
  bool has_features = false;
  for (int c = 0; c < 256; c++)
  {
    if (examples[0]->feature_space[c].size() > 0) { has_features = true; }
  }
  EXPECT_TRUE(has_features);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, JsonNumericPrecision)
{
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--no_stdin", "--quiet"));
  std::string json_text = R"({"ns": {"small": 1e-10, "large": 1e10}})";
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 1);
  EXPECT_EQ(examples[0]->feature_space['n'].size(), 2);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, JsonIntNegativeFeature)
{
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--no_stdin", "--quiet"));
  std::string json_text = R"({"ns": {"neg": -5}})";
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 1);
  EXPECT_EQ(examples[0]->feature_space['n'].size(), 1);
  EXPECT_FLOAT_EQ(examples[0]->feature_space['n'].values[0], -5.0f);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, JsonNamespaceExtentsMultiple)
{
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--no_stdin", "--quiet"));
  std::string json_text = R"(
  {
    "features_a": {"x1": 1, "x2": 2},
    "features_b": {"y1": 3}
  })";
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 1);
  // Both namespaces start with 'f', so they share the feature space but have separate extents
  EXPECT_EQ(examples[0]->feature_space['f'].size(), 3);
  EXPECT_EQ(examples[0]->feature_space['f'].namespace_extents.size(), 2);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, JsonVerifyExtentsNested)
{
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--no_stdin", "--quiet"));
  std::string json_text = R"(
  {
    "outer": {
      "f1": 1,
      "inner": {"f2": 2},
      "f3": 3
    }
  })";
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 1);
  // outer namespace features
  EXPECT_GE(examples[0]->feature_space['o'].size(), 2);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, JsonMultipleStringsInNamespace)
{
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--no_stdin", "--quiet"));
  std::string json_text = R"({"ns": {"color": "red", "shape": "circle", "size": "large"}})";
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 1);
  EXPECT_EQ(examples[0]->feature_space['n'].size(), 3);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, CbJsonEmptyMulti)
{
  std::string json_text = R"(
  {
    "shared_ns": {"sf": 1.0},
    "_multi": []
  })";
  auto vw = VW::initialize(vwtest::make_args("--cb_adf", "--json", "--chain_hash", "--no_stdin", "--quiet"));
  auto examples = vwtest::parse_json(*vw, json_text);
  // shared example only (no action examples)
  EXPECT_EQ(examples.size(), 1);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, CcbJsonEmptySlots)
{
  std::string json_text = R"(
  {
    "_multi": [
      {"a_": {"f1": 1.0}}
    ],
    "_slots": []
  })";
  auto vw = VW::initialize(vwtest::make_args("--ccb_explore_adf", "--json", "--chain_hash", "--no_stdin", "--quiet"));
  auto examples = vwtest::parse_json(*vw, json_text);
  // shared + 1 action, no slots
  EXPECT_EQ(examples.size(), 2);
  VW::finish_example(*vw, examples);
}

// ============================================================
// Section 11: Audit mode JSON tests (~5 tests)
// ============================================================

TEST(CoverageJsonParser, JsonAuditModeNumericFeatures)
{
  // audit=true is the default in parse_json helper
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--no_stdin", "--quiet"));
  std::string json_text = R"({"my_ns": {"score": 0.95, "count": 10}})";
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 1);
  EXPECT_EQ(examples[0]->feature_space['m'].space_names.size(), 2);
  // Check audit names
  bool found_score = false, found_count = false;
  for (const auto& sn : examples[0]->feature_space['m'].space_names)
  {
    if (sn.name == "score") { found_score = true; }
    if (sn.name == "count") { found_count = true; }
  }
  EXPECT_TRUE(found_score);
  EXPECT_TRUE(found_count);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, JsonAuditModeStringFeatures)
{
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--no_stdin", "--quiet"));
  std::string json_text = R"({"ns": {"color": "blue"}})";
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 1);
  EXPECT_EQ(examples[0]->feature_space['n'].space_names.size(), 1);
  EXPECT_EQ(examples[0]->feature_space['n'].space_names[0].ns, "ns");
  EXPECT_EQ(examples[0]->feature_space['n'].space_names[0].name, "color");
  EXPECT_EQ(examples[0]->feature_space['n'].space_names[0].str_value, "blue");
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, JsonAuditModeBoolFeature)
{
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--no_stdin", "--quiet"));
  std::string json_text = R"({"ns": {"is_valid": true}})";
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 1);
  EXPECT_EQ(examples[0]->feature_space['n'].space_names.size(), 1);
  EXPECT_EQ(examples[0]->feature_space['n'].space_names[0].name, "is_valid");
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, JsonAuditModeArrayFeatures)
{
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--no_stdin", "--quiet"));
  std::string json_text = R"({"ns": {"coords": [1.0, 2.0, 3.0]}})";
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 1);
  // Each element gets indexed audit name [0], [1], [2]
  EXPECT_EQ(examples[0]->feature_space['c'].space_names.size(), 3);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, JsonAuditModeNamespaceInfo)
{
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--no_stdin", "--quiet"));
  std::string json_text = R"({"MyNamespace": {"feat1": 1.0}})";
  auto examples = vwtest::parse_json(*vw, json_text);
  EXPECT_EQ(examples.size(), 1);
  EXPECT_EQ(examples[0]->feature_space['M'].space_names.size(), 1);
  EXPECT_EQ(examples[0]->feature_space['M'].space_names[0].ns, "MyNamespace");
  EXPECT_EQ(examples[0]->feature_space['M'].space_names[0].name, "feat1");
  VW::finish_example(*vw, examples);
}

// ============================================================
// Section 12: Dedup tests (~5 tests)
// ============================================================

TEST(CoverageJsonParser, DedupCbSingleAction)
{
  const std::string json_deduped_text = R"(
  {
    "GUser":{"id":"a"},
    "_multi":[{"__aid":12345}]
  })";
  const std::string action_1 = R"({"TAction":{"topic":"sports"}})";
  uint64_t dedup_id_1 = 12345;

  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--cb_explore_adf", "--no_stdin", "--quiet"));

  std::unordered_map<uint64_t, VW::example*> dedup_examples;
  VW::multi_ex examples;

  examples.push_back(&VW::get_unused_example(vw.get()));
  VW::parsers::json::read_line_json<true>(
      *vw, examples, (char*)action_1.c_str(), action_1.length(), std::bind(VW::get_unused_example, vw.get()));
  dedup_examples.emplace(dedup_id_1, examples[0]);
  examples.clear();

  examples.push_back(&VW::get_unused_example(vw.get()));
  VW::parsers::json::read_line_json<true>(*vw, examples, (char*)json_deduped_text.c_str(), json_deduped_text.length(),
      std::bind(VW::get_unused_example, vw.get()), &dedup_examples);

  EXPECT_EQ(examples.size(), 2);
  EXPECT_EQ(examples[1]->indices.size(), 1);
  EXPECT_EQ(examples[1]->indices[0], 'T');

  for (auto* example : examples) { VW::finish_example(*vw, *example); }
  for (auto& dedup : dedup_examples) { VW::finish_example(*vw, *dedup.second); }
}

TEST(CoverageJsonParser, DedupSlatesSingleAction)
{
  const std::string json_deduped_text = R"(
  {
    "GUser":{"id":"a"},
    "_multi":[{"__aid":99999}],
    "_slots":[{"Slot":{"s1":"v1"}}]
  })";
  const std::string action_1 = R"({"TAction":{"a1":"f1"},"_slot_id":0})";
  uint64_t dedup_id_1 = 99999;

  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--slates", "--no_stdin", "--quiet"));

  std::unordered_map<uint64_t, VW::example*> dedup_examples;
  VW::multi_ex examples;

  examples.push_back(&VW::get_unused_example(vw.get()));
  VW::parsers::json::read_line_json<true>(
      *vw, examples, (char*)action_1.c_str(), action_1.length(), std::bind(VW::get_unused_example, vw.get()));
  dedup_examples.emplace(dedup_id_1, examples[0]);
  examples.clear();

  examples.push_back(&VW::get_unused_example(vw.get()));
  VW::parsers::json::read_line_json<true>(*vw, examples, (char*)json_deduped_text.c_str(), json_deduped_text.length(),
      std::bind(VW::get_unused_example, vw.get()), &dedup_examples);

  EXPECT_EQ(examples.size(), 3);  // shared + 1 action + 1 slot
  EXPECT_EQ(examples[0]->l.slates.type, VW::slates::example_type::SHARED);
  EXPECT_EQ(examples[1]->l.slates.type, VW::slates::example_type::ACTION);
  EXPECT_EQ(examples[2]->l.slates.type, VW::slates::example_type::SLOT);

  for (auto* example : examples) { VW::finish_example(*vw, *example); }
  for (auto& dedup : dedup_examples) { VW::finish_example(*vw, *dedup.second); }
}

// ============================================================
// Section 13: Non-audit (template<false>) path (~4 tests)
// ============================================================

TEST(CoverageJsonParser, JsonNonAuditParsing)
{
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--no_stdin", "--quiet"));
  std::string json_text = R"({"ns": {"f1": 1.0, "f2": 2.0}})";

  VW::multi_ex examples;
  examples.push_back(&VW::get_unused_example(vw.get()));
  // Use audit=false template
  VW::parsers::json::read_line_json<false>(
      *vw, examples, (char*)json_text.c_str(), json_text.length(), std::bind(VW::get_unused_example, vw.get()));

  EXPECT_EQ(examples.size(), 1);
  EXPECT_EQ(examples[0]->feature_space['n'].size(), 2);
  // Non-audit mode: space_names should be empty
  EXPECT_EQ(examples[0]->feature_space['n'].space_names.size(), 0);

  for (auto* ex : examples) { VW::finish_example(*vw, *ex); }
}

TEST(CoverageJsonParser, JsonNonAuditStringFeature)
{
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--no_stdin", "--quiet"));
  std::string json_text = R"({"ns": {"color": "red"}})";

  VW::multi_ex examples;
  examples.push_back(&VW::get_unused_example(vw.get()));
  VW::parsers::json::read_line_json<false>(
      *vw, examples, (char*)json_text.c_str(), json_text.length(), std::bind(VW::get_unused_example, vw.get()));

  EXPECT_EQ(examples.size(), 1);
  EXPECT_EQ(examples[0]->feature_space['n'].size(), 1);

  for (auto* ex : examples) { VW::finish_example(*vw, *ex); }
}

TEST(CoverageJsonParser, JsonNonAuditCbMulti)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_adf", "--json", "--chain_hash", "--no_stdin", "--quiet"));
  std::string json_text = R"(
  {
    "_labelIndex": 0,
    "_label_Action": 1,
    "_label_Cost": 0.5,
    "_label_Probability": 0.25,
    "_multi": [
      {"a_": {"f1": 1}},
      {"a_": {"f2": 1}}
    ]
  })";

  VW::multi_ex examples;
  examples.push_back(&VW::get_unused_example(vw.get()));
  VW::parsers::json::read_line_json<false>(
      *vw, examples, (char*)json_text.c_str(), json_text.length(), std::bind(VW::get_unused_example, vw.get()));

  EXPECT_EQ(examples.size(), 3);
  EXPECT_EQ(examples[1]->l.cb.costs.size(), 1);
  EXPECT_FLOAT_EQ(examples[1]->l.cb.costs[0].cost, 0.5f);

  for (auto* ex : examples) { VW::finish_example(*vw, *ex); }
}

TEST(CoverageJsonParser, JsonNonAuditArrayFeatures)
{
  auto vw = VW::initialize(vwtest::make_args("--json", "--chain_hash", "--no_stdin", "--quiet"));
  std::string json_text = R"({"ns": {"arr": [1.0, 2.0, 3.0]}})";

  VW::multi_ex examples;
  examples.push_back(&VW::get_unused_example(vw.get()));
  VW::parsers::json::read_line_json<false>(
      *vw, examples, (char*)json_text.c_str(), json_text.length(), std::bind(VW::get_unused_example, vw.get()));

  EXPECT_EQ(examples.size(), 1);
  EXPECT_EQ(examples[0]->feature_space['a'].size(), 3);
  EXPECT_EQ(examples[0]->feature_space['a'].space_names.size(), 0);  // no audit

  for (auto* ex : examples) { VW::finish_example(*vw, *ex); }
}

// ============================================================
// Section 14: DSJson slates extended tests (~5 tests)
// ============================================================

TEST(CoverageJsonParser, SlatesDsJsonOriginalLabelCost)
{
  std::string json_text = R"(
  {
    "_label_cost": 1.0,
    "_outcomes": [
      {
        "_a": 0,
        "_p": 1.0,
        "_original_label_cost": 0.75
      }
    ],
    "c": {
      "_multi": [
        {"_slot_id": 0, "A": {"f": 1}}
      ],
      "_slots": [
        {"S": {"s": 1}}
      ]
    }
  })";
  auto vw = VW::initialize(vwtest::make_args("--slates", "--dsjson", "--chain_hash", "--no_stdin", "--quiet"));
  VW::parsers::json::decision_service_interaction interaction;
  auto examples = vwtest::parse_dsjson(*vw, json_text, &interaction);
  EXPECT_FLOAT_EQ(interaction.original_label_cost, 0.75f);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, SlatesDsJsonNoOutcomes)
{
  std::string json_text = R"(
  {
    "EventId": "no_outcomes_evt",
    "c": {
      "_multi": [
        {"_slot_id": 0, "A": {"f": 1}}
      ],
      "_slots": [
        {"S": {"s": 1}}
      ]
    }
  })";
  auto vw = VW::initialize(vwtest::make_args("--slates", "--dsjson", "--chain_hash", "--no_stdin", "--quiet"));
  VW::parsers::json::decision_service_interaction interaction;
  auto examples = vwtest::parse_dsjson(*vw, json_text, &interaction);
  EXPECT_EQ(examples.size(), 3);
  EXPECT_EQ(interaction.event_id, "no_outcomes_evt");
  // No outcomes -> no probabilities
  EXPECT_EQ(examples[2]->l.slates.probabilities.size(), 0);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, SlatesDsJsonWithPdrop)
{
  std::string json_text = R"(
  {
    "pdrop": 0.2,
    "c": {
      "_multi": [
        {"_slot_id": 0, "A": {"f": 1}}
      ],
      "_slots": [
        {"S": {"s": 1}}
      ]
    }
  })";
  auto vw = VW::initialize(vwtest::make_args("--slates", "--dsjson", "--chain_hash", "--no_stdin", "--quiet"));
  VW::parsers::json::decision_service_interaction interaction;
  auto examples = vwtest::parse_dsjson(*vw, json_text, &interaction);
  EXPECT_FLOAT_EQ(interaction.probability_of_drop, 0.2f);
  VW::finish_example(*vw, examples);
}

TEST(CoverageJsonParser, SlatesDsJsonDomParserBoolFeatures)
{
  // Test bool features in slates DOM parser
  std::string json_text = R"(
  {
    "c": {
      "User": {"active": true, "premium": false},
      "_multi": [],
      "_slots": []
    }
  })";
  auto slates_vw = VW::initialize(vwtest::make_args("--slates", "--dsjson", "--chain_hash", "--no_stdin", "--quiet"));
  auto examples = vwtest::parse_dsjson(*slates_vw, json_text);
  EXPECT_EQ(examples.size(), 1);
  // true adds a feature, false does not
  EXPECT_EQ(examples[0]->feature_space['U'].size(), 1);
  VW::finish_example(*slates_vw, examples);
}

TEST(CoverageJsonParser, SlatesDsJsonDomParserNumericArray)
{
  std::string json_text = R"(
  {
    "c": {
      "Features": {"arr": [10, 20, 30]},
      "_multi": [],
      "_slots": []
    }
  })";
  auto slates_vw = VW::initialize(vwtest::make_args("--slates", "--dsjson", "--chain_hash", "--no_stdin", "--quiet"));
  auto examples = vwtest::parse_dsjson(*slates_vw, json_text);
  EXPECT_EQ(examples.size(), 1);
  EXPECT_EQ(examples[0]->feature_space['a'].size(), 3);
  VW::finish_example(*slates_vw, examples);
}
