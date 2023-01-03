#pragma once

#include "vw/common/vw_exception.h"
#include "vw/common/vw_throw.h"

#include <limits>
#include <sstream>

namespace VW
{
template <typename RetType, typename InputType>
RetType cast_to_smaller_type(InputType input)
{
  static_assert(std::numeric_limits<RetType>::is_integer, "RetType must be an integer type");
  static_assert(std::numeric_limits<InputType>::is_integer, "InputType must be an integer type");
  static_assert(sizeof(InputType) >= sizeof(RetType), "RetType must be smaller than InputType");
  static_assert((std::numeric_limits<RetType>::is_signed && std::numeric_limits<InputType>::is_signed) ||
          (!std::numeric_limits<RetType>::is_signed && !std::numeric_limits<InputType>::is_signed),
      "RetType and InputType must be either both signed or both unsigned");
  static_assert(static_cast<InputType>(std::numeric_limits<RetType>::max()) <= std::numeric_limits<InputType>::max(),
      "RetType max value must be less than or equal to InputType max value");
  static_assert(static_cast<InputType>(std::numeric_limits<RetType>::min()) >= std::numeric_limits<InputType>::min(),
      "RetType min value must be more than or equal to InputType min value");

  if (input > static_cast<InputType>(std::numeric_limits<RetType>::max()))
  {
    std::stringstream ss;
    ss << "In cast_to_smaller_type '" << input << "' cannot be cast to smaller type as it is too large.";
    THROW_OR_RETURN(ss.str(), RetType{});
  }

  if (input < static_cast<InputType>(std::numeric_limits<RetType>::min()))
  {
    std::stringstream ss;
    ss << "In cast_to_smaller_type '" << input << "' cannot be cast to smaller type as it is too small.";
    THROW_OR_RETURN(ss.str(), RetType{});
  }

  return static_cast<RetType>(input);
}

template <typename RetType, typename InputType>
RetType cast_signed_to_unsigned(InputType input)
{
  static_assert(!std::numeric_limits<RetType>::is_signed, "RetType must be unsigned");
  static_assert(std::numeric_limits<InputType>::is_signed, "InputType must be signed");

  if (input < 0)
  {
    std::stringstream ss;
    ss << "In cast_signed_to_unsigned '" << input << "' cannot be cast to unsigned type as it is negative.";
    THROW_OR_RETURN(ss.str(), RetType{});
  }

  auto unsigned_input = static_cast<typename std::make_unsigned<InputType>::type>(input);
  return cast_to_smaller_type<RetType>(unsigned_input);
}

template <typename RetType, typename InputType>
RetType cast_unsigned_to_signed(InputType input)
{
  static_assert(!std::numeric_limits<InputType>::is_signed, "InputType must be unsigned");
  static_assert(std::numeric_limits<RetType>::is_signed, "RetType must be signed");
  static_assert(std::numeric_limits<RetType>::is_integer, "RetType must be an integer type");
  static_assert(std::numeric_limits<InputType>::is_integer, "InputType must be an integer type");

  const auto result = static_cast<RetType>(input);

  // If casting the result back to the input type is different (outside range, non-negative) or the result is negative
  // then the input was too large.
  if (static_cast<InputType>(result) != input || (result < RetType{}))
  {
    std::stringstream ss;
    ss << "In cast_unsigned_to_signed '" << input
       << "' cannot be cast to a signed type as it is outside of the bounds of the signed type.";
    THROW_OR_RETURN(ss.str(), RetType{});
  }

  return result;
}

}  // namespace VW
