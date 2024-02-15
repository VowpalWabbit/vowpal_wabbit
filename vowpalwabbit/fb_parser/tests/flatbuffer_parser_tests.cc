// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "example_data_generator.h"
#include "prototype_example.h"
#include "prototype_example_root.h"
#include "prototype_label.h"
#include "prototype_namespace.h"
#include "vw/common/future_compat.h"
#include "vw/common/string_view.h"
#include "vw/core/api_status.h"
#include "vw/core/constant.h"
#include "vw/core/error_constants.h"
#include "vw/core/example.h"
#include "vw/core/feature_group.h"
#include "vw/core/learner.h"
#include "vw/core/vw.h"
#include "vw/fb_parser/parse_example_flatbuffer.h"
#include "vw/test_common/test_common.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>
#include <vector>

USE_PROTOTYPE_MNEMONICS

namespace fb = VW::parsers::flatbuffer;
using namespace flatbuffers;
using namespace vwtest;

flatbuffers::Offset<void> get_label(flatbuffers::FlatBufferBuilder& builder, VW::parsers::flatbuffer::Label label_type)
{
  flatbuffers::Offset<void> label;
  if (label_type == VW::parsers::flatbuffer::Label_SimpleLabel)
  {
    label = VW::parsers::flatbuffer::CreateSimpleLabel(builder, 0.0, 1.0).Union();
  }

  return label;
}

flatbuffers::Offset<VW::parsers::flatbuffer::ExampleRoot> sample_flatbuffer_audit(
    flatbuffers::FlatBufferBuilder& builder, VW::parsers::flatbuffer::Label label_type)
{
  std::vector<flatbuffers::Offset<VW::parsers::flatbuffer::Namespace>> namespaces;
  auto label = get_label(builder, label_type);
  const std::vector<flatbuffers::Offset<flatbuffers::String>> feature_names = {
      builder.CreateString("hello")};  // auto temp_fn= {builder.CreateString("hello")};
  const std::vector<float> feature_values = {2.23f};
  const std::vector<uint64_t> feature_hashes;  // = {VW::details::CONSTANT};
  namespaces.push_back(VW::parsers::flatbuffer::CreateNamespaceDirect(
      builder, nullptr, VW::details::CONSTANT_NAMESPACE, 128, &feature_names, &feature_values, nullptr));
  auto example = VW::parsers::flatbuffer::CreateExampleDirect(builder, &namespaces, label_type, label);

  return CreateExampleRoot(builder, VW::parsers::flatbuffer::ExampleType_Example, example.Union());
}

flatbuffers::Offset<VW::parsers::flatbuffer::ExampleRoot> sample_flatbuffer_no_audit(
    flatbuffers::FlatBufferBuilder& builder, VW::parsers::flatbuffer::Label label_type)
{
  std::vector<flatbuffers::Offset<VW::parsers::flatbuffer::Namespace>> namespaces;
  auto label = get_label(builder, label_type);
  const std::vector<float> feature_values = {2.23f};
  const std::vector<uint64_t> feature_hashes = {VW::details::CONSTANT};
  namespaces.push_back(VW::parsers::flatbuffer::CreateNamespaceDirect(
      builder, nullptr, VW::details::CONSTANT_NAMESPACE, 128, nullptr, &feature_values, &feature_hashes));
  auto example = VW::parsers::flatbuffer::CreateExampleDirect(builder, &namespaces, label_type, label);

  return CreateExampleRoot(builder, VW::parsers::flatbuffer::ExampleType_Example, example.Union());
}

flatbuffers::Offset<VW::parsers::flatbuffer::ExampleRoot> sample_flatbuffer_collection(
    flatbuffers::FlatBufferBuilder& builder, VW::parsers::flatbuffer::Label label_type)
{
  std::vector<flatbuffers::Offset<VW::parsers::flatbuffer::Example>> examples;
  std::vector<flatbuffers::Offset<VW::parsers::flatbuffer::Namespace>> namespaces;

  auto label = get_label(builder, label_type);

  std::vector<flatbuffers::Offset<flatbuffers::String>> feature_names = {builder.CreateString("hello")};
  std::vector<float> feature_values = {2.23f};
  std::vector<uint64_t> feature_hashes = {VW::details::CONSTANT};
  namespaces.push_back(VW::parsers::flatbuffer::CreateNamespaceDirect(
      builder, nullptr, VW::details::CONSTANT_NAMESPACE, 128, &feature_names, &feature_values, &feature_hashes));
  examples.push_back(VW::parsers::flatbuffer::CreateExampleDirect(builder, &namespaces, label_type, label));

  auto eg_collection = VW::parsers::flatbuffer::CreateExampleCollectionDirect(builder, &examples);
  return CreateExampleRoot(builder, VW::parsers::flatbuffer::ExampleType_ExampleCollection, eg_collection.Union());
}

