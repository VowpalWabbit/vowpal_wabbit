#include "hash.h"

#include <string.h>
#include <algorithm>
#include <stdexcept>
#include <numeric>

#include "sampling.h"

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
  float uniform_random_merand48(uint64_t initial)
  {
    initial = a * initial + c;
    int_float temp;
    temp.i = ((initial >> 25) & 0x7FFFFF) | bias;
    return temp.f - 1;
  }

  sample sample_from_pdf(const char* seed, const float* pdf, uint32_t len)
  {
    uint64_t seed_hash = uniform_hash(seed, strlen(seed), 0);
    return sample_from_pdf(seed_hash, pdf, len);
  }

  sample sample_from_pdf(uint64_t seed, const float* pdf, uint32_t len)
  {
    // Create a discrete_distribution based on the returned weights. This class handles the
    // case where the sum of the weights is < or > 1, by normalizing agains the sum.
    float total = 0.f;
    for (uint32_t i = 0; i < len; i++)
    {
      if (pdf[i] < 0)
        throw std::invalid_argument("Probabilities must be non-negative.");
      total += pdf[i];
    }
    if (total == 0)
      throw std::invalid_argument("At least one score must be positive.");

    float draw = total * uniform_random_merand48(seed);
    if (draw > total) //make very sure that draw can not be greater than total.
      draw = total;

    float action_probability = 0.f;
    uint32_t action_index = len - 1;
    float sum = 0.f;
    for (uint32_t i = 0; i < len; i++)
    {
      sum += pdf[i];
      if (sum > draw)
      {
        action_index = i;
        action_probability = pdf[i] / total;
        break;
      }
    }

    return { action_probability, action_index };
  }

  void sample_from_pdf(const char* seed, const float* pdf, const float* scores, uint32_t len, uint32_t* out_ranking, float* out_probability)
  {
    uint64_t seed_hash = uniform_hash(seed, strlen(seed), 0);
    return sample_from_pdf(seed_hash, pdf, scores, len, out_ranking, out_probability);
  }

  void sample_from_pdf(uint64_t seed, const float* pdf, const float* scores, uint32_t len, uint32_t* out_ranking, float* out_probability)
  {
    sample s = sample_from_pdf(seed, pdf, len);

    std::iota(out_ranking, out_ranking + len, 0);

    // sort indexes based on comparing values in scores
    std::sort(out_ranking, out_ranking + len,
      [&scores](size_t i1, size_t i2) {return scores[i1] > scores[i2]; });

    // swap top element with chosen one
    std::iter_swap(out_ranking, out_ranking + s.index);

    *out_probability = s.probability;
  }

} // end-of-namespace
