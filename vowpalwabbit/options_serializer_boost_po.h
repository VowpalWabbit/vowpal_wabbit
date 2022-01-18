// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "options.h"

#include "options_types.h"

#include "vw_exception.h"

#include <sstream>

namespace VW
{
namespace config
{
struct options_serializer_boost_po : options_serializer_i, details::typed_option_handler
{
  options_serializer_boost_po() { m_output_stream.precision(15); }

  void add(base_option& option) override;
  std::string str() const override;
  size_t size() const override;

  void handle(typed_option<uint32_t>& option) override;
  void handle(typed_option<int>& option) override;
  void handle(typed_option<size_t>& option) override;
  void handle(typed_option<uint64_t>& option) override;
  void handle(typed_option<int64_t>& option) override;
  void handle(typed_option<float>& option) override;
  void handle(typed_option<double>& option) override;
  void handle(typed_option<std::string>& option) override;
  void handle(typed_option<bool>& option) override;
  void handle(typed_option<std::vector<std::string>>& option) override;

private:
  template <typename T>
  void serialize(typed_option<T>& typed_option)
  {
    m_output_stream << " --" << typed_option.m_name << " " << typed_option.value();
  }

  template <typename T>
  void serialize(typed_option<std::vector<T>>& typed_option)
  {
    auto vec = typed_option.value();
    if (vec.size() > 0)
    {
      for (auto const& value : vec)
      {
        m_output_stream << " --" << typed_option.m_name;
        m_output_stream << " " << value;
      }
    }
  }

  std::stringstream m_output_stream;
};

template <>
void options_serializer_boost_po::serialize<bool>(typed_option<bool>& typed_argument);

}  // namespace config
}  // namespace VW
