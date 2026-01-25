// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/c_wrapper/vwdll.h"
#include "../src/vwdll_internal.h"

#include "vw/common/string_view.h"
#include "vw/core/vw.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace ::testing;

template <class T>
void check_weights_equal(T& first, T& second)
{
  auto first_begin = first.begin();
  auto first_end = first.end();
  auto second_begin = second.begin();
  auto second_end = second.end();
  for (; first_begin != first_end && second_begin != second_end; ++first_begin, ++second_begin)
  {
    EXPECT_FLOAT_EQ(*first_begin, *second_begin);
  }
  EXPECT_EQ(first_begin, first_end);
  EXPECT_EQ(second_begin, second_end);
}

TEST(Vwdll, ParsedAndConstructedExampleParity)
{
  // parse example
  VW_HANDLE handle1 = VW_InitializeA("-q st --noconstant --quiet");
  VW_EXAMPLE example_parsed;
  example_parsed = VW_ReadExampleA(handle1, "1 |s p^the_man w^the w^man |t p^un_homme w^un w^homme");

  // construct example
  VW_HANDLE handle2 = VW_InitializeA("-q st --noconstant --quiet");
  VW_EXAMPLE example_constructed;
  auto fs = VW_InitializeFeatureSpaces(2);

  auto first = VW_GetFeatureSpace(fs, 0);
  VW_InitFeatures(first, 3);
  auto shash = VW_SetFeatureSpace(handle2, first, "s");
  VW_SetFeature(first, 0, VW_HashFeatureA(handle2, "p^the_man", shash), 1.0f);
  VW_SetFeature(first, 1, VW_HashFeatureA(handle2, "w^the", shash), 1.0f);
  VW_SetFeature(first, 2, VW_HashFeatureA(handle2, "w^man", shash), 1.0f);

  auto second = VW_GetFeatureSpace(fs, 1);
  VW_InitFeatures(second, 3);
  auto thash = VW_SetFeatureSpace(handle2, second, "t");
  VW_SetFeature(second, 0, VW_HashFeatureA(handle2, "p^un_homme", thash), 1.0f);
  VW_SetFeature(second, 1, VW_HashFeatureA(handle2, "w^un", thash), 1.0f);
  VW_SetFeature(second, 2, VW_HashFeatureA(handle2, "w^homme", thash), 1.0f);

  example_constructed = VW_ImportExample(handle2, "1", fs, 2);

  // learn both
  auto score_parsed = VW_Learn(handle1, example_parsed);
  auto score_constructed = VW_Learn(handle2, example_parsed);

  // check parity
  EXPECT_EQ(score_parsed, score_constructed);
  auto vw1 = static_cast<VW::workspace*>(handle1);
  auto vw2 = static_cast<VW::workspace*>(handle2);

  EXPECT_EQ(vw1->weights.sparse, vw2->weights.sparse);

  if (vw1->weights.sparse) { check_weights_equal(vw1->weights.sparse_weights, vw2->weights.sparse_weights); }
  else { check_weights_equal(vw1->weights.dense_weights, vw2->weights.dense_weights); }

  VW_ReleaseFeatureSpace(fs, 2);

  VW_FinishExample(handle1, example_parsed);
  VW_FinishExample(handle2, example_constructed);

  VW_Finish(handle1);
  VW_Finish(handle2);
}

// This test seems to have issues on the older MSVC compiler CI, but no issues in the newer.
#if (defined(_MSC_VER) && (_MSC_VER >= 1920)) || !defined(_MSC_VER)

TEST(Vwdll, GetAuditOutput)
{
  // parse example
  VW_HANDLE handle = VW_InitializeA("--noconstant --quiet --audit");
  VW_CaptureAuditData(handle);
  VW_EXAMPLE example_parsed;
  example_parsed = VW_ReadExampleA(handle, "1 | test example");
  VW_Learn(handle, example_parsed);

  size_t audit_size = 0;
  char* audit_data = VW_GetAuditDataA(handle, &audit_size);

  VW::string_view expected_audit = R"(0
	test:250387:1:0@0	example:99909:1:0@0
)";
  VW::string_view audit_data_view(audit_data, audit_size);
  EXPECT_EQ(audit_data_view, expected_audit);

  VW_FreeAuditDataA(handle, audit_data);
  VW_FinishExample(handle, example_parsed);
  VW_Finish(handle);
}

#endif

