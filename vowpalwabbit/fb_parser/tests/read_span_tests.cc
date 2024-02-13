
// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "example_data_generator.h"
#include "prototype_typemappings.h"
#include "vw/common/future_compat.h"
#include "vw/common/string_view.h"
#include "vw/core/constant.h"
#include "vw/core/error_constants.h"
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
void create_flatbuffer_span_and_validate(VW::workspace& w, const T& prototype)
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

  create_flatbuffer_span_and_validate(*all, prototype);
}

TEST(FlatbufferParser, ReadSpanFlatbuffer_MultiExample)
{
  auto all = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--flatbuffer", "--cb_explore_adf"));

  vwtest::example_data_generator data_gen;
  vwtest::prototype_multiexample_t prototype = data_gen.create_cb_adf_example(3, 1, "tag");

  create_flatbuffer_span_and_validate(*all, prototype);
}

TEST(FlatbufferParser, ReadSpanFlatbuffer_ExampleCollectionSinglelines)
{
  auto all = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--flatbuffer"));

  vwtest::example_data_generator data_gen;
  vwtest::prototype_example_collection_t prototype = data_gen.create_simple_log(3, 3, 4);

  create_flatbuffer_span_and_validate(*all, prototype);
}

TEST(FlatbufferParser, ReadSpanFlatbuffer_ExampleCollectionMultiline)
{
  auto all = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--flatbuffer", "--cb_explore_adf"));

  vwtest::example_data_generator data_gen;
  vwtest::prototype_example_collection_t prototype = data_gen.create_cb_adf_log(1, 3, 4);

  create_flatbuffer_span_and_validate(*all, prototype);
}
