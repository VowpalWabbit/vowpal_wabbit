// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "options_serializer_boost_po.h"

#include <memory>

using namespace VW::config;

std::string options_serializer_boost_po::str() { return m_output_stream.str(); }

std::unique_ptr<const char> options_serializer_boost_po::data() {
  auto str = m_output_stream.str();
  char* buffer = new char[str.size() + 1];
  str.copy(buffer, str.size());
  buffer[str.size()] = '\0';
  return std::unique_ptr<const char>(buffer);
}

size_t options_serializer_boost_po::size() { return m_output_stream.str().size(); }

void options_serializer_boost_po::add(base_option& option) { add_impl<supported_options_types>(option); }

template <>
void options_serializer_boost_po::serialize<bool>(typed_option<bool>& typed_option)
{
  if (typed_option.value())
  {
    m_output_stream << " --" << typed_option.m_name;
  }
}

template <>
void options_serializer_boost_po::add_impl<typelist<>>(base_option&)
{
  THROW("That is an unsupported option type.");
}