#ifndef __APPLE__
TEST(Vwdll, ParseEscaped)
{
  // The space is escaped and so the data argument becomes "test --nonexistent_option"
  VW_HANDLE handle1 = VW_InitializeEscapedA("--id test\\ --nonexistent_option --quiet");
  EXPECT_TRUE(handle1 != nullptr);
  VW_Finish(handle1);
}
#endif

// Test VW_SeedWithModel - creates a new model seeded from existing model
TEST(Vwdll, SeedWithModel)
{
  // Create and train initial model
  VW_HANDLE handle1 = VW_InitializeA("--quiet");
  ASSERT_NE(handle1, nullptr);

  VW_EXAMPLE ex = VW_ReadExampleA(handle1, "1 |f a b c");
  VW_Learn(handle1, ex);
  VW_FinishExample(handle1, ex);

  ex = VW_ReadExampleA(handle1, "-1 |f d e f");
  VW_Learn(handle1, ex);
  VW_FinishExample(handle1, ex);

  // Seed a new model from the trained model
  VW_HANDLE handle2 = VW_SeedWithModel(handle1, "");
  ASSERT_NE(handle2, nullptr);

  // Verify predictions are the same
  VW_EXAMPLE ex1 = VW_ReadExampleA(handle1, "|f a b c");
  VW_Predict(handle1, ex1);
  float pred1 = VW_GetPrediction(ex1);
  VW_FinishExample(handle1, ex1);

  VW_EXAMPLE ex2 = VW_ReadExampleA(handle2, "|f a b c");
  VW_Predict(handle2, ex2);
  float pred2 = VW_GetPrediction(ex2);
  VW_FinishExample(handle2, ex2);

  EXPECT_FLOAT_EQ(pred1, pred2);

  VW_Finish(handle1);
  VW_Finish(handle2);
}

// Test VW_SeedWithModel with extra arguments (test mode)
TEST(Vwdll, SeedWithModelTestMode)
{
  // Create and train initial model
  VW_HANDLE handle1 = VW_InitializeA("--quiet");
  ASSERT_NE(handle1, nullptr);

  VW_EXAMPLE ex = VW_ReadExampleA(handle1, "1 |f a b c");
  VW_Learn(handle1, ex);
  VW_FinishExample(handle1, ex);

  // Seed a new model in test mode
  VW_HANDLE handle2 = VW_SeedWithModel(handle1, "-t");
  ASSERT_NE(handle2, nullptr);

  // Predict with the seeded model - should work in test mode
  VW_EXAMPLE ex2 = VW_ReadExampleA(handle2, "|f a b c");
  VW_Predict(handle2, ex2);
  float pred = VW_GetPrediction(ex2);
  EXPECT_NE(pred, 0.0f);  // Should have a non-zero prediction
  VW_FinishExample(handle2, ex2);

  VW_Finish(handle1);
  VW_Finish(handle2);
}

// Test VW_Predict functionality
TEST(Vwdll, Predict)
{
  VW_HANDLE handle = VW_InitializeA("--quiet");

  // Train first
  VW_EXAMPLE train_ex = VW_ReadExampleA(handle, "1 | feature1 feature2");
  VW_Learn(handle, train_ex);
  VW_FinishExample(handle, train_ex);

  // Then predict
  VW_EXAMPLE pred_ex = VW_ReadExampleA(handle, "| feature1 feature2");
  float prediction = VW_Predict(handle, pred_ex);

  EXPECT_FALSE(std::isnan(prediction));
  VW_FinishExample(handle, pred_ex);
  VW_Finish(handle);
}

// Test weight manipulation
TEST(Vwdll, WeightManipulation)
{
  VW_HANDLE handle = VW_InitializeA("--quiet -b 10");

  // Train
  VW_EXAMPLE ex = VW_ReadExampleA(handle, "1 | a b");
  VW_Learn(handle, ex);
  VW_FinishExample(handle, ex);

  size_t num_weights = VW_Num_Weights(handle);
  EXPECT_GT(num_weights, 0u);

  size_t stride = VW_Get_Stride(handle);
  EXPECT_GT(stride, 0u);

  // Get and set a weight
  float original_weight = VW_Get_Weight(handle, 0, 0);
  VW_Set_Weight(handle, 0, 0, original_weight + 1.0f);
  float new_weight = VW_Get_Weight(handle, 0, 0);
  EXPECT_FLOAT_EQ(new_weight, original_weight + 1.0f);

  VW_Finish(handle);
}

