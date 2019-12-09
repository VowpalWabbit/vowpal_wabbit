// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

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

using supported_options_types =
    typelist<unsigned int, int, size_t, uint64_t, float, double, char, std::string, bool, std::vector<int>,
        std::vector<size_t>, std::vector<float>, std::vector<double>, std::vector<char>, std::vector<std::string> >;
}  // namespace config
}  // namespace VW
