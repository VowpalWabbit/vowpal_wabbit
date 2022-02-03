// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "options_serializer_boost_po.h"

using namespace VW::config;

template <typename T>
void serialize(std::stringstream& output, typed_option<T>& typed_option)
{
  output << " --" << typed_option.m_name << " " << typed_option.value();
}

template <typename T>
void serialize(std::stringstream& output, typed_option<std::vector<T>>& typed_option)
{
  auto vec = typed_option.value();
  if (vec.size() > 0)
  {
    for (auto const& value : vec)
    {
      output << " --" << typed_option.m_name;
      output << " " << value;
    }
  }
}

template <>
void serialize<bool>(std::stringstream& output, typed_option<bool>& typed_option)
{
  if (typed_option.value()) { output << " --" << typed_option.m_name; }
}

void options_serializer_boost_po::add(base_option& option) { option.accept(*this); }

std::string options_serializer_boost_po::str() const { return m_output_stream.str(); }

size_t options_serializer_boost_po::size() const { return m_output_stream.str().size(); }

void options_serializer_boost_po::visit(typed_option<uint32_t>& option) { serialize(m_output_stream, option); }
void options_serializer_boost_po::visit(typed_option<uint64_t>& option) { serialize(m_output_stream, option); }
void options_serializer_boost_po::visit(typed_option<int32_t>& option) { serialize(m_output_stream, option); }
void options_serializer_boost_po::visit(typed_option<int64_t>& option) { serialize(m_output_stream, option); }
void options_serializer_boost_po::visit(typed_option<float>& option) { serialize(m_output_stream, option); }
void options_serializer_boost_po::visit(typed_option<std::string>& option) { serialize(m_output_stream, option); }
void options_serializer_boost_po::visit(typed_option<bool>& option) { serialize(m_output_stream, option); }
void options_serializer_boost_po::visit(typed_option<std::vector<std::string>>& option)
{
  serialize(m_output_stream, option);
}
