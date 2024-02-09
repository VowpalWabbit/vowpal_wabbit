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
  EXPECT_EQ(all->parser_runtime.flat_converter->parse_examples(all.get(), unused_buffer, examples, buf), 8);
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

template <typename root_prototype_t, bool test_audit_strings = true>
void run_parse_and_verify_test(VW::workspace& w, const root_prototype_t& root_obj)
{
  flatbuffers::FlatBufferBuilder builder;
  auto root = vwtest::create_example_root<test_audit_strings>(builder, w, root_obj);
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

  vwtest::verify_example_root<test_audit_strings>(w, w.parser_runtime.flat_converter->data(), root_obj);
  vwtest::verify_example_root<test_audit_strings>(w, (std::vector<VW::multi_ex>)wrapped, root_obj);

  for (size_t i = 0; i < wrapped.size(); i++)
  {
    VW::finish_example(w, wrapped[i]);
    wrapped[i].clear();
  }
}

TEST(FlatbufferParser, ExampleCollection_Multiline)
{
  auto all = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--flatbuffer", "--cb_explore_adf"));

  example_data_generator data_gen;

  auto prototype = data_gen.create_cb_adf_log(3, 4, 0.4f);

  run_parse_and_verify_test(*all, prototype);
}

TEST(FlatbufferParser, MultiExample_Multiline)
{
  auto all = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--flatbuffer", "--cb_explore_adf"));

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

  auto all = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--flatbuffer"));
  example_data_generator datagen;

  example ex = {{datagen.create_namespace("U_a", 1, 1)},

      continuous_label({{1, 0.5f, 0.25}})};

  run_parse_and_verify_test(*all, ex);
}

TEST(FlatBufferParser, LabelSmokeTest_Slates)
{
  using namespace vwtest;

  auto all = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--flatbuffer", "--slates"));
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

  run_parse_and_verify_test(*all, labeled_example);
}

namespace vwtest
{
template <typename T>
struct fb_type
{
};

template <>
struct fb_type<prototype_namespace_t>
{
  using type = VW::parsers::flatbuffer::Namespace;
};

template <>
struct fb_type<prototype_example_t>
{
  using type = VW::parsers::flatbuffer::Example;
};

template <>
struct fb_type<prototype_multiexample_t>
{
  using type = VW::parsers::flatbuffer::MultiExample;
};

template <>
struct fb_type<prototype_example_collection_t>
{
  using type = VW::parsers::flatbuffer::ExampleCollection;
};

using union_t = void;

template <>
struct fb_type<prototype_label_t>
{
  using type = union_t;
};
}  // namespace vwtest

template <typename T, typename FB_t = typename vwtest::fb_type<T>::type>
void create_flatbuffer_and_validate(VW::workspace& w, const T& prototype)
{
  flatbuffers::FlatBufferBuilder builder;

  Offset<FB_t> buffer_offset = prototype.create_flatbuffer(builder, w);
  builder.Finish(buffer_offset);

  const FB_t* fb_obj = GetRoot<FB_t>(builder.GetBufferPointer());

  prototype.verify(w, fb_obj);
}

template <>
void create_flatbuffer_and_validate<prototype_label_t, void>(VW::workspace& w, const prototype_label_t& prototype)
{
  if (prototype.label_type == fb::Label_NONE) { return; }  // there is no flatbuffer to create

  flatbuffers::FlatBufferBuilder builder;

  Offset<void> buffer_offset = prototype.create_flatbuffer(builder, w);
  builder.Finish(buffer_offset);

  switch (prototype.label_type)
  {
    case fb::Label_SimpleLabel:
    case fb::Label_CBLabel:
    case fb::Label_ContinuousLabel:
    case fb::Label_Slates_Label:
    {
      prototype.verify(w, prototype.label_type, builder.GetBufferPointer());
      break;
    }
    case fb::Label_NONE:
    {
      break;
    }
    default:
    {
      THROW("Label type not currently supported for create_flatbuffer_and_validate");
      break;
    }
  }
}

TEST(FlatBufferParser, ValidateTestAffordances_NoLabel)
{
  auto all = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--flatbuffer"));

  prototype_label_t label_prototype = vwtest::no_label();
  create_flatbuffer_and_validate(*all, label_prototype);
}

