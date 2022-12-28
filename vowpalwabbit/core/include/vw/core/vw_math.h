#pragma once
#define NOMINMAX

#include "vw/common/future_compat.h"
#include "vw/common/vw_exception.h"
#include "vw/common/vw_throw.h"

#include <cassert>
#include <cmath>

// This is guarded behind c++17 as this header is only needed for
// std::clamp when C++17 is available.
// It just reduces the include cost of this header pre c++17
#ifdef HAS_STD17
#  include <algorithm>
#endif

namespace VW
{
namespace math
{
#define VW_DEFAULT_TOLERANCE 0.0001
constexpr float DEFAULT_FLOAT_TOLERANCE = static_cast<float>(VW_DEFAULT_TOLERANCE);

// Float/double comparison of arguments.
// Returns true if lhs and rhs are within tolerance of each other.
template <typename T>
bool are_same(T lhs, T rhs, T tolerance = VW_DEFAULT_TOLERANCE)
{
  return std::abs(lhs - rhs) < tolerance;
}

template <typename T>
bool are_same_rel(T lhs, T rhs, T tolerance = VW_DEFAULT_TOLERANCE)
{
  return std::abs(lhs - rhs) <= (tolerance * (std::abs(lhs) + std::abs(rhs)));
}

VW_STD14_CONSTEXPR inline int64_t factorial(int64_t n) noexcept
{
  int64_t result = 1;
  for (int64_t i = 2; i <= n; i++) { result *= i; }
  return result;
}

/// Both n and k must be non-zero
inline int64_t number_of_combinations_with_repetition(int64_t n, int64_t k)
{
  if ((n + k) > 21)
  {
    THROW_OR_RETURN_NORMAL("Magnitude of (n + k) is too large (> 21). Cannot compute combinations.", 0);
  }
  return factorial(n + k - 1) / (factorial(n - 1) * factorial(k));
}

inline int64_t number_of_permutations_with_repetition(int64_t n, int64_t k)
{
  return static_cast<int64_t>(std::pow(n, k));
}

constexpr inline float sign(float w) noexcept { return (w <= 0.f) ? -1.f : 1.f; }

/// C(n,k) = n!/(k!(n-k)!)
VW_STD14_CONSTEXPR inline int64_t choose(int64_t n, int64_t k) noexcept
{
  if (k > n) { return 0; }
  if (k < 0) { return 0; }
  if (k == n) { return 1; }
  if (k == 0 && n != 0) { return 1; }
  int64_t r = 1;
  for (int64_t d = 1; d <= k; ++d)
  {
    r *= n--;
    r /= d;
  }
  return r;
}

/**
 * @brief Clamp a value in the range [lower_bound, upper_bound]
 * @param num value to clamp
 * @param lower_bound lower bound of clamp range
 * @param upper_bound upper bound of clamp range
 */
template <typename T>
VW_STD14_CONSTEXPR T clamp(const T& num, const T& lower_bound, const T& upper_bound)
{
  assert(lower_bound <= upper_bound);
#ifdef HAS_STD17
  return std::clamp(num, lower_bound, upper_bound);
#else
  return std::max(lower_bound, std::min(num, upper_bound));
#endif
}

}  // namespace math
}  // namespace VW

#undef VW_DEFAULT_TOLERANCE
