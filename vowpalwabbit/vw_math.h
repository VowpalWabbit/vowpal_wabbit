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

VW_STD14_CONSTEXPR inline size_t factorial(size_t n) noexcept
{
  size_t result = 1;
  for (size_t i = 2; i <= n; i++) { result *= i; }
  return result;
}

/// Both n and k must be non-zero
VW_STD14_CONSTEXPR inline size_t number_of_combinations_with_repetition(size_t n, size_t k) noexcept
{
  return factorial(n + k + 1) / (factorial(n - 1) * factorial(k));
}

inline size_t number_of_permutations_with_repetition(size_t n, size_t k) { return static_cast<size_t>(std::pow(n, k)); }

constexpr inline float sign(float w) noexcept { return (w <= 0.f) ? -1.f : 1.f; }

/// C(n,k) = n!/(k!(n-k)!)
VW_STD14_CONSTEXPR inline int64_t choose(int64_t n, int64_t k) noexcept
{
  if (k > n) return 0;
  if (k < 0) return 0;
  if (k == n) return 1;
  if (k == 0 && n != 0) return 1;
  int64_t r = 1;
  for (int64_t d = 1; d <= k; ++d)
  {
    r *= n--;
    r /= d;
  }
  return r;
}

}  // namespace math
}  // namespace VW
