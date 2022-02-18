// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "config/cli_options_serializer.h"

#include <sstream>
#include <vector>
#include "parse_primitives.h"

using namespace VW::config;

cli_options_serializer::cli_options_serializer(bool escape) : m_escape(escape) {}

template <typename T>
void serialize(std::stringstream& output, const typed_option<T>& typed_option, bool escape)
{
  std::stringstream ss;
  // 15 is the maximum number of decimal digits for a double.
  ss.precision(15);
  ss << typed_option.value();
  auto value = ss.str();

  if (escape && VW::contains_escapeable_chars(value))
  { output << " --" << typed_option.m_name << "=\"" << VW::escape_string(value) << "\""; }
  else
  {
    output << " --" << typed_option.m_name << "=" << value;
  }
}

template <>
void serialize<std::vector<std::string>>(
    std::stringstream& output, const typed_option<std::vector<std::string>>& typed_option, bool escape)
{
  auto vec = typed_option.value();
  if (!vec.empty())
  {
    for (auto const& value : vec)
    {
      if (escape && VW::contains_escapeable_chars(value))
      { output << " --" << typed_option.m_name << "=\"" << VW::escape_string(value) << "\""; }
      else
      {
        output << " --" << typed_option.m_name << "=" << value;
      }
    }
  }
}

template <>
void serialize<bool>(std::stringstream& output, const typed_option<bool>& typed_option, bool /*escape*/)
{
  if (typed_option.value()) { output << " --" << typed_option.m_name; }
}

void cli_options_serializer::add(base_option& option) { option.accept(*this); }

std::string cli_options_serializer::str() const { return m_output_stream.str(); }

size_t cli_options_serializer::size() const { return m_output_stream.str().size(); }

void cli_options_serializer::visit(typed_option<uint32_t>& option) { serialize(m_output_stream, option, m_escape); }
void cli_options_serializer::visit(typed_option<uint64_t>& option) { serialize(m_output_stream, option, m_escape); }
void cli_options_serializer::visit(typed_option<int32_t>& option) { serialize(m_output_stream, option, m_escape); }
void cli_options_serializer::visit(typed_option<int64_t>& option) { serialize(m_output_stream, option, m_escape); }
void cli_options_serializer::visit(typed_option<float>& option) { serialize(m_output_stream, option, m_escape); }
void cli_options_serializer::visit(typed_option<std::string>& option) { serialize(m_output_stream, option, m_escape); }
void cli_options_serializer::visit(typed_option<bool>& option) { serialize(m_output_stream, option, m_escape); }
void cli_options_serializer::visit(typed_option<std::vector<std::string>>& option)
{
  serialize(m_output_stream, option, m_escape);
}
