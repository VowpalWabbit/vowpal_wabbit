#pragma once

#include <cmath>

namespace VW
{
namespace math
{
#define DEFAULT_TOLERANCE 0.0001
constexpr float DEFAULT_FLOAT_TOLERANCE = static_cast<float>(DEFAULT_TOLERANCE);

// Float/double comparison of arguments.
// Returns true if lhs and rhs are within tolerance of each other.
template <typename T>
bool are_same(T lhs, T rhs, T tolerance = DEFAULT_TOLERANCE)
{
  return std::abs(lhs - rhs) < tolerance;
}

template <typename T>
bool are_same_rel(T lhs, T rhs, T tolerance = DEFAULT_TOLERANCE)
{
  return std::abs(lhs - rhs) <= (tolerance * (std::abs(lhs) + std::abs(rhs)));
}

}  // namespace math
}  // namespace VW
