// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/constant.h"
#include "vw/core/feature_group.h"
#include "vw/core/parse_example.h"
#include "vw/core/vw.h"
#include "vw/fb_parser/parse_example_flatbuffer.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>
#include <vector>

flatbuffers::Offset<void> get_label(flatbuffers::FlatBufferBuilder& builder, VW::parsers::flatbuffer::Label label_type)
{
  flatbuffers::Offset<void> label;
  if (label_type == VW::parsers::flatbuffer::Label_SimpleLabel)
    label = VW::parsers::flatbuffer::CreateSimpleLabel(builder, 0.0, 1.0).Union();

  return label;
}

flatbuffers::Offset<VW::parsers::flatbuffer::ExampleRoot> sample_flatbuffer_collection(
    flatbuffers::FlatBufferBuilder& builder, VW::parsers::flatbuffer::Label label_type)
{
  std::vector<flatbuffers::Offset<VW::parsers::flatbuffer::Example>> examples;
  std::vector<flatbuffers::Offset<VW::parsers::flatbuffer::Namespace>> namespaces;
  // std::vector<flatbuffers::Offset<VW::parsers::flatbuffer::Feature>> fts;

  auto label = get_label(builder, label_type);

  // fts.push_back(VW::parsers::flatbuffer::CreateFeatureDirect(builder, "hello", 2.23f, constant));
  std::vector<flatbuffers::Offset<flatbuffers::String>> feature_names = {builder.CreateString("hello")};
  std::vector<float> feature_values = {2.23f};
  std::vector<uint64_t> feature_hashes;// = {constant};
  namespaces.push_back(VW::parsers::flatbuffer::CreateNamespaceDirect(builder, nullptr, constant_namespace, 128, &feature_names, &feature_values, &feature_hashes));
  // namespaces.push_back(VW::parsers::flatbuffer::CreateNamespaceDirect(builder, nullptr, 128, constant_namespace, "hello", 2.23f, constant, 128));
  examples.push_back(VW::parsers::flatbuffer::CreateExampleDirect(builder, &namespaces, label_type, label));

  auto eg_collection = VW::parsers::flatbuffer::CreateExampleCollectionDirect(builder, &examples);
  return CreateExampleRoot(builder, VW::parsers::flatbuffer::ExampleType_ExampleCollection, eg_collection.Union());
}

flatbuffers::Offset<VW::parsers::flatbuffer::ExampleRoot> sample_flatbuffer_audit(
    flatbuffers::FlatBufferBuilder& builder, VW::parsers::flatbuffer::Label label_type)
{
  std::vector<flatbuffers::Offset<VW::parsers::flatbuffer::Namespace>> namespaces;
  // std::vector<flatbuffers::Offset<VW::parsers::flatbuffer::Feature>> fts;
  auto label = get_label(builder, label_type);

  // fts.push_back(VW::parsers::flatbuffer::CreateFeatureDirect(builder, "hello", 2.23f, constant));
   const std::vector<flatbuffers::Offset<flatbuffers::String>> feature_names = {builder.CreateString("hello")}; //auto temp_fn= {builder.CreateString("hello")}; 
   const std::vector<float>  feature_values = {2.23f};
   const std::vector<uint64_t> feature_hashes;// = {constant}; 
  namespaces.push_back(VW::parsers::flatbuffer::CreateNamespaceDirect(builder, nullptr, constant_namespace, 128, &feature_names, &feature_values, nullptr));
  auto example = VW::parsers::flatbuffer::CreateExampleDirect(builder, &namespaces, label_type, label);

  return CreateExampleRoot(builder, VW::parsers::flatbuffer::ExampleType_Example, example.Union());
}

flatbuffers::Offset<VW::parsers::flatbuffer::ExampleRoot> sample_flatbuffer_no_audit(
    flatbuffers::FlatBufferBuilder& builder, VW::parsers::flatbuffer::Label label_type)
{
  std::vector<flatbuffers::Offset<VW::parsers::flatbuffer::Namespace>> namespaces;
  // std::vector<flatbuffers::Offset<VW::parsers::flatbuffer::Feature>> fts;
  auto label = get_label(builder, label_type);

  // fts.push_back(VW::parsers::flatbuffer::CreateFeatureDirect(builder, "hello", 2.23f, constant));
  //  const std::vector<flatbuffers::Offset<flatbuffers::String>> feature_names = {builder.CreateString("hello")}; //auto temp_fn= {builder.CreateString("hello")}; 
   const std::vector<float>  feature_values = {2.23f};
   const std::vector<uint64_t> feature_hashes = {constant}; 
  namespaces.push_back(VW::parsers::flatbuffer::CreateNamespaceDirect(builder, nullptr, constant_namespace, 128, nullptr, &feature_values, &feature_hashes));
  auto example = VW::parsers::flatbuffer::CreateExampleDirect(builder, &namespaces, label_type, label);

  return CreateExampleRoot(builder, VW::parsers::flatbuffer::ExampleType_Example, example.Union());
}

