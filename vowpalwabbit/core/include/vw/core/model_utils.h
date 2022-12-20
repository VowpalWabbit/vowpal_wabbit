// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "vw/core/io_buf.h"
#include "vw/core/memory.h"
#include "vw/core/v_array.h"

#include <fmt/format.h>

#include <map>
#include <queue>
#include <set>
#include <string>
#include <type_traits>

namespace VW
{
namespace model_utils
{
namespace details
{
inline size_t check_length_matches(size_t actual_len, size_t expected_len)
{
  if (expected_len > 0 && actual_len != expected_len) { THROW_OR_RETURN("Unexpected end of file encountered.", 0); }
  return actual_len;
}

template <typename T>
size_t write_text_mode_output(io_buf& io, const T& var, const std::string& name_or_readable_field_template)
{
  if (name_or_readable_field_template.empty()) { return 0; }

  std::string message;
  // If the user has supplied a template string then use that.
  if (name_or_readable_field_template.find("{}") != VW::string_view::npos)
  {
#if FMT_VERSION >= 80000
    message = fmt::format(fmt::runtime(name_or_readable_field_template), var);
#else
    message = fmt::format(name_or_readable_field_template, var);
#endif
  }
  else
  {
    // Use the default template string.
    message = fmt::format("{} = {}\n", name_or_readable_field_template, var);
  }

  return details::check_length_matches(io.bin_write_fixed(message.c_str(), message.size()), message.size());
}
}  // namespace details

/**
 * @brief This function is the uniform interface reading a variable from the model file. The value must be a POD type.
 *
 * @tparam T type of value to serialize or deserialize. This is important as it determines the number of bytes to be
 * processed.
 * @param io model file io_buf to read from or write to
 * @param var Variable to write into for the deserialized value. Must be a type which supports
 * validly bit casting to a sequence of bytes. Does not need to be initialized.
 * @return size_t the number of bytes read
 */
template <typename T,
    typename std::enable_if<
        !std::is_pointer<T>::value && std::is_trivial<T>::value && std::is_standard_layout<T>::value, bool>::type =
        true>
size_t read_model_field(io_buf& io, T& var)
{
  auto* data = reinterpret_cast<char*>(&var);
  auto len = sizeof(var);

  return details::check_length_matches(io.bin_read_fixed(data, len), len);
}

/**
 * @brief This function is the uniform interface for serializing a variable to
 * the model file. The value must be a POD type.
 *
 * @tparam T type of value to serialize or deserialize. This is important as it determines the number of bytes to be
 * processed.
 * @param io model file io_buf to read from or write to
 * @param var value to serialize. Must be a type which supports
 * validly bit casting to a sequence of bytes. Must be initialized.
 * @param name_or_readable_field_template (Only used when text == true) The name
 * of the variable or a template string to use when writing.
 *   - If empty, then no output is written for this call
 *   - If the string contains a "{}", then the variable value will be substituted for that. Note: more than one "{}" in
 * the string will result in a runtime error due to the templating failure.
 *   - Otherwise, if the string is non-empty and does not contain a "{}" the value will be used as the name of the
 * variable and be outputted in the form `("{} = {}\n", name_or_readable_field_template, var)`
 * @param text whether to write in text mode or binary mode.
 * @return size_t the number of bytes written
 */
template <typename T,
    typename std::enable_if<
        !std::is_pointer<T>::value && std::is_trivial<T>::value && std::is_standard_layout<T>::value, bool>::type =
        true>
size_t write_model_field(io_buf& io, const T& var, const std::string& name_or_readable_field_template, bool text)
{
  if (text) { return details::write_text_mode_output(io, var, name_or_readable_field_template); }

  const auto* data = reinterpret_cast<const char*>(&var);
  auto len = sizeof(var);

  // If not text we are just writing the binary data.
  return details::check_length_matches(io.bin_write_fixed(data, len), len);
}

size_t read_model_field(io_buf&, std::string&);
size_t write_model_field(io_buf&, const std::string&, const std::string&, bool);
template <typename T>
size_t read_model_field(io_buf&, std::set<T>&);
template <typename T>
size_t write_model_field(io_buf&, const std::set<T>&, const std::string&, bool);
template <typename T>
size_t read_model_field(io_buf&, std::vector<T>&);
template <typename T>
size_t write_model_field(io_buf&, const std::vector<T>&, const std::string&, bool);
template <typename T>
size_t read_model_field(io_buf&, v_array<T>&);
template <typename T>
size_t write_model_field(io_buf&, const v_array<T>&, const std::string&, bool);
template <typename F, typename S>
size_t read_model_field(io_buf&, std::pair<F, S>&);
template <typename F, typename S>
size_t write_model_field(io_buf&, const std::pair<F, S>&, const std::string&, bool);
template <typename T>
size_t read_model_field(io_buf&, std::priority_queue<T>&);
template <typename T>
size_t write_model_field(io_buf&, const std::priority_queue<T>&, const std::string&, bool);
template <typename K, typename V>
size_t read_model_field(io_buf&, std::map<K, V>&);
template <typename K, typename V>
size_t write_model_field(io_buf&, const std::map<K, V>&, const std::string&, bool);
template <typename T>
size_t read_model_field(io_buf&, std::unique_ptr<T>&);
template <typename T>
size_t write_model_field(io_buf&, const std::unique_ptr<T>&, const std::string&, bool);

inline size_t read_model_field(io_buf& io, std::string& str)
{
  size_t bytes = 0;
  uint32_t str_size;
  bytes += read_model_field(io, str_size);
  char* cs = nullptr;
  bytes += io.buf_read(cs, str_size * sizeof(char));
  str = std::string(cs, str_size);
  return bytes;
}

inline size_t write_model_field(io_buf& io, const std::string& str, const std::string& upstream_name, bool text)
{
  if (upstream_name.find("{}") != std::string::npos) { THROW_OR_RETURN("Field template not allowed for string.", 0); }
  size_t bytes = 0;
  uint32_t str_size = static_cast<uint32_t>(str.size());
  bytes += write_model_field(io, str_size, upstream_name + ".size()", text);
  std::string message;
  if (text) { message = fmt::format("{} = {}\n", upstream_name, str); }
  else { message = str; }
  bytes += io.bin_write_fixed(message.c_str(), message.size());
  return bytes;
}

template <typename T>
size_t read_model_field(io_buf& io, std::set<T>& set)
{
  size_t bytes = 0;
  uint32_t set_size;
  bytes += read_model_field(io, set_size);
  for (uint32_t i = 0; i < set_size; ++i)
  {
    T v;
    bytes += read_model_field(io, v);
    set.insert(v);
  }
  return bytes;
}

template <typename T>
size_t write_model_field(io_buf& io, const std::set<T>& set, const std::string& upstream_name, bool text)
{
  if (upstream_name.find("{}") != std::string::npos) { THROW_OR_RETURN("Field template not allowed for set.", 0); }
  size_t bytes = 0;
  uint32_t set_size = static_cast<uint32_t>(set.size());
  bytes += write_model_field(io, set_size, upstream_name + ".size()", text);
  uint32_t i = 0;
  for (const T& v : set)
  {
    bytes += write_model_field(io, v, fmt::format("{}[{}]", upstream_name, i), text);
    ++i;
  }
  return bytes;
}

template <typename T>
size_t read_model_field(io_buf& io, std::vector<T>& vec)
{
  size_t bytes = 0;
  uint32_t vec_size;
  bytes += read_model_field(io, vec_size);
  for (uint32_t i = 0; i < vec_size; ++i)
  {
    T v;
    bytes += read_model_field(io, v);
    vec.push_back(std::move(v));
  }
  return bytes;
}

template <typename T>
size_t write_model_field(io_buf& io, const std::vector<T>& vec, const std::string& upstream_name, bool text)
{
  if (upstream_name.find("{}") != std::string::npos) { THROW_OR_RETURN("Field template not allowed for vector.", 0); }
  size_t bytes = 0;
  uint32_t vec_size = static_cast<uint32_t>(vec.size());
  bytes += write_model_field(io, vec_size, upstream_name + ".size()", text);
  for (uint32_t i = 0; i < vec_size; ++i)
  {
    bytes += write_model_field(io, vec[i], fmt::format("{}[{}]", upstream_name, i), text);
  }
  return bytes;
}

template <typename T>
size_t read_model_field(io_buf& io, v_array<T>& vec)
{
  size_t bytes = 0;
  uint32_t vec_size;
  bytes += read_model_field(io, vec_size);
  for (uint32_t i = 0; i < vec_size; ++i)
  {
    T v;
    bytes += read_model_field(io, v);
    vec.push_back(v);
  }
  return bytes;
}

template <typename T>
size_t write_model_field(io_buf& io, const v_array<T>& vec, const std::string& upstream_name, bool text)
{
  if (upstream_name.find("{}") != std::string::npos) { THROW_OR_RETURN("Field template not allowed for v_array.", 0); }
  size_t bytes = 0;
  uint32_t vec_size = static_cast<uint32_t>(vec.size());
  bytes += write_model_field(io, vec_size, upstream_name + ".size()", text);
  for (uint32_t i = 0; i < vec_size; ++i)
  {
    bytes += write_model_field(io, vec[i], fmt::format("{}[{}]", upstream_name, i), text);
  }
  return bytes;
}

template <typename F, typename S>
size_t read_model_field(io_buf& io, std::pair<F, S>& pair)
{
  size_t bytes = 0;
  bytes += read_model_field(io, pair.first);
  bytes += read_model_field(io, pair.second);
  return bytes;
}

template <typename F, typename S>
size_t write_model_field(io_buf& io, const std::pair<F, S>& pair, const std::string& upstream_name, bool text)
{
  if (upstream_name.find("{}") != std::string::npos) { THROW_OR_RETURN("Field template not allowed for pair.", 0); }
  size_t bytes = 0;
  bytes += write_model_field(io, pair.first, upstream_name + ".first", text);
  bytes += write_model_field(io, pair.second, upstream_name + ".second", text);
  return bytes;
}

template <typename T>
size_t read_model_field(io_buf& io, std::priority_queue<T>& pq)
{
  size_t bytes = 0;
  uint32_t queue_size;
  bytes += read_model_field(io, queue_size);
  for (uint32_t i = 0; i < queue_size; ++i)
  {
    T v;
    bytes += read_model_field(io, v);
    pq.push(v);
  }
  return bytes;
}

template <typename T>
size_t write_model_field(io_buf& io, const std::priority_queue<T>& pq, const std::string& upstream_name, bool text)
{
  if (upstream_name.find("{}") != std::string::npos)
  {
    THROW_OR_RETURN("Field template not allowed for priority_queue.", 0);
  }
  std::priority_queue<T> pq_cp = pq;
  size_t bytes = 0;
  uint32_t queue_size = static_cast<uint32_t>(pq_cp.size());
  bytes += write_model_field(io, queue_size, upstream_name + ".size()", text);
  uint32_t i = 0;
  while (!pq_cp.empty())
  {
    const T& v = pq_cp.top();
    bytes += write_model_field(io, v, fmt::format("{}[{}]", upstream_name, i), text);
    pq_cp.pop();
    ++i;
  }
  return bytes;
}

template <typename K, typename V>
size_t read_model_field(io_buf& io, std::map<K, V>& map)
{
  size_t bytes = 0;
  uint32_t map_size;
  bytes += read_model_field(io, map_size);
  for (uint32_t i = 0; i < map_size; ++i)
  {
    std::pair<K, V> pair;
    bytes += read_model_field(io, pair);
    map[pair.first] = pair.second;
  }
  return bytes;
}

template <typename K, typename V>
size_t write_model_field(io_buf& io, const std::map<K, V>& map, const std::string& upstream_name, bool text)
{
  if (upstream_name.find("{}") != std::string::npos) { THROW_OR_RETURN("Field template not allowed for map.", 0); }
  size_t bytes = 0;
  uint32_t map_size = static_cast<uint32_t>(map.size());
  bytes += write_model_field(io, map_size, upstream_name + ".size()", text);
  uint32_t i = 0;
  for (const auto& pair : map)
  {
    bytes += write_model_field(io, pair.first, fmt::format("{}.key{}", upstream_name, i), text);
    bytes += write_model_field(io, pair.second, fmt::format("{}[key{}]", upstream_name, i), text);
    ++i;
  }
  return bytes;
}

template <typename T>
size_t read_model_field(io_buf& io, std::unique_ptr<T>& ptr)
{
  size_t bytes = 0;
  bool is_null{};
  bytes += read_model_field(io, is_null);

  if (is_null)
  {
    ptr = nullptr;
    return bytes;
  }

  ptr = VW::make_unique<T>();
  bytes += read_model_field(io, *ptr);
  return bytes;
}

template <typename T>
size_t write_model_field(io_buf& io, const std::unique_ptr<T>& ptr, const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  if (ptr == nullptr)
  {
    bytes += write_model_field(io, true, fmt::format("{}.is_null()", upstream_name), text);
    return bytes;
  }

  bytes += write_model_field(io, false, fmt::format("{}.is_null()", upstream_name), text);
  bytes += write_model_field(io, *ptr, upstream_name, text);

  return bytes;
}

}  // namespace model_utils
}  // namespace VW
