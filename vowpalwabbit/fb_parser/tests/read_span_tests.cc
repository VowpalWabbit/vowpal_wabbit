
// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "example_data_generator.h"
#include "prototype_typemappings.h"
#include "vw/common/future_compat.h"
#include "vw/common/string_view.h"
#include "vw/core/constant.h"
#include "vw/core/error_constants.h"
#include "vw/core/scope_exit.h"
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
// using namespace vwtest;

namespace vwtest
{
inline void verify_multi_ex(VW::workspace& w, const prototype_example_t& single_ex, VW::multi_ex& multi_ex)
{
  ASSERT_EQ(multi_ex.size(), 1);

  prototype_multiexample_t validator;
  validator.examples.push_back(single_ex);

  validator.verify(w, multi_ex);
}

inline void verify_multi_ex(VW::workspace& w, const prototype_multiexample_t& validator, VW::multi_ex& multi_ex)
{
  validator.verify(w, multi_ex);
}

inline void verify_multi_ex(
    VW::workspace& w, const prototype_example_collection_t& ex_collection, const VW::multi_ex& multi_ex)
{
  // we expect ex_collection to either have a set of singleexamples, or a single multiexample
  if (ex_collection.examples.size() > 0)
  {
    ASSERT_EQ(multi_ex.size(), ex_collection.examples.size());
    ASSERT_EQ(ex_collection.multi_examples.size(), 0);

    prototype_multiexample_t validator = {ex_collection.examples};
    validator.verify(w, multi_ex);
  }
  else
  {
    ASSERT_EQ(ex_collection.multi_examples.size(), 1);
    ASSERT_EQ(multi_ex.size(), ex_collection.multi_examples[0].examples.size());
    ASSERT_EQ(ex_collection.examples.size(), 0);

    ex_collection.multi_examples[0].verify(w, multi_ex);
  }
}
}  // namespace vwtest

template <typename T, typename FB_t = typename vwtest::fb_type<T>::type>
void create_flatbuffer_span_and_validate(VW::workspace& w, vwtest::example_data_generator& data_gen, const T& prototype)
{
  // This is what we expect to see when we use read_span_flatbuffer, since this is intended
  // to be used for inference, and we would prefer not to force consumers of the API to have
  // to hash the input feature names manually.
  constexpr vwtest::FeatureSerialization serialization = vwtest::FeatureSerialization::ExcludeFeatureHash;

  VW::example_factory_t ex_fac = [&w]() -> VW::example& { return VW::get_unused_example(&w); };

  FlatBufferBuilder builder;
  Offset<fb::ExampleRoot> example_root = vwtest::create_example_root<serialization>(builder, w, prototype);

  builder.FinishSizePrefixed(example_root);

  const uint8_t* buffer = builder.GetBufferPointer();
  flatbuffers::uoffset_t size = builder.GetSize();

  VW::multi_ex parsed_examples;
  if (data_gen.random_bool())
  {
    parsed_examples.push_back(&ex_fac());
  }

  VW::parsers::flatbuffer::read_span_flatbuffer(&w, buffer, size, ex_fac, parsed_examples);

  verify_multi_ex(w, prototype, parsed_examples);

  VW::finish_example(w, parsed_examples);
}

TEST(FlatbufferParser, ReadSpanFlatbuffer_SingleExample)
{
  auto all = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--flatbuffer"));

  vwtest::example_data_generator data_gen;
  vwtest::prototype_example_t prototype = {
      {data_gen.create_namespace("A", 3, 4), data_gen.create_namespace("B", 2, 5)}, vwtest::simple_label(1.0f)};

  create_flatbuffer_span_and_validate(*all, data_gen, prototype);
}

TEST(FlatbufferParser, ReadSpanFlatbuffer_MultiExample)
{
  auto all = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--flatbuffer", "--cb_explore_adf"));

  vwtest::example_data_generator data_gen;
  vwtest::prototype_multiexample_t prototype = data_gen.create_cb_adf_example(3, 1, "tag");

  create_flatbuffer_span_and_validate(*all, data_gen, prototype);
}