flatbuffers::Offset<VW::parsers::flatbuffer::ExampleRoot> sample_flatbuffer_features_collection(
    flatbuffers::FlatBufferBuilder& builder, VW::parsers::flatbuffer::Label label_type)
{
  std::vector<flatbuffers::Offset<VW::parsers::flatbuffer::Example>> examples;
  std::vector<flatbuffers::Offset<VW::parsers::flatbuffer::Namespace>> namespaces;
  // std::vector<flatbuffers::Offset<VW::parsers::flatbuffer::Feature>> fts;

  auto label = get_label(builder, label_type);

  std::vector<flatbuffers::Offset<flatbuffers::String>> feature_names = {builder.CreateString("hello"), builder.CreateString("world")};
  std::vector<float> feature_values = {2.23f, 5.0f};
  std::vector<uint64_t> feature_hashes = {constant, constant};
  namespaces.push_back(VW::parsers::flatbuffer::CreateNamespaceDirect(builder, nullptr, constant_namespace, 128, &feature_names, &feature_values, &feature_hashes));
  // namespaces.push_back(VW::parsers::flatbuffer::CreateNamespaceDirect(builder, nullptr, 128, constant_namespace, "hello", 2.23f, constant, 128));
  examples.push_back(VW::parsers::flatbuffer::CreateExampleDirect(builder, &namespaces, label_type, label));

  auto eg_collection = VW::parsers::flatbuffer::CreateExampleCollectionDirect(builder, &examples);
  return CreateExampleRoot(builder, VW::parsers::flatbuffer::ExampleType_ExampleCollection, eg_collection.Union());
}


TEST(flatbuffer_parser_tests, test_flatbuffer_standalone_example_audit)
{
  //Testcase where user would provide feature names and feature values (no feature hashes)
  auto all = VW::initialize("--no_stdin --quiet --flatbuffer --audit", nullptr, false, nullptr, nullptr);

  flatbuffers::FlatBufferBuilder builder;

  auto root = sample_flatbuffer_audit(builder, VW::parsers::flatbuffer::Label_SimpleLabel);
  builder.FinishSizePrefixed(root);

  uint8_t* buf = builder.GetBufferPointer();

  VW::multi_ex examples;
  examples.push_back(&VW::get_unused_example(all));
  io_buf unused_buffer;
  all->flat_converter->parse_examples(all, unused_buffer, examples, buf);

  auto example = all->flat_converter->data()->example_obj_as_Example();
  EXPECT_EQ(example->namespaces()->size(), 1);
  EXPECT_EQ(example->namespaces()->Get(0)->feature_names()->size(), 1);
  EXPECT_FLOAT_EQ(example->label_as_SimpleLabel()->label(), 0.0);
  EXPECT_FLOAT_EQ(example->label_as_SimpleLabel()->weight(), 1.0);
  EXPECT_EQ(example->namespaces()->Get(0)->hash(), constant_namespace);
  EXPECT_EQ(example->namespaces()->Get(0)->full_hash(), constant_namespace);
  EXPECT_STREQ(example->namespaces()->Get(0)->feature_names()->Get(0)->c_str(), "hello");
  // EXPECT_EQ(example->namespaces()->Get(0)->feature_hashes()->Get(0), constant);
  EXPECT_FLOAT_EQ(example->namespaces()->Get(0)->feature_values()->Get(0), 2.23);

  // Check vw example
  EXPECT_EQ(examples.size(), 1);
  EXPECT_FLOAT_EQ(examples[0]->l.simple.label, 0.f);
  const auto& red_features = examples[0]->_reduction_features.template get<simple_label_reduction_features>();
  EXPECT_FLOAT_EQ(red_features.weight, 1.f);

  EXPECT_EQ(examples[0]->indices[0], constant_namespace);
  EXPECT_FLOAT_EQ(examples[0]->feature_space[examples[0]->indices[0]].values[0], 2.23f);
  EXPECT_EQ(examples[0]->feature_space[examples[0]->indices[0]].namespace_extents.size(), 1);
  EXPECT_EQ(examples[0]->feature_space[examples[0]->indices[0]].namespace_extents[0],
      (VW::namespace_extent{0, 1, constant_namespace}));

  VW::finish_example(*all, *examples[0]);
  VW::finish(*all);
}

