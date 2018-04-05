#pragma once

#include <cstdint>
#include <vector>

namespace exploration
{
  struct sample
  {
    // if the pdf is not normalized, this is the normalized probability
    float probability;

    uint32_t index;
  };

  sample sample_from_pdf(const char* seed, const float* pdf, uint32_t len);

  sample sample_from_pdf(uint64_t seed, const float* pdf, uint32_t len);

  // if the pdf is not normalized, out_probability is the normalized probability
  void sample_from_pdf(const char* seed, const float* pdf, const float* scores, uint32_t len, uint32_t* out_ranking, float* out_probability);

  // if the pdf is not normalized, out_probability is the normalized probability
  void sample_from_pdf(uint64_t seed, const float* pdf, const float* scores, uint32_t len, uint32_t* out_ranking, float* out_probability);

} // end-of-namespace
