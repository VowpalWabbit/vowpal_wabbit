#pragma once

#include <cstdint>
#include <vector>

namespace exploration
{
  const uint64_t a = 0xeece66d5deece66dULL;
  const uint64_t c = 2147483647;

  const int bias = 127 << 23;

  union int_float
  {
    int32_t i;
    float f;
  };

  // uniform random between 0 and 1
  inline float uniform_random_merand48(uint64_t initial)
  {
    initial = a * initial + c;
    int_float temp;
    temp.i = ((initial >> 25) & 0x7FFFFF) | bias;
    return temp.f - 1;
  }

  template<typename InputIt>
  uint32_t sample_from_pdf(uint64_t seed, InputIt pdf_first, InputIt pdf_last, std::input_iterator_tag pdf_category)
  {
    // Create a discrete_distribution based on the returned weights. This class handles the
    // case where the sum of the weights is < or > 1, by normalizing agains the sum.
    float total = 0.f;
    for (InputIt pdf = pdf_first; pdf != pdf_last; ++pdf)
    {
#ifdef EXPLORE_NOEXCEPT
      *pdf = 0;
#else
      if (*pdf < 0)
        throw std::invalid_argument("Probabilities must be non-negative.");
#endif
      total += *pdf;
    }
    if (total == 0)
    {
#ifdef EXPLORE_NOEXCEPT
      // assume the first is the best
      *pdf_first = 1.f;
      total = 1.f;
#else
      throw std::invalid_argument("At least one score must be positive.");
#endif
    }
    
#ifndef EXPLORE_NOEXCEPT
    if (total < (1.f-1e-3f))
      throw std::invalid_argument("Total must some almost to one (1-1e-3f) positive.");
#endif

    float draw = total * uniform_random_merand48(seed);
    if (draw > total) //make very sure that draw can not be greater than total.
      draw = total;

    float sum = 0.f;
    uint32_t i = 0;
    for (InputIt pdf = pdf_first; pdf != pdf_last; ++pdf, ++i)
    {
      sum += *pdf;
      if (sum > draw)
        return i;
    }

    return i - 1;
  }

  template<typename InputIt>
  uint32_t sample_from_pdf(const char* seed, InputIt pdf_first, InputIt pdf_last, std::input_iterator_tag pdf_category)
  {
    uint64_t seed_hash = uniform_hash(seed, strlen(seed), 0);
    return sample_from_pdf(seed_hash, pdf_first, pdf_last);
  }

  template<typename InputPdfIt, typename InputScoreIt, typename OutputIt>
  void sample_from_pdf(uint64_t seed,
      InputPdfIt pdf_begin, InputPdfIt pdf_end, std::input_iterator_tag pdf_category,
      InputScoreIt scores_begin, InputScoreIt scores_end, std::random_access_iterator_tag scores_category,
      OutputIt ranking_begin, OutputIt ranking_end, std::output_iterator_tag ranking_category)
  {
    uint32_t chosen_action = sample_from_pdf(seed, pdf_begin, pdf_end);

    std::iota(ranking_begin, ranking_end, 0);

    // sort indexes based on comparing values in scores
    std::sort(ranking_begin, ranking_end,
      [&scores_begin, &scores_end](size_t i1, size_t i2) { return scores_begin[i1] > scores_end[i2]; });

    // swap top element with chosen one
    std::iter_swap(ranking_begin, ranking_end + chosen_action);
  }
} // end-of-namespace