TEST(flatbuffer_parser_tests, test_flatbuffer_standalone_example_no_audit)
{
  //Testcase where user would provide feature names and feature values (no feature hashes)
  auto all = VW::initialize("--no_stdin --quiet --flatbuffer", nullptr, false, nullptr, nullptr);

  flatbuffers::FlatBufferBuilder builder;

  auto root = sample_flatbuffer_no_audit(builder, VW::parsers::flatbuffer::Label_SimpleLabel);
  builder.FinishSizePrefixed(root);

  uint8_t* buf = builder.GetBufferPointer();

  VW::multi_ex examples;
  examples.push_back(&VW::get_unused_example(all));
  io_buf unused_buffer;
  all->flat_converter->parse_examples(all, unused_buffer, examples, buf);

  auto example = all->flat_converter->data()->example_obj_as_Example();
  EXPECT_EQ(example->namespaces()->size(), 1);
  // EXPECT_EQ(example->namespaces()->Get(0)->feature_names()->size(), 0);
  EXPECT_FLOAT_EQ(example->label_as_SimpleLabel()->label(), 0.0);
  EXPECT_FLOAT_EQ(example->label_as_SimpleLabel()->weight(), 1.0);
  EXPECT_EQ(example->namespaces()->Get(0)->hash(), constant_namespace);
  EXPECT_EQ(example->namespaces()->Get(0)->full_hash(), constant_namespace);
  // EXPECT_STREQ(example->namespaces()->Get(0)->feature_names()->Get(0)->c_str(), "hello");
  EXPECT_EQ(example->namespaces()->Get(0)->feature_names(), nullptr);
  EXPECT_EQ(example->namespaces()->Get(0)->feature_hashes()->Get(0), constant);
  EXPECT_FLOAT_EQ(example->namespaces()->Get(0)->feature_values()->Get(0), 2.23);

  // Check vw example
  EXPECT_EQ(examples.size(), 1);
  EXPECT_FLOAT_EQ(examples[0]->l.simple.label, 0.f);
  const auto& red_features = examples[0]->_reduction_features.template get<simple_label_reduction_features>();
  EXPECT_FLOAT_EQ(red_features.weight, 1.f);

  EXPECT_EQ(examples[0]->indices[0], constant_namespace);
  EXPECT_FLOAT_EQ(examples[0]->feature_space[examples[0]->indices[0]].values[0], 2.23f);
  EXPECT_EQ(examples[0]->feature_space[examples[0]->indices[0]].namespace_extents.size(), 1);
  EXPECT_EQ(examples[0]->feature_space[examples[0]->indices[0]].namespace_extents[0],
      (VW::namespace_extent{0, 1, constant_namespace}));

  VW::finish_example(*all, *examples[0]);
  VW::finish(*all);
}

// TEST(flatbuffer_parser_tests, test_flatbuffer_collection)
// {
//   auto all = VW::initialize("--no_stdin --quiet --flatbuffer", nullptr, false, nullptr, nullptr);

//   flatbuffers::FlatBufferBuilder builder;

//   auto root = sample_flatbuffer_collection(builder, VW::parsers::flatbuffer::Label_SimpleLabel);
//   builder.FinishSizePrefixed(root);

//   uint8_t* buf = builder.GetBufferPointer();
//   int size = builder.GetSize();

//   VW::multi_ex examples;
//   examples.push_back(&VW::get_unused_example(all));
//   io_buf unused_buffer;
//   all->flat_converter->parse_examples(all, unused_buffer, examples, buf);

//   auto collection_examples = all->flat_converter->data()->example_obj_as_ExampleCollection()->examples();
//   EXPECT_EQ(collection_examples->size(), 1);
//   EXPECT_EQ(collection_examples->Get(0)->namespaces()->size(), 1);
//   EXPECT_EQ(collection_examples->Get(0)->namespaces()->Get(0)->feature_names()->size(), 1);
//   EXPECT_FLOAT_EQ(collection_examples->Get(0)->label_as_SimpleLabel()->label(), 0.0);
//   EXPECT_FLOAT_EQ(collection_examples->Get(0)->label_as_SimpleLabel()->weight(), 1.0);
//   EXPECT_EQ(collection_examples->Get(0)->namespaces()->Get(0)->hash(), constant_namespace);
//   EXPECT_EQ(collection_examples->Get(0)->namespaces()->Get(0)->full_hash(), constant_namespace);
//   EXPECT_STREQ(collection_examples->Get(0)->namespaces()->Get(0)->feature_names()->Get(0)->c_str(), "hello");
//   EXPECT_EQ(collection_examples->Get(0)->namespaces()->Get(0)->feature_hashes()->Get(0), constant);
//   EXPECT_FLOAT_EQ(collection_examples->Get(0)->namespaces()->Get(0)->feature_values()->Get(0), 2.23);

