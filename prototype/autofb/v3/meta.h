#include "base.h"

// Provides functionality to implement "contracts" (see C++ concepts for a syntactically nicer idea.)

// The way this works is through the use of static_assert and various template metaprogramming library 
// functions, as well as a few templates to reduce the large amount of type-boilerplate that TML requires.

template <typename PredT, class = std::enable_if_t<std::is_same_v<const char*, decltype(PredT::message)>>>
struct extract_message_or_empty
{
  static constexpr char* message = PredT::message;
};

template <typename PredT>
struct extract_message_or_empty
{
  static constexpr const char* message = "";
};

template <typename PredT, class = std::enable_if_t<std::is_same_v<bool, decltype(PredT::value)>>>
struct extract_value_or_false : std::bool_constant<PredT::value>
{};

template <typename PredT>
struct extract_value_or_false : std::false_type
{};

template <typename condition, typename message, class = std::enable_if_t<std::is_same_v<const char*, decltype(message::message)>>>
struct assert
{
  static_assert(condition::value, message::message);
};

template <typename condition>
struct assert
{
  static_assert(condition::value);
};

//#define SUCH_THAT(predicate) , typename = std::enable_if_t<predicate>