TEST(FlatBufferParser, ValidateTestAffordances_SimpleLabel)
{
  auto all = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--flatbuffer"));
  create_flatbuffer_and_validate(*all, simple_label(0.5, 1.0));
}

TEST(FlatBufferParser, ValidateTestAffordances_CBLabel)
{
  auto all = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--flatbuffer", "--cb_explore_adf"));
  create_flatbuffer_and_validate(*all, cb_label({1.5, 2, 0.25f}));
}

TEST(FlatBufferParser, ValidateTestAffordances_ContinuousLabel)
{
  auto all = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--flatbuffer"));

  std::vector<VW::cb_continuous::continuous_label_elm> probabilities = {{1, 0.5f, 0.25}};

  create_flatbuffer_and_validate(*all, continuous_label(probabilities));
}

TEST(FlatBufferParser, ValidateTestAffordances_Slates)
{
  auto all = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--flatbuffer", "--slates"));

  std::vector<VW::action_score> probabilities = {{1, 0.5f}, {2, 0.25f}};

  VW::slates::example_type types[] = {
      VW::slates::example_type::UNSET,
      VW::slates::example_type::ACTION,
      VW::slates::example_type::SHARED,
      VW::slates::example_type::SLOT,
  };

  for (VW::slates::example_type type : types)
  {
    create_flatbuffer_and_validate(*all, slates_label_raw(type, 0.5, true, 0.3, 1, probabilities));
  }
}

TEST(FlatbufferParser, ValidateTestAffordances_Namespace)
{
  auto all = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--flatbuffer"));

  prototype_namespace_t ns_prototype = {"U_a", {{"a", 1.f}, {"b", 2.f}}};
  create_flatbuffer_and_validate(*all, ns_prototype);
}

TEST(FlatbufferParser, ValidateTestAffordances_Example_Simple)
{
  auto all = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--flatbuffer"));

  prototype_example_t ex_prototype = {{
                                          {"U_a", {{"a", 1.f}, {"b", 2.f}}},
                                          {"U_b", {{"a", 3.f}, {"b", 4.f}}},
                                      },
      vwtest::simple_label(0.5, 1.0)};
  create_flatbuffer_and_validate(*all, ex_prototype);
}

TEST(FlatbufferParser, ValidateTestAffordances_Example_Unlabeled)
{
  auto all = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--flatbuffer"));

  prototype_example_t ex_prototype = {{
      {"U_a", {{"a", 1.f}, {"b", 2.f}}},
      {"U_b", {{"a", 3.f}, {"b", 4.f}}},
  }};
  create_flatbuffer_and_validate(*all, ex_prototype);
}

TEST(FlatbufferParser, ValidateTestAffordances_Example_CBShared)
{
  auto all = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--flatbuffer", "--cb_explore_adf"));

  prototype_example_t ex_prototype = {{
                                          {"U_a", {{"a", 1.f}, {"b", 2.f}}},
                                          {"U_b", {{"a", 3.f}, {"b", 4.f}}},
                                      },
      vwtest::cb_label_shared(), "tag1"};
  create_flatbuffer_and_validate(*all, ex_prototype);
}

TEST(FlatbufferParser, ValidateTestAffordances_Example_CB)
{
  auto all = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--flatbuffer", "--cb_explore_adf"));

  prototype_example_t ex_prototype = {{
                                          {"T_a", {{"a", 5.f}, {"b", 6.f}}},
                                          {"T_b", {{"a", 7.f}, {"b", 8.f}}},
                                      },
      vwtest::cb_label({1, 1, 0.5f}), "tag1"};
  create_flatbuffer_and_validate(*all, ex_prototype);
}

TEST(FlatbufferParser, ValidateTestAffordances_MultiExample)
{
  auto all = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--flatbuffer"));

  prototype_multiexample_t multiex_prototype = {{
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
  create_flatbuffer_and_validate(*all, multiex_prototype);
}

TEST(FlatbufferParser, ValidateTestAffordances_ExampleCollectionMultiline)
{
  auto all = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--flatbuffer", "--cb_explore_adf"));

  example_data_generator data_gen;
  prototype_example_collection_t prototype = data_gen.create_cb_adf_log(2, 2, 0.5f);

  create_flatbuffer_and_validate(*all, prototype);
}
