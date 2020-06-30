#pragma once

#include <limits>

//static constexpr std::array<float, 75> pow_10_lookup_table;



extern constexpr std::array<float, 75> pow_10_lookup_table = {
    10e-37,
    10e-36,
    10e-35,
    10e-34,
    10e-33,
    10e-32,
    10e-31,
    10e-30,
    10e-29,
    10e-28,
    10e-27,
    10e-26,
    10e-25,
    10e-24,
    10e-23,
    10e-22,
    10e-21,
    10e-20,
    10e-19,
    10e-18,
    10e-17,
    10e-16,
    10e-15,
    10e-14,
    10e-13,
    10e-12,
    10e-11,
    10e-10,
    10e-9,
    10e-8,
    10e-7,
    10e-6,
    10e-5,
    10e-4,
    10e-3,
    10e-2,
    10e-1,
    10e0,  // 1 <---------------------
    10e1,
    10e2,
    10e3,
    10e4,
    10e5,
    10e6,
    10e7,
    10e8,
    10e9,
    10e10,
    10e11,
    10e12,
    10e13,
    10e14,
    10e15,
    10e16,
    10e17,
    10e18,
    10e19,
    10e20,
    10e21,
    10e22,
    10e23,
    10e24,
    10e25,
    10e26,
    10e27,
    10e28,
    10e29,
    10e30,
    10e31,
    10e32,
    10e33,
    10e34,
    10e35,
    10e36,
    10e37,
};


constexpr inline float fast_pow10(int8_t pow)
{
  // If the power would be above the range float can represent, return NaN.
  return (pow > 37 || pow < -37) ? std::numeric_limits<float>::quiet_NaN() : pow_10_lookup_table[pow + 37];
}

constexpr inline float fast_pow10_unsafe(int8_t pow) { return pow_10_lookup_table[pow + 37]; }