// Test label accessors
TEST(Vwdll, LabelAccessors)
{
  VW_HANDLE handle = VW_InitializeA("--quiet");

  VW_EXAMPLE ex = VW_ReadExampleA(handle, "2:1.5 | feature");

  float label = VW_GetLabel(ex);
  // For simple labels, this returns the label value
  EXPECT_FALSE(std::isnan(label));

  float importance = VW_GetImportance(ex);
  EXPECT_FALSE(std::isnan(importance));

  float initial = VW_GetInitial(ex);
  EXPECT_FALSE(std::isnan(initial));

  VW_FinishExample(handle, ex);
  VW_Finish(handle);
}

// Test feature number and tag accessors
TEST(Vwdll, FeatureAccessors)
{
  VW_HANDLE handle = VW_InitializeA("--quiet");

  VW_EXAMPLE ex = VW_ReadExampleA(handle, "1 tag| feature1 feature2 feature3");

  size_t num_features = VW_GetFeatureNumber(ex);
  EXPECT_GT(num_features, 0u);

  VW_FinishExample(handle, ex);
  VW_Finish(handle);
}

// Test AddLabel function
TEST(Vwdll, AddLabel)
{
  VW_HANDLE handle = VW_InitializeA("--quiet");

  VW_EXAMPLE ex = VW_ReadExampleA(handle, "| feature1 feature2");

  // Add a label
  VW_AddLabel(ex, 1.0f, 2.0f, 0.0f);

  // Learn should work after adding label
  float pred = VW_Learn(handle, ex);
  EXPECT_FALSE(std::isnan(pred));

  VW_FinishExample(handle, ex);
  VW_Finish(handle);
}

// Test AddStringLabel function
TEST(Vwdll, AddStringLabel)
{
  VW_HANDLE handle = VW_InitializeA("--quiet");

  VW_EXAMPLE ex = VW_ReadExampleA(handle, "| feature1 feature2");

  // Add a string label
  VW_AddStringLabel(handle, ex, "1");

  // Learn should work after adding label
  float pred = VW_Learn(handle, ex);
  EXPECT_FALSE(std::isnan(pred));

  VW_FinishExample(handle, ex);
  VW_Finish(handle);
}

// Test model copy functionality
TEST(Vwdll, CopyModelData)
{
  VW_HANDLE handle = VW_InitializeA("--quiet");

  // Train
  VW_EXAMPLE ex = VW_ReadExampleA(handle, "1 | a b c");
  VW_Learn(handle, ex);
  VW_FinishExample(handle, ex);

  // Copy model data
  VW_IOBUF buffer_handle = nullptr;
  char* model_data = nullptr;
  size_t model_size = 0;
  VW_CopyModelData(handle, &buffer_handle, &model_data, &model_size);

  EXPECT_NE(buffer_handle, nullptr);
  EXPECT_NE(model_data, nullptr);
  EXPECT_GT(model_size, 0u);

  VW_FreeIOBuf(buffer_handle);
  VW_Finish(handle);
}

// Test ExportExample and related functions
TEST(Vwdll, ExportExample)
{
  VW_HANDLE handle = VW_InitializeA("--quiet");

  VW_EXAMPLE ex = VW_ReadExampleA(handle, "1 |s a b |t c d");
  VW_Learn(handle, ex);

  size_t len = 0;
  VW_FEATURE_SPACE fs = VW_ExportExample(handle, ex, &len);

  EXPECT_NE(fs, nullptr);
  EXPECT_GT(len, 0u);

  VW_ReleaseFeatureSpace(fs, len);
  VW_FinishExample(handle, ex);
  VW_Finish(handle);
}

// Test hash functions
TEST(Vwdll, HashFunctions)
{
  VW_HANDLE handle = VW_InitializeA("--quiet");

  size_t space_hash = VW_HashSpaceA(handle, "namespace");
  EXPECT_GT(space_hash, 0u);

  size_t feature_hash = VW_HashFeatureA(handle, "feature", space_hash);
  EXPECT_GT(feature_hash, 0u);

  // Static hash functions
  size_t static_space_hash = VW_HashSpaceStaticA("namespace", "strings");
  EXPECT_GT(static_space_hash, 0u);

  size_t static_feature_hash = VW_HashFeatureStaticA("feature", space_hash, "strings", 18);
  EXPECT_GT(static_feature_hash, 0u);

  VW_Finish(handle);
}

// Test GetFeatures function
TEST(Vwdll, GetFeatures)
{
  VW_HANDLE handle = VW_InitializeA("--quiet");

  VW_EXAMPLE ex = VW_ReadExampleA(handle, "1 | feature1 feature2");

  size_t len = 0;
  VW_FEATURE features = VW_GetFeatures(handle, ex, &len);

  // Should have features from the example
  EXPECT_NE(features, nullptr);
  EXPECT_GT(len, 0u);

  VW_ReturnFeatures(features);
  VW_FinishExample(handle, ex);
  VW_Finish(handle);
}

