// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "io_buf.h"

#include <fmt/format.h>

namespace VW
{
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
size_t process_model_field(io_buf& io, T& var, bool read, const std::string& name_or_readable_field_template, bool text)
{
  // It is not valid to read a text based field.
  assert(!(read && text));

  auto* data = reinterpret_cast<char*>(&var);
  auto len = sizeof(var);

  if (read) { return details::check_length_matches(io.bin_read_fixed(data, len), len); }
  if (text) { return details::write_text_mode_output(io, var, name_or_readable_field_template); }
  // If not read or text we are just writing the binary data.
  return details::check_length_matches(io.bin_write_fixed(data, len), len);
}

}  // namespace model_utils
}  // namespace VW