flatbuffers::Offset<VW::parsers::flatbuffer::ExampleRoot> sample_flatbuffer_error_code(
    flatbuffers::FlatBufferBuilder& builder, VW::parsers::flatbuffer::Label label_type)
{
  std::vector<flatbuffers::Offset<VW::parsers::flatbuffer::Namespace>> namespaces;
  auto label = get_label(builder, label_type);

  const std::vector<flatbuffers::Offset<flatbuffers::String>>
      feature_names;  // = {builder.CreateString("hello")}; //auto temp_fn= {builder.CreateString("hello")};
  const std::vector<float> feature_values = {2.23f};
  const std::vector<uint64_t> feature_hashes;  // = {VW::details::CONSTANT};
  namespaces.push_back(VW::parsers::flatbuffer::CreateNamespaceDirect(
      builder, nullptr, VW::details::CONSTANT_NAMESPACE, 128, nullptr, &feature_values, nullptr));
  auto example = VW::parsers::flatbuffer::CreateExampleDirect(builder, &namespaces, label_type, label);

  return CreateExampleRoot(builder, VW::parsers::flatbuffer::ExampleType_Example, example.Union());
}

TEST(FlatbufferParser, SingleExample_SimpleLabel_FeatureNames)
{
  // Testcase where user would provide feature names and feature values (no feature hashes)
  auto all = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--flatbuffer"));

  flatbuffers::FlatBufferBuilder builder;

  auto root = sample_flatbuffer_audit(builder, VW::parsers::flatbuffer::Label_SimpleLabel);
  builder.FinishSizePrefixed(root);

  uint8_t* buf = builder.GetBufferPointer();

  VW::multi_ex examples;
  examples.push_back(&VW::get_unused_example(all.get()));
  VW::io_buf unused_buffer;
  all->parser_runtime.flat_converter->parse_examples(all.get(), unused_buffer, examples, buf);

  auto example = all->parser_runtime.flat_converter->data()->example_obj_as_Example();
  EXPECT_EQ(example->namespaces()->size(), 1);
  EXPECT_EQ(example->namespaces()->Get(0)->feature_names()->size(), 1);
  EXPECT_FLOAT_EQ(example->label_as_SimpleLabel()->label(), 0.0);
  EXPECT_FLOAT_EQ(example->label_as_SimpleLabel()->weight(), 1.0);
  EXPECT_EQ(example->namespaces()->Get(0)->hash(), VW::details::CONSTANT_NAMESPACE);
  EXPECT_EQ(example->namespaces()->Get(0)->full_hash(), VW::details::CONSTANT_NAMESPACE);
  EXPECT_STREQ(example->namespaces()->Get(0)->feature_names()->Get(0)->c_str(), "hello");
  // EXPECT_EQ(example->namespaces()->Get(0)->feature_hashes()->Get(0), VW::details::CONSTANT);
  EXPECT_FLOAT_EQ(example->namespaces()->Get(0)->feature_values()->Get(0), 2.23);

  // Check vw example
  EXPECT_EQ(examples.size(), 1);
  EXPECT_FLOAT_EQ(examples[0]->l.simple.label, 0.f);
  const auto& red_features = examples[0]->ex_reduction_features.template get<VW::simple_label_reduction_features>();
  EXPECT_FLOAT_EQ(red_features.weight, 1.f);

  EXPECT_EQ(examples[0]->indices[0], VW::details::CONSTANT_NAMESPACE);
  EXPECT_FLOAT_EQ(examples[0]->feature_space[examples[0]->indices[0]].values[0], 2.23f);
  EXPECT_EQ(examples[0]->feature_space[examples[0]->indices[0]].namespace_extents.size(), 1);
  EXPECT_EQ(examples[0]->feature_space[examples[0]->indices[0]].namespace_extents[0],
      (VW::namespace_extent{0, 1, VW::details::CONSTANT_NAMESPACE}));

  VW::finish_example(*all, *examples[0]);
}