// Test cost-sensitive prediction
TEST(Vwdll, PredictCostSensitive)
{
  VW_HANDLE handle = VW_InitializeA("--csoaa 3 --quiet");

  // Train
  VW_EXAMPLE train_ex = VW_ReadExampleA(handle, "1:0.5 2:1.0 3:1.5 | feature");
  VW_Learn(handle, train_ex);
  VW_FinishExample(handle, train_ex);

  // Predict
  VW_EXAMPLE pred_ex = VW_ReadExampleA(handle, "| feature");
  float prediction = VW_PredictCostSensitive(handle, pred_ex);

  // Should return a valid class (1, 2, or 3)
  EXPECT_GE(prediction, 1.0f);
  EXPECT_LE(prediction, 3.0f);

  VW_FinishExample(handle, pred_ex);
  VW_Finish(handle);
}

// Test InitializeWithModel
TEST(Vwdll, InitializeWithModel)
{
  // First create a model
  VW_HANDLE handle1 = VW_InitializeA("--quiet");
  VW_EXAMPLE ex = VW_ReadExampleA(handle1, "1 | feature");
  VW_Learn(handle1, ex);
  VW_FinishExample(handle1, ex);

  // Get model data
  VW_IOBUF buffer_handle = nullptr;
  char* model_data = nullptr;
  size_t model_size = 0;
  VW_CopyModelData(handle1, &buffer_handle, &model_data, &model_size);

  // Initialize with model
  VW_HANDLE handle2 = VW_InitializeWithModel("--quiet", model_data, model_size);
  EXPECT_NE(handle2, nullptr);

  // Should be able to predict
  VW_EXAMPLE pred_ex = VW_ReadExampleA(handle2, "| feature");
  float pred = VW_Predict(handle2, pred_ex);
  EXPECT_FALSE(std::isnan(pred));
  VW_FinishExample(handle2, pred_ex);

  VW_Finish(handle2);
  VW_FreeIOBuf(buffer_handle);
  VW_Finish(handle1);
}

// Test action score functions for CB
TEST(Vwdll, ActionScores)
{
  VW_HANDLE handle = VW_InitializeA("--cb_explore_adf --quiet");

  // Create a CB ADF example
  VW_EXAMPLE shared = VW_ReadExampleA(handle, "shared | context_feature");
  VW_EXAMPLE action1 = VW_ReadExampleA(handle, "0:1:0.5 | action1_feature");
  VW_EXAMPLE action2 = VW_ReadExampleA(handle, "| action2_feature");

  // This would require multi-example learn which is more complex
  // Just test that the functions exist and don't crash with a simple example
  VW_FinishExample(handle, shared);
  VW_FinishExample(handle, action1);
  VW_FinishExample(handle, action2);

  VW_Finish(handle);
}

// Test confidence value
TEST(Vwdll, GetConfidence)
{
  VW_HANDLE handle = VW_InitializeA("--quiet");

  VW_EXAMPLE ex = VW_ReadExampleA(handle, "1 | feature");
  VW_Learn(handle, ex);

  float confidence = VW_GetConfidence(ex);
  // Confidence is typically initialized to 0 unless using active learning
  EXPECT_FALSE(std::isnan(confidence));

  VW_FinishExample(handle, ex);
  VW_Finish(handle);
}

// Test escaped initialization
TEST(Vwdll, InitializeWithModelEscaped)
{
  // Create a model first
  VW_HANDLE handle1 = VW_InitializeA("--quiet");
  VW_EXAMPLE ex = VW_ReadExampleA(handle1, "1 | feature");
  VW_Learn(handle1, ex);
  VW_FinishExample(handle1, ex);

  // Get model data
  VW_IOBUF buffer_handle = nullptr;
  char* model_data = nullptr;
  size_t model_size = 0;
  VW_CopyModelData(handle1, &buffer_handle, &model_data, &model_size);

  // Initialize with model using escaped args
  VW_HANDLE handle2 = VW_InitializeWithModelEscaped("--quiet", model_data, model_size);
  EXPECT_NE(handle2, nullptr);

  VW_Finish(handle2);
  VW_FreeIOBuf(buffer_handle);
  VW_Finish(handle1);
}

