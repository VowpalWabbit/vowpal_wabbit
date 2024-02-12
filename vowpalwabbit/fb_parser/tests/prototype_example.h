// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "flatbuffers/flatbuffers.h"
#include "prototype_label.h"
#include "prototype_namespace.h"
#include "vw/fb_parser/parse_example_flatbuffer.h"

#ifndef VWFB_BUILDERS_ONLY
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#endif

namespace fb = VW::parsers::flatbuffer;
using namespace flatbuffers;

namespace vwtest
{

struct prototype_example_t
{
  std::vector<prototype_namespace_t> namespaces;
  prototype_label_t label = no_label();
  const char* tag = nullptr;

  inline size_t count_indices() const
  {
    size_t count = 0;
    bool seen[VW::NUM_NAMESPACES] = {false};
    for (auto& ns : namespaces)
    {
      count += !seen[ns.feature_group];
      seen[ns.feature_group] = true;
    }

    return count;
  }

  template <bool include_feature_names = true>
  Offset<fb::Example> create_flatbuffer(flatbuffers::FlatBufferBuilder& builder, VW::workspace& w) const
  {
    std::vector<Offset<fb::Namespace>> fb_namespaces;
    for (auto& ns : namespaces) { fb_namespaces.push_back(ns.create_flatbuffer<>(builder, w)); }

    Offset<Vector<Offset<fb::Namespace>>> fb_namespaces_vector = builder.CreateVector(fb_namespaces);

    auto label = this->label.create_flatbuffer(builder, w);

    Offset<String> tag_offset = this->tag ? builder.CreateString(this->tag) : Offset<String>();

    auto example = fb::CreateExample(builder, fb_namespaces_vector, this->label.label_type, label, tag_offset);

    return example;
  }

#ifndef VWFB_BUILDERS_ONLY
  template <bool expect_feature_names = true>
  void verify(VW::workspace& w, const fb::Example* e) const
  {
    for (size_t i = 0; i < namespaces.size(); i++)
    {
      namespaces[i].verify<expect_feature_names>(w, e->namespaces()->Get(i));
    }

    label.verify(w, e);
  }

  template <bool expect_feature_names = true>
  void verify(VW::workspace& w, const VW::example& e) const
  {
    EXPECT_EQ(e.indices.size(), count_indices());

    for (size_t i = 0; i < namespaces.size(); i++)
    {
      namespaces[i].verify<expect_feature_names>(w, namespaces[i].feature_group, e);
    }

    label.verify(w, e);
  }
#endif
};

struct prototype_multiexample_t
{
  std::vector<prototype_example_t> examples;

  template <bool include_feature_names = true>
  Offset<fb::MultiExample> create_flatbuffer(flatbuffers::FlatBufferBuilder& builder, VW::workspace& w) const
  {
    std::vector<Offset<fb::Example>> fb_examples;
    for (auto& ex : examples) { fb_examples.push_back(ex.create_flatbuffer<include_feature_names>(builder, w)); }

    Offset<Vector<Offset<fb::Example>>> fb_examples_vector = builder.CreateVector(fb_examples);

    return fb::CreateMultiExample(builder, fb_examples_vector);
  }

#ifndef VWFB_BUILDERS_ONLY
  template <bool expect_feature_names = true>
  void verify(VW::workspace& w, const fb::MultiExample* e) const
  {
    EXPECT_EQ(e->examples()->size(), examples.size());

    for (size_t i = 0; i < examples.size(); i++) { examples[i].verify<expect_feature_names>(w, e->examples()->Get(i)); }
  }

  template <bool expect_feature_names = true>
  void verify(VW::workspace& w, const VW::multi_ex& e) const
  {
    EXPECT_EQ(e.size(), examples.size());

    for (size_t i = 0; i < examples.size(); i++) { examples[i].verify<expect_feature_names>(w, *e[i]); }
  }
#endif
};

struct prototype_example_collection_t
{
  using type_t = bool;

  std::vector<prototype_example_t> examples;
  std::vector<prototype_multiexample_t> multi_examples;
  bool is_multiline;

  template <bool include_feature_names = true>
  Offset<fb::ExampleCollection> create_flatbuffer(flatbuffers::FlatBufferBuilder& builder, VW::workspace& w) const
  {
    std::vector<Offset<fb::Example>> fb_examples;
    for (auto& ex : examples) { fb_examples.push_back(ex.create_flatbuffer<include_feature_names>(builder, w)); }

    std::vector<Offset<fb::MultiExample>> fb_multi_examples;
    for (auto& ex : multi_examples)
    {
      fb_multi_examples.push_back(ex.create_flatbuffer<include_feature_names>(builder, w));
    }

    Offset<Vector<Offset<fb::Example>>> fb_examples_vector = builder.CreateVector(fb_examples);
    Offset<Vector<Offset<fb::MultiExample>>> fb_multi_example_vector = builder.CreateVector(fb_multi_examples);

    return fb::CreateExampleCollection(builder, fb_examples_vector, fb_multi_example_vector, is_multiline);
  }

#ifndef VWFB_BUILDERS_ONLY
  template <bool expect_feature_names = true>
  void verify(VW::workspace& w, const fb::ExampleCollection* e) const
  {
    EXPECT_EQ(e->examples()->size(), examples.size());
    EXPECT_EQ(e->multi_examples()->size(), multi_examples.size());

    for (size_t i = 0; i < examples.size(); i++) { examples[i].verify<expect_feature_names>(w, e->examples()->Get(i)); }

    for (size_t i = 0; i < multi_examples.size(); i++)
    {
      multi_examples[i].verify<expect_feature_names>(w, e->multi_examples()->Get(i));
    }
  }

  template <bool expect_feature_names = true>
  void verify_singleline(VW::workspace& w, const VW::multi_ex& e) const
  {
    EXPECT_EQ(is_multiline, false);

    for (size_t i = 0; i < examples.size(); i++) { examples[i].verify<expect_feature_names>(w, *e[i]); }
  }

  template <bool expect_feature_names = true>
  void verify_multiline(VW::workspace& w, const std::vector<VW::multi_ex>& e) const
  {
    EXPECT_EQ(is_multiline, true);

    for (size_t i = 0; i < multi_examples.size(); i++) { multi_examples[i].verify<expect_feature_names>(w, e[i]); }
  }
#endif
};

}  // namespace vwtest

#define USE_PROTOTYPE_MNEMONICS_EX                              \
  namespace vwtest                                              \
  {                                                             \
  using example = vwtest::prototype_example_t;                  \
  using multiex = vwtest::prototype_multiexample_t;             \
  using ex_collection = vwtest::prototype_example_collection_t; \
  }

// // template function is_example_root_type, returns true if T is prototype_example,
// // prototype_multiexample, or prototype_example_collection
// template <typename T>
// struct is_example_root_type
// {
//   static constexpr bool value =
//       std::is_same<T, prototype_example_t>::value ||
//       std::is_same<T, prototype_multiexample_t>::value ||
//       std::is_same<T, prototype_example_collection_t>::value;
// };

// template <typename T, class = typename std::enable_if<is_example_root_type<T>::value>::type>
// struct prototype_example_root
// {
// public:
//   using fb_type = ExampleRoot;

//   template <typename... Args>
//   prototype_example_root(Args&&... args) : _example(std::forward<Args>(args)...) {}

//   ::flatbuffers::Offset<ExampleRoot> create(flatbuffers::FlatBufferBuilder& builder) const;

//   void assert_equivalent(const flatbuffers::Table* table) const;
//   void assert_equivalent(const VW::multi_ex& examples) const;

// private:
//   T _prototype_example;
// };