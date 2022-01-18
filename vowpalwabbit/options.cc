// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "options.h"
#include "options_types.h"

namespace VW
{
namespace config
{
  namespace details
  {

template <typename T>
bool handle_option_if_t(base_option& opt, typed_option_handler& handler)
{
  if (opt.m_type_hash == typeid(T).hash_code())
  {
    auto* typed = dynamic_cast<typed_option<T>*>(&opt);
    assert(typed != nullptr);
    handler.handle(*typed);
    return true;
  }

  return false;
}

  template <typename TTypes>
  void handle_option_impl(base_option& opt, typed_option_handler& handler)
  {
    if (handle_option_if_t<typename TTypes::head>(opt, handler)) { return; }
    handle_option_impl<typename TTypes::tail>(opt, handler);
  }

template <>
void handle_option_impl<typelist<>>(
    base_option& opt, typed_option_handler& /*handler*/)
{
  THROW(fmt::format("Option '{}' has an unsupported option type.", opt.m_name));
}

void handle_option_by_type(base_option& opt, typed_option_handler& handler)
{
  handle_option_impl<supported_options_types>(opt, handler);
}
}
}  // namespace config
}  // namespace VW
