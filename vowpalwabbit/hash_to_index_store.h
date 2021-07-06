// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <map>
#include <cstdint>

typedef unsigned char namespace_index;

namespace VW
{
  struct hash_to_index_store {
    virtual namespace_index to_index(uint64_t hash) = 0;
    virtual void serialize(io_buf& buf) = 0;
    // Will overwrite existing contents and read from the given buffer.
    virtual void load(io_buf& buf) = 0;
    virtual ~hash_to_index_store() = default;
  };

  struct single_threaded_hash_to_index_store : hash_to_index_store {
    namespace_index to_index(uint64_t hash) override
    {
      auto it = _store.find(hash);
      if (it == _store.end())
      {
        // This skips the first value but it means we won't fail when we use the final value.
        auto next = get_next_valid_index();
        _store[hash] = next;
        return next;
      }
      return it->second;
    }

    void serialize(io_buf& buf) override
    {
      buf.write_value(_current);
      buf.write_value(static_cast<uint64_t>(_store.size()));
      for (const auto& it : _store)
      {
        uint64_t hash = it.first;
        namespace_index index = it.second;
        buf.write_value(hash);
        buf.write_value(index);
      }
    }

    void load(io_buf& buf) override
    {
      _store.clear();
      _current = buf.read_value<namespace_index>();
      uint64_t number_of_values = buf.read_value<uint64_t>();
      for (uint64_t i = 0; i < number_of_values; ++i)
      {
        uint64_t hash = buf.read_value<uint64_t>();
        namespace_index index = buf.read_value<namespace_index>();
        _store[hash] = index;
      }
    }

  private:
    namespace_index get_next_valid_index()
    {
      auto value = _current;
      _current++;

      // Skip past constant_namespace and wildcard_namespace.
      if(_current == default_namespace || _current == wildcard_namespace)
      {
        _current++;
      }

      if (_current >= wap_ldf_namespace)
      {
        // Run out of non reserved namespaces.
        THROW("Run out of namespaces.")
      }

      return value;
    }

    std::map<uint64_t, namespace_index> _store;
    namespace_index _current = 0;
  };
}