//   // check vw example
//   EXPECT_EQ(examples.size(), 1);
//   EXPECT_FLOAT_EQ(examples[0]->l.simple.label, 0.f);
//   const auto& red_features = examples[0]->_reduction_features.template get<simple_label_reduction_features>();
//   EXPECT_FLOAT_EQ(red_features.weight, 1.f);

//   EXPECT_EQ(examples[0]->indices[0], constant_namespace);
//   EXPECT_FLOAT_EQ(examples[0]->feature_space[examples[0]->indices[0]].values[0], 2.23f);
//   EXPECT_EQ(examples[0]->feature_space[examples[0]->indices[0]].namespace_extents.size(), 1);
//   EXPECT_EQ(examples[0]->feature_space[examples[0]->indices[0]].namespace_extents[0],
//       (VW::namespace_extent{0, 1, constant_namespace}));

//   VW::finish_example(*all, *examples[0]);
//   VW::finish(*all);
// }


// TEST(flatbuffer_parser_tests, test_flatbuffer_features_collection)
// {
//   auto all = VW::initialize("--no_stdin --quiet --flatbuffer", nullptr, false, nullptr, nullptr);

//   flatbuffers::FlatBufferBuilder builder;

//   auto root = sample_flatbuffer_features_collection(builder, VW::parsers::flatbuffer::Label_SimpleLabel);
//   builder.FinishSizePrefixed(root);

//   uint8_t* buf = builder.GetBufferPointer();
//   int size = builder.GetSize();

//   VW::multi_ex examples;
//   examples.push_back(&VW::get_unused_example(all));
//   io_buf unused_buffer;
//   all->flat_converter->parse_examples(all, unused_buffer, examples, buf);

//   auto collection_examples = all->flat_converter->data()->example_obj_as_ExampleCollection()->examples();
//   EXPECT_EQ(collection_examples->size(), 1);
//   EXPECT_EQ(collection_examples->Get(0)->namespaces()->size(), 1);
//   EXPECT_EQ(collection_examples->Get(0)->namespaces()->Get(0)->feature_names()->size(), 2);
//   EXPECT_FLOAT_EQ(collection_examples->Get(0)->label_as_SimpleLabel()->label(), 0.0);
//   EXPECT_FLOAT_EQ(collection_examples->Get(0)->label_as_SimpleLabel()->weight(), 1.0);
//   EXPECT_EQ(collection_examples->Get(0)->namespaces()->Get(0)->hash(), constant_namespace);
//   EXPECT_EQ(collection_examples->Get(0)->namespaces()->Get(0)->full_hash(), constant_namespace);
//   EXPECT_STREQ(collection_examples->Get(0)->namespaces()->Get(0)->feature_names()->Get(0)->c_str(), "hello");
//   EXPECT_STREQ(collection_examples->Get(0)->namespaces()->Get(0)->feature_names()->Get(1)->c_str(), "world");
//   // EXPECT_STREQ(collection_examples->Get(0)->namespaces()->Get(0)->feature_names()->Get(1)->c_str(), nullptr);
//   EXPECT_EQ(collection_examples->Get(0)->namespaces()->Get(0)->feature_hashes()->Get(0), constant);
//   EXPECT_EQ(collection_examples->Get(0)->namespaces()->Get(0)->feature_hashes()->Get(1), constant);
//   EXPECT_FLOAT_EQ(collection_examples->Get(0)->namespaces()->Get(0)->feature_values()->Get(0), 2.23);
//   EXPECT_FLOAT_EQ(collection_examples->Get(0)->namespaces()->Get(0)->feature_values()->Get(1), 5.0);

//   // check vw example
//   EXPECT_EQ(examples.size(), 1);
//   EXPECT_FLOAT_EQ(examples[0]->l.simple.label, 0.f);
//   const auto& red_features = examples[0]->_reduction_features.template get<simple_label_reduction_features>();
//   EXPECT_FLOAT_EQ(red_features.weight, 1.f);

//   EXPECT_EQ(examples[0]->indices[0], constant_namespace);
//   EXPECT_FLOAT_EQ(examples[0]->feature_space[examples[0]->indices[0]].values[0], 2.23f);
//   EXPECT_EQ(examples[0]->feature_space[examples[0]->indices[0]].namespace_extents.size(), 1);
//   EXPECT_EQ(examples[0]->feature_space[examples[0]->indices[0]].namespace_extents[0],
//       (VW::namespace_extent{0, 2, constant_namespace}));

//   VW::finish_example(*all, *examples[0]);
//   VW::finish(*all);
// }