TEST(FlatbufferParser, ReadSpanFlatbuffer_ExampleCollectionSinglelines)
{
  auto all = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--flatbuffer"));

  vwtest::example_data_generator data_gen;
  vwtest::prototype_example_collection_t prototype = data_gen.create_simple_log(3, 3, 4);

  create_flatbuffer_span_and_validate(*all, data_gen, prototype);
}

TEST(FlatbufferParser, ReadSpanFlatbuffer_ExampleCollectionMultiline)
{
  auto all = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--flatbuffer", "--cb_explore_adf"));

  vwtest::example_data_generator data_gen;
  vwtest::prototype_example_collection_t prototype = data_gen.create_cb_adf_log(1, 3, 4);

  create_flatbuffer_span_and_validate(*all, data_gen, prototype);
}

template <int error_code>
void finish_flatbuffer_and_expect_error(FlatBufferBuilder& builder, Offset<fb::ExampleRoot> root, VW::workspace& w)
{
  VW::example_factory_t ex_fac = [&w]() -> VW::example& { return VW::get_unused_example(&w); };
  VW::example_sink_f ex_sink = [&w](VW::example& ex) { VW::finish_example(w, ex); };
  if (vwtest::example_data_generator{}::random_bool())
  {
    // This is only valid because ex_fac is grabbing an example from the VW example pool
    ex_sink = nullptr;
  }

  builder.FinishSizePrefixed(root);

  const uint8_t* buffer = builder.GetBufferPointer();
  flatbuffers::uoffset_t size = builder.GetSize();

  std::vector<uint8_t> buffer_copy(buffer, buffer + size);

  VW::multi_ex parsed_examples;
  EXPECT_EQ(VW::parsers::flatbuffer::read_span_flatbuffer(
                &w, buffer_copy.data(), buffer_copy.size(), ex_fac, parsed_examples, ex_sink),
      error_code);
}

using namespace_factory_f = std::function<Offset<fb::Namespace>(FlatBufferBuilder&, VW::workspace&)>;

Offset<fb::Example> create_bad_ns_root_example(FlatBufferBuilder& builder, VW::workspace& w, namespace_factory_f ns_fac)
{
  std::vector<Offset<fb::Namespace>> namespaces = {ns_fac(builder, w)};

  Offset<void> label_offset = fb::Createno_label(builder).Union();
  return fb::CreateExample(builder, builder.CreateVector(namespaces), fb::Label_no_label, label_offset);
}

Offset<fb::MultiExample> create_bad_ns_root_multiex(
    FlatBufferBuilder& builder, VW::workspace& w, namespace_factory_f ns_fac)
{
  std::vector<Offset<fb::Example>> examples = {create_bad_ns_root_example(builder, w, ns_fac)};

  return fb::CreateMultiExample(builder, builder.CreateVector(examples));
}

template <typename T, typename FB_t = typename vwtest::fb_type<T>::type>
using builder_f = Offset<FB_t> (*)(FlatBufferBuilder&, VW::workspace&, namespace_factory_f);

template <bool multiline>
Offset<fb::ExampleCollection> create_bad_ns_root_collection(
    FlatBufferBuilder& builder, VW::workspace& w, namespace_factory_f ns_fac)
{
  if VW_STD17_CONSTEXPR (multiline)
  {
    // using "auto" here breaks the code coverage build due to template substitution failure
    std::vector<Offset<fb::MultiExample>> inner_examples = {create_bad_ns_root_multiex(builder, w, ns_fac)};
    return fb::CreateExampleCollection(builder, builder.CreateVector(std::vector<Offset<fb::Example>>()),
        builder.CreateVector(inner_examples), multiline);
  }
  else
  {
    // using "auto" here breaks the code coverage build due to template substitution failure
    std::vector<Offset<fb::Example>> inner_examples = {create_bad_ns_root_example(builder, w, ns_fac)};
    return fb::CreateExampleCollection(builder, builder.CreateVector(inner_examples),
        builder.CreateVector(std::vector<Offset<fb::MultiExample>>()), multiline);
  }
}

