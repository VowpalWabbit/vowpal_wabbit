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

// is_scalar detection relies on only accepting std::vector<T> for vector types. If that changes, update
// is_scalar_impl below.
using supported_options_types =
    typelist<unsigned int, int, size_t, uint64_t, float, double, char, std::string, bool, std::vector<int>,
        std::vector<size_t>, std::vector<float>, std::vector<double>, std::vector<char>, std::vector<std::string> >;


namespace is_scalar_impl {
  template <typename T>       struct is_scalar_option_type                        : std::true_type{};
  template <typename... Args> struct is_scalar_option_type<std::vector <Args...>> : std::false_type{};
}

template <typename T> struct is_scalar_option_type {
  static constexpr bool const value = is_scalar_impl::is_scalar_option_type<std::decay_t<T>>::value;
};

}  // namespace config
}  // namespace VW