TEST(FlatbufferParser, SingleExample_SimpleLabel_FeatureHashes)
{
  // Testcase where user would provide feature names and feature values (no feature hashes)
  auto all = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--flatbuffer"));

  flatbuffers::FlatBufferBuilder builder;

  auto root = sample_flatbuffer_no_audit(builder, VW::parsers::flatbuffer::Label_SimpleLabel);
  builder.FinishSizePrefixed(root);

  uint8_t* buf = builder.GetBufferPointer();

  VW::multi_ex examples;
  examples.push_back(&VW::get_unused_example(all.get()));
  VW::io_buf unused_buffer;
  all->parser_runtime.flat_converter->parse_examples(all.get(), unused_buffer, examples, buf);

  auto example = all->parser_runtime.flat_converter->data()->example_obj_as_Example();
  EXPECT_EQ(example->namespaces()->size(), 1);
  // EXPECT_EQ(example->namespaces()->Get(0)->feature_names()->size(), 0);
  EXPECT_FLOAT_EQ(example->label_as_SimpleLabel()->label(), 0.0);
  EXPECT_FLOAT_EQ(example->label_as_SimpleLabel()->weight(), 1.0);
  EXPECT_EQ(example->namespaces()->Get(0)->hash(), VW::details::CONSTANT_NAMESPACE);
  EXPECT_EQ(example->namespaces()->Get(0)->full_hash(), VW::details::CONSTANT_NAMESPACE);
  // EXPECT_STREQ(example->namespaces()->Get(0)->feature_names()->Get(0)->c_str(), "hello");
  EXPECT_EQ(example->namespaces()->Get(0)->feature_names(), nullptr);
  EXPECT_EQ(example->namespaces()->Get(0)->feature_hashes()->Get(0), VW::details::CONSTANT);
  EXPECT_FLOAT_EQ(example->namespaces()->Get(0)->feature_values()->Get(0), 2.23);

  // Check vw example
  EXPECT_EQ(examples.size(), 1);
  EXPECT_FLOAT_EQ(examples[0]->l.simple.label, 0.f);
  const auto& red_features = examples[0]->ex_reduction_features.template get<VW::simple_label_reduction_features>();
  EXPECT_FLOAT_EQ(red_features.weight, 1.f);

  EXPECT_EQ(examples[0]->indices[0], VW::details::CONSTANT_NAMESPACE);
  EXPECT_FLOAT_EQ(examples[0]->feature_space[examples[0]->indices[0]].values[0], 2.23f);
  EXPECT_EQ(examples[0]->feature_space[examples[0]->indices[0]].namespace_extents.size(), 1);
  EXPECT_EQ(examples[0]->feature_space[examples[0]->indices[0]].namespace_extents[0],
      (VW::namespace_extent{0, 1, VW::details::CONSTANT_NAMESPACE}));

  VW::finish_example(*all, *examples[0]);
}

TEST(FlatbufferParser, ExampleCollection_Singleline)
{
  auto all = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--flatbuffer"));

  flatbuffers::FlatBufferBuilder builder;

  auto root = sample_flatbuffer_collection(builder, VW::parsers::flatbuffer::Label_SimpleLabel);
  builder.FinishSizePrefixed(root);

  uint8_t* buf = builder.GetBufferPointer();

  VW::multi_ex examples;
  examples.push_back(&VW::get_unused_example(all.get()));
  VW::io_buf unused_buffer;
  all->parser_runtime.flat_converter->parse_examples(all.get(), unused_buffer, examples, buf);

  auto collection_examples = all->parser_runtime.flat_converter->data()->example_obj_as_ExampleCollection()->examples();
  EXPECT_EQ(collection_examples->size(), 1);
  EXPECT_EQ(collection_examples->Get(0)->namespaces()->size(), 1);
  EXPECT_EQ(collection_examples->Get(0)->namespaces()->Get(0)->feature_names()->size(), 1);
  EXPECT_FLOAT_EQ(collection_examples->Get(0)->label_as_SimpleLabel()->label(), 0.0);
  EXPECT_FLOAT_EQ(collection_examples->Get(0)->label_as_SimpleLabel()->weight(), 1.0);
  EXPECT_EQ(collection_examples->Get(0)->namespaces()->Get(0)->hash(), VW::details::CONSTANT_NAMESPACE);
  EXPECT_EQ(collection_examples->Get(0)->namespaces()->Get(0)->full_hash(), VW::details::CONSTANT_NAMESPACE);
  EXPECT_STREQ(collection_examples->Get(0)->namespaces()->Get(0)->feature_names()->Get(0)->c_str(), "hello");
  EXPECT_EQ(collection_examples->Get(0)->namespaces()->Get(0)->feature_hashes()->Get(0), VW::details::CONSTANT);
  EXPECT_FLOAT_EQ(collection_examples->Get(0)->namespaces()->Get(0)->feature_values()->Get(0), 2.23);

  // check vw example
  EXPECT_EQ(examples.size(), 1);
  EXPECT_FLOAT_EQ(examples[0]->l.simple.label, 0.f);
  const auto& red_features = examples[0]->ex_reduction_features.template get<VW::simple_label_reduction_features>();
  EXPECT_FLOAT_EQ(red_features.weight, 1.f);

  EXPECT_EQ(examples[0]->indices[0], VW::details::CONSTANT_NAMESPACE);
  EXPECT_FLOAT_EQ(examples[0]->feature_space[examples[0]->indices[0]].values[0], 2.23f);
  EXPECT_EQ(examples[0]->feature_space[examples[0]->indices[0]].namespace_extents.size(), 1);
  EXPECT_EQ(examples[0]->feature_space[examples[0]->indices[0]].namespace_extents[0],
      (VW::namespace_extent{0, 1, VW::details::CONSTANT_NAMESPACE}));

  VW::finish_example(*all, *examples[0]);
}