template <int error_code, typename FB_t, fb::ExampleType root_type>
void create_flatbuffer_span_and_expect_error(VW::workspace& w, namespace_factory_f ns_fac, builder_f<FB_t> root_builder)
{
  FlatBufferBuilder builder;
  Offset<void> data_obj = root_builder(builder, w, ns_fac).Union();

  Offset<fb::ExampleRoot> root_obj = fb::CreateExampleRoot(builder, root_type, data_obj);

  finish_flatbuffer_and_expect_error<error_code>(builder, root_obj, w);
}

using NamespaceErrors = vwtest::example_data_generator::NamespaceErrors;
template <NamespaceErrors errors, int error_code>
void run_bad_namespace_test(VW::workspace& w)
{
  vwtest::example_data_generator data_gen;

  static_assert(errors != NamespaceErrors::BAD_NAMESPACE_NO_ERROR, "This test is intended to test bad namespaces");
  namespace_factory_f ns_fac = [&data_gen](FlatBufferBuilder& builder, VW::workspace& w) -> Offset<fb::Namespace>
  { return data_gen.create_bad_namespace<errors>(builder, w); };

  create_flatbuffer_span_and_expect_error<error_code, vwtest::example, fb::ExampleType_Example>(
      w, ns_fac, &create_bad_ns_root_example);
  create_flatbuffer_span_and_expect_error<error_code, vwtest::multiex, fb::ExampleType_MultiExample>(
      w, ns_fac, &create_bad_ns_root_multiex);

  create_flatbuffer_span_and_expect_error<error_code, vwtest::ex_collection, fb::ExampleType_ExampleCollection>(
      w, ns_fac, &create_bad_ns_root_collection<false>);

  create_flatbuffer_span_and_expect_error<error_code, vwtest::ex_collection, fb::ExampleType_ExampleCollection>(
      w, ns_fac, &create_bad_ns_root_collection<true>);
}

template <NamespaceErrors target_errors, int expected_error_code>
void run_bad_namespace_test()
{
  auto all = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--flatbuffer"));

  run_bad_namespace_test<target_errors, expected_error_code>(*all);
}

TEST(FlatbufferParser, BadNamespace_FeatureValuesMissing)
{
  namespace err = VW::experimental::error_code;
  constexpr NamespaceErrors target_errors = NamespaceErrors::BAD_NAMESPACE_FEATURE_VALUES_MISSING;
  constexpr int expected_error_code = err::fb_parser_feature_values_missing;

  run_bad_namespace_test<target_errors, expected_error_code>();
}

TEST(FlatbufferParser, BadNamespace_FeatureHashesNamesMissing)
{
  namespace err = VW::experimental::error_code;
  constexpr NamespaceErrors target_errors = NamespaceErrors::BAD_NAMESPACE_FEATURE_HASHES_NAMES_MISSING;
  constexpr int expected_error_code = err::fb_parser_feature_hashes_names_missing;

  run_bad_namespace_test<target_errors, expected_error_code>();
}

TEST(FlatbufferParser, BadNamespace_FeatureValuesHashMismatch)
{
  namespace err = VW::experimental::error_code;
  constexpr NamespaceErrors target_errors = NamespaceErrors::BAD_NAMESPACE_FEATURE_VALUES_HASHES_MISMATCH;
  constexpr int expected_error_code = err::fb_parser_size_mismatch_ft_hashes_ft_values;

  run_bad_namespace_test<target_errors, expected_error_code>();
}

TEST(FlatbufferParser, BadNamespace_FeatureValuesNamesMismatch)
{
  namespace err = VW::experimental::error_code;
  constexpr NamespaceErrors target_errors = NamespaceErrors::BAD_NAMESPACE_FEATURE_VALUES_NAMES_MISMATCH;
  constexpr int expected_error_code = err::fb_parser_size_mismatch_ft_names_ft_values;

  run_bad_namespace_test<target_errors, expected_error_code>();
}

// This test is disabled because it is not possible to create a flatbuffer with a missing namespace name hash.
// TEST(FlatbufferParser, BadNamespace_NameHashMissing)
// {
//   namespace err = VW::experimental::error_code;
//   constexpr NamespaceErrors target_errors = NamespaceErrors::BAD_NAMESPACE_NAME_HASH_MISSING;
//   constexpr int expected_error_code = err::success;

//   run_bad_namespace_test<target_errors, expected_error_code>();
// }
