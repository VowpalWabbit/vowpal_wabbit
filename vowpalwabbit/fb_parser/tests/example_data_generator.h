// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "flatbuffers/flatbuffers.h"
#include "vw/fb_parser/generated/example_generated.h"

#include "prototype_example.h"
#include "prototype_example_root.h"
#include "prototype_label.h"
#include "prototype_namespace.h"
#include "vw/common/hash.h"
#include "vw/common/random.h"
#include "vw/common/future_compat.h"

#include "vw/core/error_constants.h"

#include <vector>

USE_PROTOTYPE_MNEMONICS_EX

using namespace flatbuffers;
namespace fb = VW::parsers::flatbuffer;

namespace vwtest
{

class example_data_generator
{
public:
  example_data_generator() : rng(create_test_random_state()) {}

  static VW::rand_state create_test_random_state();

  prototype_namespace_t create_namespace(std::string name, uint8_t numeric_features, uint8_t string_features);

  prototype_example_t create_simple_example(uint8_t numeric_features, uint8_t string_features);
  prototype_example_t create_cb_action(
      uint8_t action, float probability = 0.0, bool rewarded = false, const char* tag = nullptr);
  prototype_example_t create_shared_context(
      uint8_t numeric_features, uint8_t string_features, const char* tag = nullptr);

  prototype_multiexample_t create_cb_adf_example(
      uint8_t num_actions, uint8_t reward_action_id, const char* tag = nullptr);
  prototype_example_collection_t create_cb_adf_log(uint8_t num_examples, uint8_t num_actions, float reward_p);
  prototype_example_collection_t create_simple_log(
      uint8_t num_examples, uint8_t numeric_features, uint8_t string_features);

public:
  enum NamespaceErrors
  {
    BAD_NAMESPACE_NO_ERROR = 0,
    BAD_NAMESPACE_NAME_HASH_MISSING = 1,
    BAD_NAMESPACE_FEATURE_VALUES_MISSING = 2,
    BAD_NAMESPACE_FEATURE_VALUES_HASHES_MISMATCH = 4,
    BAD_NAMESPACE_FEATURE_VALUES_NAMES_MISMATCH = 8,
    BAD_NAMESPACE_FEATURE_HASHES_NAMES_MISSING = 16,
  };

  template <NamespaceErrors errors = NamespaceErrors::BAD_NAMESPACE_NO_ERROR>
  Offset<fb::Namespace> create_bad_namespace(FlatBufferBuilder& builder, VW::workspace& w);

private:
  VW::rand_state rng;
};

template <example_data_generator::NamespaceErrors errors>
Offset<fb::Namespace> example_data_generator::create_bad_namespace(FlatBufferBuilder& builder, VW::workspace& w)
{
  prototype_namespace_t ns = create_namespace("BadNamespace", 1, 1);
  if VW_STD17_CONSTEXPR (errors == NamespaceErrors::BAD_NAMESPACE_NO_ERROR) return ns.create_flatbuffer(builder, w);

  constexpr bool include_ns_name_hash = !(errors & NamespaceErrors::BAD_NAMESPACE_NAME_HASH_MISSING);
  constexpr bool include_feature_values = !(errors & NamespaceErrors::BAD_NAMESPACE_FEATURE_VALUES_MISSING);

  constexpr bool include_feature_hashes = !(errors & NamespaceErrors::BAD_NAMESPACE_FEATURE_HASHES_NAMES_MISSING);
  constexpr bool skip_a_feature_hash = (errors & NamespaceErrors::BAD_NAMESPACE_FEATURE_VALUES_HASHES_MISMATCH);
  static_assert(!skip_a_feature_hash || include_feature_hashes, "Cannot skip a feature hash if they are not included");

  constexpr bool include_feature_names = !(errors & NamespaceErrors::BAD_NAMESPACE_FEATURE_HASHES_NAMES_MISSING);
  constexpr bool skip_a_feature_name = (errors & NamespaceErrors::BAD_NAMESPACE_FEATURE_VALUES_NAMES_MISMATCH);
  static_assert(!skip_a_feature_name || include_feature_names, "Cannot skip a feature name if they are not included");

  std::vector<Offset<String>> feature_names;
  std::vector<float> feature_values;
  std::vector<uint64_t> feature_hashes;

  for (size_t i = 0; i < ns.features.size(); i++)
  {
    const auto& f = ns.features[i];

    if VW_STD17_CONSTEXPR (include_feature_names && (!skip_a_feature_name || i == 1))
    {
      feature_names.push_back(builder.CreateString(f.name));
    }

    if VW_STD17_CONSTEXPR (include_feature_values) feature_values.push_back(f.value);

    if VW_STD17_CONSTEXPR (include_feature_hashes && (!skip_a_feature_hash || i == 0))
    {
      feature_hashes.push_back(f.hash);
    }
  }

  Offset<String> name_offset = Offset<String>();
  if (include_ns_name_hash)
  {
    name_offset = builder.CreateString(ns.name);
  }

  // This function attempts to, insofar as possible, generate a layout that looks like it could have
  // been created using the normal serialization code: In this case, that means that the strings for
  // the feature names are serialized into the builder before a call to CreateNamespaceDirect is made,
  // which is where the feature_names vector is allocated.
  Offset<Vector<Offset<String>>> feature_names_offset = include_feature_names ? builder.CreateVector(feature_names) : Offset<Vector<Offset<String>>>();
  Offset<Vector<float>> feature_values_offset = include_feature_values ? builder.CreateVector(feature_values) : Offset<Vector<float>>();
  Offset<Vector<uint64_t>> feature_hashes_offset = include_feature_hashes ? builder.CreateVector(feature_hashes) : Offset<Vector<uint64_t>>();

  fb::NamespaceBuilder ns_builder(builder);

  if VW_STD17_CONSTEXPR (include_ns_name_hash) ns_builder.add_full_hash(VW::hash_space(w, ns.name));
  if VW_STD17_CONSTEXPR (include_feature_hashes) ns_builder.add_feature_hashes(feature_hashes_offset);
  if VW_STD17_CONSTEXPR (include_feature_values) ns_builder.add_feature_values(feature_values_offset);
  if VW_STD17_CONSTEXPR (include_feature_names) ns_builder.add_feature_names(feature_names_offset);
  if VW_STD17_CONSTEXPR (include_ns_name_hash) ns_builder.add_name(name_offset);

  ns_builder.add_hash(ns.feature_group);
  return ns_builder.Finish();
}

}  // namespace vwtest