// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/core/vw.h"

#include "vw/fb_parser/parse_example_flatbuffer.h"
#include "flatbuffers/flatbuffers.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace fb = VW::parsers::flatbuffer;
using namespace flatbuffers;

namespace vwtest 
{

struct feature_t
{
  feature_t(std::string name, float value) : has_name(true), name(name), value(value), hash(0)
  {
  }

  feature_t(uint64_t hash, float value) : has_name(false), name(nullptr), value(value), hash(hash)
  {
  }

  feature_t(feature_t&& other) : name(std::move(other.name)), value(other.value), hash(other.hash)
  {
  }

  feature_t(const feature_t& other) : name(other.name), value(other.value), hash(other.hash)
  {
  }
  
  bool has_name;
  std::string name;
  float value;
  uint64_t hash;
};

struct prototype_namespace_t
{
  prototype_namespace_t(const char* name, std::vector<feature_t>&& features) : has_name(true), name(name), features(features), hash(0), feature_group(name[0])
  {
  }

  prototype_namespace_t(char feature_group, uint64_t hash, std::vector<feature_t>&& features) : has_name(false), name(nullptr), features(features), hash(hash), feature_group(feature_group)
  {
  }

  prototype_namespace_t(prototype_namespace_t&& other) = delete;
  prototype_namespace_t(const prototype_namespace_t& other) : has_name(other.has_name), name(other.name), features(other.features), hash(other.hash), feature_group(other.feature_group)
  {
  }

  bool has_name;
  std::string name;
  std::vector<feature_t> features;
  uint64_t hash;
  uint8_t feature_group;

  template <bool include_feature_names = true>
  Offset<fb::Namespace> create_flatbuffer(FlatBufferBuilder& builder, VW::workspace& w) const
  {
    // When building these objects, we interpret the presence of a string as a signal to
    // hash the string
    uint64_t hash = this->hash;
    if (has_name)
    {
      hash = VW::hash_space(w, name);
    }

    std::vector<Offset<String>> feature_names;
    std::vector<float> feature_values;
    std::vector<uint64_t> feature_hashes;

    for (const auto& f : features)
    {
      if VW_STD17_CONSTEXPR (include_feature_names)
      {
        feature_names.push_back(f.has_name ? builder.CreateString(f.name) : Offset<String>() /* nullptr */ );
      }

      feature_values.push_back(f.value);
      feature_hashes.push_back(f.has_name ? VW::hash_feature(w, f.name, hash) : f.hash);
    }

    const auto name_offset = has_name ? builder.CreateString(name) : Offset<String>();
    
    Offset<Vector<Offset<String>>> feature_names_offset = include_feature_names ? builder.CreateVector(feature_names) : Offset<Vector<Offset<String>>>() /* nullptr */;
    Offset<Vector<float>> feature_values_offset = builder.CreateVector(feature_values);
    Offset<Vector<uint64_t>> feature_hashes_offset = builder.CreateVector(feature_hashes);

    return fb::CreateNamespace(builder, name_offset, feature_group, hash, feature_names_offset, feature_values_offset, feature_hashes_offset);
  }


  template <bool expect_feature_names = true>
  void verify(VW::workspace& w, const fb::Namespace* ns) const
  {
    uint64_t hash = this->hash;
    if (has_name)
    {
      hash = VW::hash_space(w, name);
      EXPECT_EQ(ns->name()->str(), name);
    }
    else
    {
      EXPECT_EQ(ns->name(), nullptr);
    }

    EXPECT_EQ(ns->full_hash(), hash);
    EXPECT_EQ(ns->hash(), feature_group);

    if VW_STD17_CONSTEXPR (expect_feature_names)
    {
      EXPECT_EQ(ns->feature_names()->size(), features.size());
    }

    EXPECT_EQ(ns->feature_values()->size(), features.size());
    EXPECT_EQ(ns->feature_hashes()->size(), features.size());

    for (size_t i = 0; i < features.size(); i++)
    {
      if (features[i].has_name)
      {
        EXPECT_EQ(ns->feature_names()->Get(i)->str(), features[i].name);
        EXPECT_EQ(ns->feature_hashes()->Get(i), VW::hash_feature(w, features[i].name, hash));
      }
      else
      {
        EXPECT_EQ(ns->feature_names()->Get(i), nullptr);
        EXPECT_EQ(ns->feature_hashes()->Get(i), features[i].hash);
      }

      EXPECT_EQ(ns->feature_values()->Get(i), features[i].value);
    }
  }

  template <bool expect_feature_names = true>
  void verify(VW::workspace& w, const size_t, const VW::example& e) const
  {
    uint64_t hash = this->hash;
    if (has_name)
    {
      hash = VW::hash_space(w, name);
    }
    
    bool is_indexed = false;
    for (size_t i = 0; i < e.indices.size(); i++)
    {
      if (e.indices[i] == feature_group)
      {
        is_indexed = true;
        break;
      }
    }
    EXPECT_TRUE(is_indexed);

    const VW::features& features = e.feature_space[feature_group];

    size_t extent_index = 0;
    for (; extent_index < features.namespace_extents.size(); extent_index++)
    {
      if (features.namespace_extents[extent_index].hash == hash)
      {
        break;
      }
    }

    EXPECT_LT(extent_index, features.namespace_extents.size());
    const auto& extent = features.namespace_extents[extent_index];
    
    for (size_t i_f = extent.begin_index; i_f < extent.end_index; i_f++)
    {
      auto& f = this->features[i_f];
      if (f.has_name)
      {
        EXPECT_EQ(features.indices[i_f], VW::hash_feature(w, f.name, hash));

        if VW_STD17_CONSTEXPR (expect_feature_names)
        {
          EXPECT_EQ(features.space_names[i_f].name, f.name);
        }
      }
      else
      {
        EXPECT_EQ(features.indices[i_f], f.hash);
      }

      EXPECT_EQ(features.values[i_f], f.value);
    }
  }
};

}

#define USE_PROTOTYPE_MNEMONICS_NS \
namespace vwtest { \
  using ns = vwtest::prototype_namespace_t; \
}