TEST(FlatbufferParser, SingleExample_MissingFeatureIndices)
{
  // Testcase where user would provide feature names and feature values (no feature hashes)
  auto all = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--flatbuffer", "--audit"));

  flatbuffers::FlatBufferBuilder builder;

  auto root = sample_flatbuffer_error_code(builder, VW::parsers::flatbuffer::Label_SimpleLabel);
  builder.FinishSizePrefixed(root);

  uint8_t* buf = builder.GetBufferPointer();

  VW::multi_ex examples;
  examples.push_back(&VW::get_unused_example(all.get()));
  VW::io_buf unused_buffer;
  EXPECT_EQ(all->parser_runtime.flat_converter->parse_examples(all.get(), unused_buffer, examples, buf),
      VW::experimental::error_code::fb_parser_feature_hashes_names_missing);
  EXPECT_EQ(all->parser_runtime.example_parser->reader(all.get(), unused_buffer, examples), 0);

  auto example = all->parser_runtime.flat_converter->data()->example_obj_as_Example();
  EXPECT_EQ(example->namespaces()->size(), 1);
  EXPECT_FLOAT_EQ(example->label_as_SimpleLabel()->label(), 0.0);
  EXPECT_FLOAT_EQ(example->label_as_SimpleLabel()->weight(), 1.0);
  EXPECT_EQ(example->namespaces()->Get(0)->hash(), VW::details::CONSTANT_NAMESPACE);
  EXPECT_EQ(example->namespaces()->Get(0)->full_hash(), VW::details::CONSTANT_NAMESPACE);
  EXPECT_FLOAT_EQ(example->namespaces()->Get(0)->feature_values()->Get(0), 2.23);
  EXPECT_EQ(example->namespaces()->Get(0)->feature_names(), nullptr);

  // Check vw example
  EXPECT_EQ(examples.size(), 1);
  EXPECT_FLOAT_EQ(examples[0]->l.simple.label, 0.f);
  const auto& red_features = examples[0]->ex_reduction_features.template get<VW::simple_label_reduction_features>();
  EXPECT_FLOAT_EQ(red_features.weight, 1.f);
  EXPECT_EQ(examples[0]->indices[0], VW::details::CONSTANT_NAMESPACE);

  VW::finish_example(*all, *examples[0]);
}

namespace vwtest
{
template <bool test_audit_strings>
constexpr FeatureSerialization get_feature_serialization()
{
  return test_audit_strings ? FeatureSerialization::ExcludeFeatureHash : FeatureSerialization::ExcludeFeatureNames;
}
}  // namespace vwtest

