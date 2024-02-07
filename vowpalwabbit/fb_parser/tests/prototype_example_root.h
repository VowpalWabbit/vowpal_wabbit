// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "prototype_example.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace fb = VW::parsers::flatbuffer;
using namespace flatbuffers;

namespace vwtest
{

template <bool include_feature_names = true>
inline Offset<fb::ExampleRoot> create_example_root(
    FlatBufferBuilder& builder, VW::workspace& vw, const prototype_example_t& example)
{
  auto fb_example = example.create_flatbuffer<include_feature_names>(builder, vw);
  return fb::CreateExampleRoot(builder, fb::ExampleType_Example, fb_example.Union());
}

template <bool expect_feature_names = true>
inline void verify_example_root(VW::workspace& vw, const fb::ExampleRoot* root, const prototype_example_t& expected)
{
  EXPECT_EQ(root->example_obj_type(), fb::ExampleType_Example);

  auto example = root->example_obj_as_Example();
  expected.verify<expect_feature_names>(vw, example);
}

template <bool expect_feature_names = true>
inline void verify_example_root(
    VW::workspace& vw, std::vector<VW::multi_ex> examples, const prototype_example_t& expected)
{
  EXPECT_EQ(examples.size(), 1);
  EXPECT_EQ(examples[0].size(), 1);
  expected.verify<expect_feature_names>(vw, examples[0]);
}

template <bool include_feature_names = true>
inline Offset<fb::ExampleRoot> create_example_root(
    FlatBufferBuilder& builder, VW::workspace& vw, const prototype_multiexample_t& multiex)
{
  auto fb_multiex = multiex.create_flatbuffer<include_feature_names>(builder, vw);
  return fb::CreateExampleRoot(builder, fb::ExampleType_MultiExample, fb_multiex.Union());
}

template <bool expect_feature_names = true>
inline void verify_example_root(
    VW::workspace& vw, const fb::ExampleRoot* root, const prototype_multiexample_t& expected)
{
  EXPECT_EQ(root->example_obj_type(), fb::ExampleType_MultiExample);

  auto multiex = root->example_obj_as_MultiExample();
  expected.verify<expect_feature_names>(vw, multiex);
}

template <bool expect_feature_names = true>
inline void verify_example_root(
    VW::workspace& vw, std::vector<VW::multi_ex> examples, const prototype_multiexample_t& expected)
{
  bool expecting_none = expected.examples.size() == 0;
  EXPECT_EQ(examples.size(), 1 - expecting_none);

  EXPECT_EQ(examples[0].size(), expected.examples.size());
  expected.verify<expect_feature_names>(vw, examples[0]);
}

template <bool include_feature_names = true>
inline Offset<fb::ExampleRoot> create_example_root(
    FlatBufferBuilder& builder, VW::workspace& vw, const prototype_example_collection_t& collection)
{
  auto fb_collection = collection.create_flatbuffer<include_feature_names>(builder, vw);
  return fb::CreateExampleRoot(builder, fb::ExampleType_ExampleCollection, fb_collection.Union());
}

template <bool expect_feature_names = true>
inline void verify_example_root(
    VW::workspace& vw, const fb::ExampleRoot* root, const prototype_example_collection_t& expected)
{
  EXPECT_EQ(root->example_obj_type(), fb::ExampleType_ExampleCollection);

  auto collection = root->example_obj_as_ExampleCollection();
  expected.verify<expect_feature_names>(vw, collection);
}

template <bool expect_feature_names = true>
inline void verify_example_root(
    VW::workspace& vw, std::vector<VW::multi_ex> examples, const prototype_example_collection_t& expected)
{
  // either we have a list of single examples (so a single multi_ex), or a list of multi_ex
  EXPECT_TRUE(expected.is_multiline || examples.size() == 1);

  if (expected.is_multiline)
  {
    EXPECT_EQ(examples.size(), expected.multi_examples.size());
    expected.verify_multiline<expect_feature_names>(vw, examples);
  }
  else
  {
    EXPECT_EQ(examples[0].size(), expected.examples.size());
    expected.verify_singleline<expect_feature_names>(vw, examples[0]);
  }
}

}  // namespace vwtest

#define USE_PROTOTYPE_MNEMONICS \
  USE_PROTOTYPE_MNEMONICS_EX;   \
  USE_PROTOTYPE_MNEMONICS_NS;
