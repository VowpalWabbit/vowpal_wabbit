// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/config/cli_options_serializer.h"

#include <vector>

using namespace VW::config;

cli_options_serializer::cli_options_serializer() { m_output_stream.precision(15); }

template <typename T>
void serialize(std::stringstream& output, const typed_option<T>& typed_option)
{
  output << " --" << typed_option.m_name << " " << typed_option.value();
}

template <>
void serialize<std::vector<std::string>>(
    std::stringstream& output, const typed_option<std::vector<std::string>>& typed_option)
{
  auto vec = typed_option.value();
  if (!vec.empty())
  {
    for (auto const& value : vec)
    {
      output << " --" << typed_option.m_name;
      output << " " << value;
    }
  }
}

template <>
void serialize<bool>(std::stringstream& output, const typed_option<bool>& typed_option)
{
  if (typed_option.value()) { output << " --" << typed_option.m_name; }
}

void cli_options_serializer::add(base_option& option) { option.accept(*this); }

std::string cli_options_serializer::str() const { return m_output_stream.str(); }

size_t cli_options_serializer::size() const { return m_output_stream.str().size(); }

void cli_options_serializer::visit(typed_option<uint32_t>& option) { serialize(m_output_stream, option); }
void cli_options_serializer::visit(typed_option<uint64_t>& option) { serialize(m_output_stream, option); }
void cli_options_serializer::visit(typed_option<int32_t>& option) { serialize(m_output_stream, option); }
void cli_options_serializer::visit(typed_option<int64_t>& option) { serialize(m_output_stream, option); }
void cli_options_serializer::visit(typed_option<float>& option) { serialize(m_output_stream, option); }
void cli_options_serializer::visit(typed_option<std::string>& option) { serialize(m_output_stream, option); }
void cli_options_serializer::visit(typed_option<bool>& option) { serialize(m_output_stream, option); }
void cli_options_serializer::visit(typed_option<std::vector<std::string>>& option)
{
  serialize(m_output_stream, option);
}