template <typename root_prototype_t, bool test_audit_strings = true>
void run_parse_and_verify_test(VW::workspace& w, const root_prototype_t& root_obj)
{
  constexpr FeatureSerialization feature_serialization = vwtest::get_feature_serialization<test_audit_strings>();

  flatbuffers::FlatBufferBuilder builder;

  auto root = vwtest::create_example_root<feature_serialization>(builder, w, root_obj);
  builder.FinishSizePrefixed(root);

  VW::io_buf buf;

  uint8_t* buf_ptr = builder.GetBufferPointer();
  size_t buf_size = builder.GetSize();

  buf.add_file(VW::io::create_buffer_view((const char*)buf_ptr, buf_size));

  std::vector<VW::multi_ex> wrapped;
  VW::multi_ex examples;

  bool done = false;
  while (!done && !w.parser_runtime.example_parser->done)
  {
    VW::multi_ex dispatch_examples;
    dispatch_examples.push_back(&VW::get_unused_example(&w));

    VW::experimental::api_status status;
    int result = w.parser_runtime.flat_converter->parse_examples(&w, buf, dispatch_examples, nullptr, &status);

    switch (result)
    {
      case VW::experimental::error_code::success:
        if (!w.l->is_multiline() || !dispatch_examples[0]->is_newline)
        {
          examples.push_back(dispatch_examples[0]);
          dispatch_examples.clear();
        }
        else if (w.l->is_multiline())
        {
          EXPECT_TRUE(dispatch_examples[0]->is_newline);

          // since we are in multiline mode, we have a complete multi_ex, so put it
          // in 'wrapped', and move to the next one
          wrapped.push_back(std::move(examples));
          examples.clear();

          // note that we do not clean up dispatch_examples, because we want the
          // extra newline example to be cleaned up below
        }

        break;
      case VW::experimental::error_code::nothing_to_parse:

        done = true;
        break;
      default:
        throw std::runtime_error(status.get_error_msg());
    }

    VW::finish_example(w, dispatch_examples);
  }

  if (examples.size() > 0)
  {
    wrapped.push_back(std::move(examples));
    examples.clear();
  }

  vwtest::verify_example_root<feature_serialization>(w, w.parser_runtime.flat_converter->data(), root_obj);
  vwtest::verify_example_root<feature_serialization>(w, (std::vector<VW::multi_ex>)wrapped, root_obj);

  for (size_t i = 0; i < wrapped.size(); i++)
  {
    VW::finish_example(w, wrapped[i]);
    wrapped[i].clear();
  }
}

TEST(FlatbufferParser, ExampleCollection_Multiline)
{
  auto all = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--flatbuffer", "--audit", "--cb_explore_adf"));

  example_data_generator data_gen;

  auto prototype = data_gen.create_cb_adf_log(2, 1, 0.4f);

  run_parse_and_verify_test(*all, prototype);
}

TEST(FlatbufferParser, MultiExample_Multiline)
{
  auto all = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--flatbuffer", "--audit", "--cb_explore_adf"));

  flatbuffers::FlatBufferBuilder builder;

  multiex prototype = {{
      {{
           {"U_a", {{"a", 1.f}, {"b", 2.f}}},
           {"U_b", {{"a", 3.f}, {"b", 4.f}}},
       },
          vwtest::cb_label_shared(), "tag1"},
      {
          {
              {"T_a", {{"a", 5.f}, {"b", 6.f}}},
              {"T_b", {{"a", 7.f}, {"b", 8.f}}},
          },
          vwtest::cb_label({{1, 1, 0.5f}}),
      },
  }};

  run_parse_and_verify_test(*all, prototype);
}

TEST(FlatBufferParser, LabelSmokeTest_ContinuousLabel)
{
  using namespace vwtest;
  using example = vwtest::example;

  auto all = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--flatbuffer", "--audit"));
  example_data_generator datagen;

  example ex = {{datagen.create_namespace("U_a", 1, 1)},

      continuous_label({{1, 0.5f, 0.25}})};

  run_parse_and_verify_test(*all, ex);
}

TEST(FlatBufferParser, LabelSmokeTest_Slates)
{
  using namespace vwtest;

  auto all = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--flatbuffer", "--audit", "--slates"));
  example_data_generator datagen;

  // this is not the best way to describe it as it is technically labelled in the strictest sense
  // (namely, having slates labels associated with the examples), but there is no labelling data
  // there, because we do not have a global cost or probabilities for the slots.
  multiex unlabeled_example = {{{{datagen.create_namespace("Context", 1, 1)},

                                    slates::shared()},
      {{datagen.create_namespace("Action", 1, 1)},

          slates::action(0)},
      {{datagen.create_namespace("Action", 1, 1)},

          slates::action(0)},
      {{datagen.create_namespace("Slot", 1, 1)},

          slates::slot(0)}}};

  run_parse_and_verify_test(*all, unlabeled_example);

  multiex labeled_example{{{{datagen.create_namespace("Context", 1, 1)},

                               slates::shared(0.5)},
      {{datagen.create_namespace("Action", 1, 1)},

          slates::action(0)},
      {{datagen.create_namespace("Action", 1, 1)},

          slates::action(0)},
      {{datagen.create_namespace("Slot", 1, 1)},

          slates::slot(0, {{1, 0.6}, {0, 0.4}})}}};

  run_parse_and_verify_test<multiex, false>(*all, labeled_example);
}
