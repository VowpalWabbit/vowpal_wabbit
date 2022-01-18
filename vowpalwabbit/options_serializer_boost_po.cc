// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "options_serializer_boost_po.h"
#include "options.h"

using namespace VW::config;

std::string options_serializer_boost_po::str() const { return m_output_stream.str(); }

size_t options_serializer_boost_po::size() const { return m_output_stream.str().size(); }

void options_serializer_boost_po::add(base_option& option) { details::handle_option_by_type(option, *this); }

void options_serializer_boost_po::handle(typed_option<uint32_t>& option) { serialize(option); }
void options_serializer_boost_po::handle(typed_option<int>& option) { serialize(option); }
void options_serializer_boost_po::handle(typed_option<size_t>& option) { serialize(option); }
void options_serializer_boost_po::handle(typed_option<uint64_t>& option) { serialize(option); }
void options_serializer_boost_po::handle(typed_option<int64_t>& option) { serialize(option); }
void options_serializer_boost_po::handle(typed_option<float>& option) { serialize(option); }
void options_serializer_boost_po::handle(typed_option<double>& option) { serialize(option); }
void options_serializer_boost_po::handle(typed_option<std::string>& option) { serialize(option); }
void options_serializer_boost_po::handle(typed_option<bool>& option) { serialize(option); }
void options_serializer_boost_po::handle(typed_option<std::vector<std::string>>& option) { serialize(option); }

template <>
void options_serializer_boost_po::serialize<bool>(typed_option<bool>& typed_option)
{
  if (typed_option.value()) { m_output_stream << " --" << typed_option.m_name; }
}
