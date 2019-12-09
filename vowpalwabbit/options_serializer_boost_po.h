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
struct options_serializer_boost_po : options_serializer_i
{
  options_serializer_boost_po() { m_output_stream.precision(15); }

  virtual void add(base_option& option) override;
  virtual std::string str() override;
  virtual const char* data() override;
  virtual size_t size() override;

 private:
  template <typename T>
  bool serialize_if_t(base_option& base_option)
  {
    if (base_option.m_type_hash == typeid(T).hash_code())
    {
      auto typed = dynamic_cast<typed_option<T>&>(base_option);
      serialize(typed);
      return true;
    }

    return false;
  }

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

  template <typename TTypes>
  void add_impl(base_option& options)
  {
    if (serialize_if_t<typename TTypes::head>(options))
    {
      return;
    }
    add_impl<typename TTypes::tail>(options);
  }

  std::stringstream m_output_stream;
};

template <>
void options_serializer_boost_po::serialize<bool>(typed_option<bool>& typed_argument);

template <>
void options_serializer_boost_po::add_impl<typelist<>>(base_option& options);
}  // namespace config
}  // namespace VW
