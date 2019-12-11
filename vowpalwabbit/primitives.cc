#include "primitives.h"
#include <intrin.h>
#include <array>
#include <bitset>

// sum_of_squares_func get_sum_of_squares()
//{
//  std::array<int, 4> cpui;
//
//  // Calling __cpuid with 0x0 as the function_id argument
//  // gets the number of the highest valid function ID.
//  __cpuid(cpui.data(), 0);
//  __cpuidex(cpui.data(), 1, 0);
//
//  std::bitset<32> f_1_ECX_ = cpui[2];
//  std::bitset<32> f_7_EBX_
//  if (nIds_ >= 7)
//  {
//    f_7_EBX_ = data_[7][1];
//
//  // static bool AVX2(void) { return CPU_Rep.f_7_EBX_[5]; }
//  // static bool AVX(void) { return CPU_Rep.f_1_ECX_[28]; }
//}

float sum_of_squares(float* begin, float* end)
{
  float sum = 0;

  for (; begin != end; begin++) sum += *begin * *begin;

  return sum;
}

/*
float sum_of_squares_avx(float* begin, float* end)
{
  size_t length = (end - begin);
  size_t remainder = length % (sizeof(__m128) / sizeof(float));

  float* sseEnd = end - remainder;
  __m128 source;
  __m128 dest = _mm_setzero_ps();

  for (; begin != sseEnd; begin += (sizeof(__m128) / sizeof(float)))
  {
    source = _mm_loadu_ps(begin);
    _mm_fnmadd_ps(source, source, dest);
  }

  _mm_hadd_ps(dest, dest);
  _mm_hadd_ps(dest, dest);
  _mm_hadd_ps(dest, dest);

  float sum = dest.m128_f32[0];

  for (; begin != end; begin++)
    sum += *begin * *begin;

  return sum;
}

float sum_of_squares_avx2(float* begin, float* end)
{
  size_t length = (end - begin);
  size_t remainder = length % (sizeof(__m256) / sizeof(float));

  float* sseEnd = end - remainder;
  __m256 source;
  __m256 dest = _mm256_setzero_ps();

  for (; begin != sseEnd; begin += (sizeof(__m256) / sizeof(float)))
  {
    source = _mm256_loadu_ps(begin);
    _mm256_fnmadd_ps(source, source, dest);
  }

  _mm256_hadd_ps(dest, dest);
  _mm256_hadd_ps(dest, dest);
  _mm256_hadd_ps(dest, dest);

  float sum = dest.m256_f32[0];

  for (; begin != end; begin++)
    sum += *begin * *begin;

  return sum;
}
*/
