// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <cstddef>

namespace VW
{
namespace config
{
template <typename... Types>
struct typelist
{
private:
  template <class Head, class... Tail>
  struct head_impl
  {
    using type = Head;
  };
  template <class Head, class... Tail>
  struct tail_impl
  {
    using type = typelist<Tail...>;
  };

public:
  using head = typename head_impl<Types...>::type;
  using tail = typename tail_impl<Types...>::type;
};

// is_scalar detection relies on only accepting std::vector<T> for vector types. If that changes, update
// is_scalar_impl below.
using supported_options_types =
    typelist<uint32_t, uint64_t, int32_t, int64_t, float, std::string, bool, std::vector<std::string>>;

}  // namespace config
}  // namespace VW
