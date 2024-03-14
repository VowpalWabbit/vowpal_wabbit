// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "prototype_example.h"

#ifndef VWFB_BUILDERS_ONLY
#  include <gmock/gmock.h>
#  include <gtest/gtest.h>
#endif

namespace fb = VW::parsers::flatbuffer;
using namespace flatbuffers;

namespace vwtest
{

template <FeatureSerialization feature_serialization = FeatureSerialization::IncludeFeatureNames>
inline Offset<fb::ExampleRoot> create_example_root(
    FlatBufferBuilder& builder, VW::workspace& vw, const prototype_example_t& example)
{
  auto fb_example = example.create_flatbuffer<feature_serialization>(builder, vw);
  return fb::CreateExampleRoot(builder, fb::ExampleType_Example, fb_example.Union());
}

#ifndef VWFB_BUILDERS_ONLY
template <FeatureSerialization feature_serialization = FeatureSerialization::IncludeFeatureNames>
inline void verify_example_root(VW::workspace& vw, const fb::ExampleRoot* root, const prototype_example_t& expected)
{
  EXPECT_EQ(root->example_obj_type(), fb::ExampleType_Example);

  auto example = root->example_obj_as_Example();
  expected.verify<feature_serialization>(vw, example);
}

template <FeatureSerialization feature_serialization = FeatureSerialization::IncludeFeatureNames>
inline void verify_example_root(
    VW::workspace& vw, std::vector<VW::multi_ex> examples, const prototype_example_t& expected)
{
  EXPECT_EQ(examples.size(), 1);
  EXPECT_EQ(examples[0].size(), 1);

  expected.verify<feature_serialization>(vw, *(examples[0][0]));
}
#endif

template <FeatureSerialization feature_serialization = FeatureSerialization::IncludeFeatureNames>
inline Offset<fb::ExampleRoot> create_example_root(
    FlatBufferBuilder& builder, VW::workspace& vw, const prototype_multiexample_t& multiex)
{
  auto fb_multiex = multiex.create_flatbuffer<feature_serialization>(builder, vw);
  return fb::CreateExampleRoot(builder, fb::ExampleType_MultiExample, fb_multiex.Union());
}

#ifndef VWFB_BUILDERS_ONLY
template <FeatureSerialization feature_serialization = FeatureSerialization::IncludeFeatureNames>
inline void verify_example_root(
    VW::workspace& vw, const fb::ExampleRoot* root, const prototype_multiexample_t& expected)
{
  EXPECT_EQ(root->example_obj_type(), fb::ExampleType_MultiExample);

  auto multiex = root->example_obj_as_MultiExample();
  expected.verify<feature_serialization>(vw, multiex);
}

template <FeatureSerialization feature_serialization = FeatureSerialization::IncludeFeatureNames>
inline void verify_example_root(
    VW::workspace& vw, std::vector<VW::multi_ex> examples, const prototype_multiexample_t& expected)
{
  bool expecting_none = expected.examples.size() == 0;
  EXPECT_EQ(examples.size(), 1 - expecting_none);

  EXPECT_EQ(examples[0].size(), expected.examples.size());
  expected.verify<feature_serialization>(vw, examples[0]);
}
#endif

template <FeatureSerialization feature_serialization = FeatureSerialization::IncludeFeatureNames>
inline Offset<fb::ExampleRoot> create_example_root(
    FlatBufferBuilder& builder, VW::workspace& vw, const prototype_example_collection_t& collection)
{
  auto fb_collection = collection.create_flatbuffer<feature_serialization>(builder, vw);
  return fb::CreateExampleRoot(builder, fb::ExampleType_ExampleCollection, fb_collection.Union());
}

#ifndef VWFB_BUILDERS_ONLY
template <FeatureSerialization feature_serialization = FeatureSerialization::IncludeFeatureNames>
inline void verify_example_root(
    VW::workspace& vw, const fb::ExampleRoot* root, const prototype_example_collection_t& expected)
{
  EXPECT_EQ(root->example_obj_type(), fb::ExampleType_ExampleCollection);

  auto collection = root->example_obj_as_ExampleCollection();
  expected.verify<feature_serialization>(vw, collection);
}

template <FeatureSerialization feature_serialization = FeatureSerialization::IncludeFeatureNames>
inline void verify_example_root(
    VW::workspace& vw, std::vector<VW::multi_ex> examples, const prototype_example_collection_t& expected)
{
  // either we have a list of single examples (so a single multi_ex), or a list of multi_ex
  EXPECT_TRUE(expected.is_multiline || examples.size() == 1);

  if (expected.is_multiline)
  {
    EXPECT_EQ(examples.size(), expected.multi_examples.size());
    expected.verify_multiline<feature_serialization>(vw, examples);
  }
  else
  {
    EXPECT_EQ(examples[0].size(), expected.examples.size());
    expected.verify_singleline<feature_serialization>(vw, examples[0]);
  }
}
#endif

}  // namespace vwtest

#define USE_PROTOTYPE_MNEMONICS \
  USE_PROTOTYPE_MNEMONICS_EX;   \
  USE_PROTOTYPE_MNEMONICS_NS;
