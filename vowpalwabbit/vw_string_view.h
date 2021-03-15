#pragma once

#include "hashstring.h"

#include <boost/version.hpp>

#if !defined(_M_CEE) && !defined(_MANAGED)
# include <fmt/format.h>
#endif

#if BOOST_VERSION < 106100
#  include <boost/utility/string_ref.hpp>
namespace VW
{
using string_view = boost::string_ref;
}
#else
#  include <boost/utility/string_view.hpp>
namespace VW
{
using string_view = boost::string_view;
}
#endif

namespace std
{
// boost VW::string_view hashing isn't available until 1.69. Implement our own for now
template <>
struct hash<VW::string_view>
{
  size_t operator()(const VW::string_view& s) const { return hashstring(s.begin(), s.length(), 0); }
};
}  // namespace std

#if !defined(_M_CEE) && !defined(_MANAGED)
namespace fmt
{
// Enable VW::string_view in fmt calls (uses the fmt::string_view formatter underneath)
template<>
struct formatter<VW::string_view> : formatter<fmt::string_view>
{
  template <typename FormatContext>
  auto format(const VW::string_view& sv, FormatContext& ctx) -> decltype(ctx.out()) {
    return formatter<fmt::string_view>::format({sv.begin(), sv.size()}, ctx);
  }
};
}
#endif