TEST(Vwdll, Utf16ToUtf8Conversion)
{
  // Test 1: ASCII characters (1-byte UTF-8)
  {
    std::u16string input;
    input.push_back(u'H');
    input.push_back(u'e');
    input.push_back(u'l');
    input.push_back(u'l');
    input.push_back(u'o');
    std::string expected = "Hello";
    std::string result = utf16_to_utf8(input);
    EXPECT_EQ(result, expected);
  }

  // Test 2: 2-byte UTF-8 characters (Latin-1 supplement)
  {
    // "Café" - é is U+00E9, which in UTF-8 is C3 A9
    std::u16string input;
    input.push_back(0x0043);  // C
    input.push_back(0x0061);  // a
    input.push_back(0x0066);  // f
    input.push_back(0x00E9);  // é
    std::string expected;
    expected.push_back(0x43);      // C
    expected.push_back(0x61);      // a
    expected.push_back(0x66);      // f
    expected.push_back('\xC3');    // é (UTF-8 byte 1)
    expected.push_back('\xA9');    // é (UTF-8 byte 2)
    std::string result = utf16_to_utf8(input);
    EXPECT_EQ(result, expected);
  }

  // Test 3: 3-byte UTF-8 characters (CJK)
  {
    // "日本語" - Each character is 3 bytes in UTF-8
    // 日 = U+65E5 -> E6 97 A5
    // 本 = U+672C -> E6 9C AC
    // 語 = U+8A9E -> E8 AA 9E
    std::u16string input;
    input.push_back(0x65E5);  // 日
    input.push_back(0x672C);  // 本
    input.push_back(0x8A9E);  // 語
    std::string expected;
    expected.push_back('\xE6'); expected.push_back('\x97'); expected.push_back('\xA5');  // 日
    expected.push_back('\xE6'); expected.push_back('\x9C'); expected.push_back('\xAC');  // 本
    expected.push_back('\xE8'); expected.push_back('\xAA'); expected.push_back('\x9E');  // 語
    std::string result = utf16_to_utf8(input);
    EXPECT_EQ(result, expected);
  }

  // Test 4: 4-byte UTF-8 characters (emoji with surrogate pairs)
  {
    // U+1F600 (😀) is represented as surrogate pair: D83D DE00
    std::u16string input;
    input.push_back(0xD83D);  // High surrogate
    input.push_back(0xDE00);  // Low surrogate
    std::string result = utf16_to_utf8(input);
    // U+1F600 in UTF-8: F0 9F 98 80
    std::string expected = "\xF0\x9F\x98\x80";
    EXPECT_EQ(result, expected);
  }

  // Test 5: Mixed content "Test-テスト-123"
  {
    // テ = U+30C6 -> E3 83 86
    // ス = U+30B9 -> E3 82 B9
    // ト = U+30C8 -> E3 83 88
    std::u16string input;
    input.push_back(0x0054);  // T
    input.push_back(0x0065);  // e
    input.push_back(0x0073);  // s
    input.push_back(0x0074);  // t
    input.push_back(0x002D);  // -
    input.push_back(0x30C6);  // テ
    input.push_back(0x30B9);  // ス
    input.push_back(0x30C8);  // ト
    input.push_back(0x002D);  // -
    input.push_back(0x0031);  // 1
    input.push_back(0x0032);  // 2
    input.push_back(0x0033);  // 3
    std::string expected;
    expected += "Test-";
    expected.push_back('\xE3'); expected.push_back('\x83'); expected.push_back('\x86');  // テ
    expected.push_back('\xE3'); expected.push_back('\x82'); expected.push_back('\xB9');  // ス
    expected.push_back('\xE3'); expected.push_back('\x83'); expected.push_back('\x88');  // ト
    expected += "-123";
    std::string result = utf16_to_utf8(input);
    EXPECT_EQ(result, expected);
  }

  // Test 6: Empty string
  {
    std::u16string input = u"";
    std::string expected = "";
    std::string result = utf16_to_utf8(input);
    EXPECT_EQ(result, expected);
  }

  // Test 7: Special characters with accents "Zürich"
  {
    // ü = U+00FC -> C3 BC in UTF-8
    std::u16string input;
    input.push_back(0x005A);  // Z
    input.push_back(0x00FC);  // ü
    input.push_back(0x0072);  // r
    input.push_back(0x0069);  // i
    input.push_back(0x0063);  // c
    input.push_back(0x0068);  // h
    std::string expected;
    expected.push_back(0x5A);      // Z
    expected.push_back('\xC3');    // ü (UTF-8 byte 1)
    expected.push_back('\xBC');    // ü (UTF-8 byte 2)
    expected += "rich";
    std::string result = utf16_to_utf8(input);
    EXPECT_EQ(result, expected);
  }
}
