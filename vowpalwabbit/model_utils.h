// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "io_buf.h"
#include <iostream>
#include <set>
#include <map>
#include <queue>
#include <type_traits>

#include <fmt/format.h>

namespace VW
{
namespace automl
{
struct exclusion_config;
struct scored_config;
struct interaction_config_manager;
}  // namespace automl
namespace model_utils
{
namespace details
{
inline size_t check_length_matches(size_t actual_len, size_t expected_len)
{
  if (expected_len > 0 && actual_len != expected_len) { THROW("Unexpected end of file encountered."); }
  return actual_len;
}

template <typename T>
size_t write_text_mode_output(io_buf& io, const T& var, const std::string& name_or_readable_field_template)
{
  if (name_or_readable_field_template.empty()) { return 0; }

  std::string message;
  // If the user has supplied a template string then use that.
  if (name_or_readable_field_template.find("{}") != VW::string_view::npos)
  { message = fmt::format(name_or_readable_field_template, var); }
  else
  {
    // Use the default template string.
    message = fmt::format("{} = {}\n", name_or_readable_field_template, var);
  }

  return details::check_length_matches(io.bin_write_fixed(message.c_str(), message.size()), message.size());
}
}  // namespace details

size_t process_model_field(io_buf& io, VW::automl::exclusion_config& var, bool read,
    const std::string& name_or_readable_field_template, bool text);
size_t process_model_field(io_buf& io, VW::automl::scored_config& var, bool read,
    const std::string& name_or_readable_field_template, bool text);
size_t process_model_field(io_buf& io, VW::automl::interaction_config_manager& var, bool read,
    const std::string& name_or_readable_field_template, bool text);

/**
 * @brief This function is the uniform interface for serializing a variable to
 * the model file.
 *
 * @tparam T type of value to serialize or deserialize. This is important as it determines the number of bytes to be
 * processed.
 * @param io model file io_buf to read from or write to
 * @param var value to serialize or deserialize. Must be a type which supports
 * validly bit casting to a sequence of bytes. If reading, does not need to be
 * initialized. If writing, must be initialized.
 * @param read whether to read from io or to write to io
 * @param name_or_readable_field_template (Only used when text == true) The name
 * of the variable or a template string to use when writing.
 *   - If empty, then no output is written for this call
 *   - If the string contains a "{}", then the variable value will be substituted for that. Note: more than one "{}" in
 * the string will result in a runtime error due to the templating failure.
 *   - Otherwise, if the string is non-empty and does not contain a "{}" the value will be used as the name of the
 * variable and be outputted in the form `("{} = {}\n", name_or_readable_field_template, var)`
 * @param text whether to write in text mode or binary mode. It is invalid to
 * specify both read and text.
 * @return size_t the number of bytes written or read
 */
template <typename T>
size_t process_model_field(
    io_buf& io, const T& var, bool read, const std::string& name_or_readable_field_template, bool text)
{
  // It is not valid to read a text based field.
  assert(!(read && text));

  if (read)
  {
    auto* data = reinterpret_cast<char*>(const_cast<T*>(&var));
    auto len = sizeof(var);
    return details::check_length_matches(io.bin_read_fixed(data, len), len);
  }

  auto* data = reinterpret_cast<const char*>(&var);
  auto len = sizeof(var);

  if (text) { return details::write_text_mode_output(io, var, name_or_readable_field_template); }
  // If not read or text we are just writing the binary data.
  return details::check_length_matches(io.bin_write_fixed(data, len), len);
}

template <typename T>
size_t process_model_field(
    io_buf& io, std::set<T>& set, bool read, const std::string& name_or_readable_field_template, bool text)
{
  size_t bytes = 0;
  size_t set_size;
  if (read)
  {
    bytes += process_model_field(io, set_size, read, "", text);
    for (size_t i = 0; i < set_size; ++i)
    {
      T v;
      bytes += process_model_field(io, v, read, "", text);
      set.insert(v);
    }
  }
  else
  {
    set_size = set.size();
    bytes += process_model_field(io, set_size, read, name_or_readable_field_template + "_size", text);
    size_t i = 0;
    for (T v : set)
    {
      bytes += process_model_field(io, v, read, name_or_readable_field_template + "_" + std::to_string(i), text);
      ++i;
    }
  }
  return bytes;
}

template <typename T>
size_t process_model_field(
    io_buf& io, std::vector<T>& vec, bool read, const std::string& name_or_readable_field_template, bool text)
{
  size_t bytes = 0;
  size_t vec_size;
  if (read)
  {
    bytes += process_model_field(io, vec_size, read, "", text);
    for (size_t i = 0; i < vec_size; ++i)
    {
      T v;
      bytes += process_model_field(io, v, read, "", text);
      vec.push_back(v);
    }
  }
  else
  {
    vec_size = vec.size();
    bytes += process_model_field(io, vec_size, read, name_or_readable_field_template + "_size", text);
    size_t i = 0;
    for (T v : vec)
    {
      bytes += process_model_field(io, v, read, name_or_readable_field_template + "_" + std::to_string(i), text);
      ++i;
    }
  }
  return bytes;
}

template <typename F, typename S>
size_t process_model_field(
    io_buf& io, std::pair<F, S>& pair, bool read, const std::string& name_or_readable_field_template, bool text)
{
  size_t bytes = 0;
  bytes += process_model_field(io, pair.first, read, name_or_readable_field_template + "_first", text);
  bytes += process_model_field(io, pair.second, read, name_or_readable_field_template + "_second", text);
  return bytes;
}

template <typename T>
size_t process_model_field(
    io_buf& io, std::priority_queue<T>& pq, bool read, const std::string& name_or_readable_field_template, bool text)
{
  size_t bytes = 0;
  size_t queue_size;
  if (read)
  {
    bytes += process_model_field(io, queue_size, read, "", text);
    for (size_t i = 0; i < queue_size; ++i)
    {
      T v;
      bytes += process_model_field(io, v, read, "", text);
      pq.push(v);
    }
  }
  else
  {
    queue_size = pq.size();
    bytes += process_model_field(io, queue_size, read, name_or_readable_field_template + "_size", text);
    size_t i = 0;
    while (!pq.empty())
    {
      T v = pq.top();
      pq.pop();
      bytes += process_model_field(io, v, read, name_or_readable_field_template + "_" + std::to_string(i), text);
      ++i;
    }
  }
  return bytes;
}

template <typename K, typename V>
size_t process_model_field(
    io_buf& io, std::map<K, V>& map, bool read, const std::string& name_or_readable_field_template, bool text)
{
  size_t bytes = 0;
  size_t map_size;
  if (read)
  {
    bytes += process_model_field(io, map_size, read, "", text);
    for (size_t i = 0; i < map_size; ++i)
    {
      std::pair<K, V> pair;
      bytes += VW::model_utils::process_model_field(io, pair, read, "", text);
      map[pair.first] = pair.second;
    }
  }
  else
  {
    map_size = map.size();
    bytes += process_model_field(io, map_size, read, name_or_readable_field_template + "_size", text);
    size_t i = 0;
    for (auto& pair : map)
    {
      bytes += VW::model_utils::process_model_field(
          io, pair, read, name_or_readable_field_template + "_" + std::to_string(i), text);
      ++i;
    }
  }
  return bytes;
}

}  // namespace model_utils
}  // namespace VW
