// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw_string_view.h"
#include "hashstring.h"

typedef unsigned char namespace_index;

namespace VW
{
struct namespace_info
{
private:
  // The lowest 8 bits of this value are the
  uint64_t _namespace_combined_hash;

public:
  explicit namespace_info(uint64_t ns_hash) : _namespace_combined_hash(ns_hash) {}

  // The first character of the namespace, or the index, is stored as the lowest 8 bits.
  // To get the value we just mask out the 8 bits.
  namespace_index get_namespace_index() const
  {
    return static_cast<namespace_index>(_namespace_combined_hash & static_cast<uint64_t>(0xFF));
  }
  uint64_t get_hash() const { return _namespace_combined_hash; }
};

namespace_info make_namespace_info(VW::string_view namespace_name, uint64_t hash_seed, hash_func_t hasher)
{
  namespace_index index = namespace_name[0];
  uint64_t hash_value = hasher(namespace_name.data(), namespace_name.length(), hash_seed);
  // Mask out bottom 8 bits
  hash_value = hash_value & (~static_cast<uint64_t>(0xFF));
  // Insert the index bits
  hash_value = hash_value & static_cast<uint64_t>(index);
  return namespace_info{hash_value};
}
}  // namespace VW
