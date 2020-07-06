// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <array>
#include <cfloat>
#include <cstdint>
#include <limits>

#include "future_compat.h"

namespace VW
{
namespace details
{
constexpr float POW10_BASE = 10.f;
const constexpr uint8_t VALUES_BELOW_ZERO = FLT_MIN_10_EXP * -1;
// We add one to this because 0 isn't counted in FLT_MAX_10_EXP, but it takes a slot.
const constexpr uint8_t VALUES_ABOVE_AND_INCLUDING_ZERO = FLT_MAX_10_EXP + 1;
const constexpr uint8_t VALUES_ABOVE_ZERO = FLT_MAX_10_EXP;

constexpr float constexpr_int_pow10(uint8_t pow)
{
  return pow > VALUES_ABOVE_AND_INCLUDING_ZERO ? std::numeric_limits<float>::infinity()
                                               : pow == 0 ? 1 : POW10_BASE * constexpr_int_pow10(pow - 1);
}

constexpr float constexpr_negative_int_pow10(uint8_t pow)
{
  return pow > VALUES_BELOW_ZERO ? 0.f : pow == 0 ? 1 : constexpr_negative_int_pow10(pow - 1) / POW10_BASE;
}

constexpr float constexpr_negative_int_pow10_with_offset(uint8_t pow, uint8_t offset)
{
  return constexpr_negative_int_pow10(offset - pow);
}

template <std::size_t... Integers>
class index_sequence
{
};

template <std::size_t CurrentNum, std::size_t... Integers>
struct make_index_sequence : make_index_sequence<CurrentNum - 1, CurrentNum - 1, Integers...>
{
};

template <std::size_t... Integers>
struct make_index_sequence<0, Integers...> : index_sequence<Integers...>
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

static constexpr std::array<float, VALUES_ABOVE_AND_INCLUDING_ZERO> pow_10_positive_lookup_table =
    gen_positive_pow10s<VALUES_ABOVE_AND_INCLUDING_ZERO>(make_index_sequence<VALUES_ABOVE_AND_INCLUDING_ZERO>{});
static constexpr std::array<float, VALUES_BELOW_ZERO> pow_10_negative_lookup_table =
    gen_negative_pow10s<VALUES_BELOW_ZERO>(make_index_sequence<VALUES_BELOW_ZERO>{});

}  // namespace details

// std::array::operator[] const is made constexpr in C++14, so this can only be guaranteed to be constexpr when this standard
// is used.
VW_STD14_CONSTEXPR inline float fast_pow10(int8_t pow)
{
  // If the power would be above the range float can represent, return inf.
  return pow > details::VALUES_ABOVE_ZERO ? std::numeric_limits<float>::infinity()
                                          : (pow < -1 * details::VALUES_BELOW_ZERO)
          ? 0.f
          : pow >= 0 ? details::pow_10_positive_lookup_table[static_cast<size_t>(pow)]
                     : details::pow_10_negative_lookup_table[static_cast<size_t>(pow) +
                           static_cast<size_t>(details::VALUES_BELOW_ZERO)];
}

}  // namespace VW
