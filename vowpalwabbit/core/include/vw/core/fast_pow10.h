// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/common/future_compat.h"

#include <array>
#include <cfloat>
#include <cstdint>
#include <limits>

// The reason for this implementation is that for specific scenarios using a
// lookup table can drastically improve performance over the generic std::pow
// implemenation. In the parse_float function there is a place where we raise 10
// to some whole number and store the result in a float. This means there can
// only be approximately 80 values possible for this calculation. This can
// result in approximately a 50% reduction in runtime, which is why all of this
// complicated work was done. Furthermore, care was taken to be able to build
// this lookup table at compiletime that adheres to the systems defined bounds
// for float precision (see FLT_MIN_10_EXP and FLT_MAX_10_EXP)

namespace VW
{
namespace details
{
constexpr float POW10_BASE = 10.f;
const constexpr uint8_t VALUES_BELOW_ZERO = FLT_MIN_10_EXP * -1;
// We add one to this because 0 isn't counted in FLT_MAX_10_EXP, but it takes a slot.
const constexpr uint8_t VALUES_ABOVE_AND_INCLUDING_ZERO = FLT_MAX_10_EXP + 1;
const constexpr uint8_t VALUES_ABOVE_ZERO = FLT_MAX_10_EXP;

constexpr float constexpr_int_pow10(uint8_t exponent)
{
  return exponent > VALUES_ABOVE_AND_INCLUDING_ZERO ? std::numeric_limits<float>::infinity()
      : exponent == 0                               ? 1
                                                    : POW10_BASE * constexpr_int_pow10(exponent - 1);
}

constexpr float constexpr_negative_int_pow10(uint8_t exponent)
{
  return exponent > VALUES_BELOW_ZERO ? 0.f
      : exponent == 0                 ? 1
                                      : constexpr_negative_int_pow10(exponent - 1) / POW10_BASE;
}

// This function is a simple workaround for the fact it is tricky to generate compile time sequences of numbers that
// count down to zero from a number instead of up to a number. Please feel free to remove this if a decreasing sequence
// can be used instead
constexpr float constexpr_negative_int_pow10_with_offset(uint8_t exponent, uint8_t offset)
{
  return constexpr_negative_int_pow10(offset - exponent);
}

template <std::size_t... Integers>
class index_sequence
{
};

template <std::size_t CurrentNum, std::size_t... Integers>
class make_index_sequence : public make_index_sequence<CurrentNum - 1, CurrentNum - 1, Integers...>
{
};

template <std::size_t... Integers>
class make_index_sequence<0, Integers...> : public index_sequence<Integers...>
{
};

template <std::size_t ArrayLength, std::size_t... IntegerSequence>
constexpr std::array<float, ArrayLength> gen_negative_pow10s(index_sequence<IntegerSequence...> /*integer_sequence*/)
{
  return {constexpr_negative_int_pow10_with_offset(IntegerSequence, ArrayLength)...};
}

template <std::size_t ArrayLength, std::size_t... IntegerSequence>
constexpr std::array<float, ArrayLength> gen_positive_pow10s(index_sequence<IntegerSequence...> /*integer_sequence*/)
{
  return {constexpr_int_pow10(IntegerSequence)...};
}

static constexpr std::array<float, VALUES_ABOVE_AND_INCLUDING_ZERO> POW_10_POSITIVE_LOOKUP_TABLE =
    gen_positive_pow10s<VALUES_ABOVE_AND_INCLUDING_ZERO>(make_index_sequence<VALUES_ABOVE_AND_INCLUDING_ZERO>{});
static constexpr std::array<float, VALUES_BELOW_ZERO> POW_10_NEGATIVE_LOOKUP_TABLE =
    gen_negative_pow10s<VALUES_BELOW_ZERO>(make_index_sequence<VALUES_BELOW_ZERO>{});

}  // namespace details

// std::array::operator[] const is made constexpr in C++14, so this can only be guaranteed to be constexpr when this
// standard is used.
VW_STD14_CONSTEXPR inline float fast_pow10(int8_t exponent)
{
  // If the result would be above the range float can represent, return inf.
  return exponent > details::VALUES_ABOVE_ZERO       ? std::numeric_limits<float>::infinity()
      : (exponent < -1 * details::VALUES_BELOW_ZERO) ? 0.f
      : exponent >= 0 ? details::POW_10_POSITIVE_LOOKUP_TABLE[static_cast<std::size_t>(exponent)]
                      : details::POW_10_NEGATIVE_LOOKUP_TABLE[static_cast<std::size_t>(exponent) +
                            static_cast<std::size_t>(details::VALUES_BELOW_ZERO)];
}

}  // namespace VW
