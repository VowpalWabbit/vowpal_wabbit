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

  struct ranked_sample
  {
    // if the pdf is not normalized, this is the normalized probability
    float probability;

    std::vector<uint32_t> ranking;
  };

  sample sample_from_pdf(const char* seed, const float* pdf, uint32_t len);

  sample sample_from_pdf(uint64_t seed, const float* pdf, uint32_t len);

  void sample_from_pdf(const char* seed, const float* pdf, const float* scores, uint32_t len, ranked_sample& result);

  void sample_from_pdf(uint64_t seed, const float* pdf, const float* scores, uint32_t len, ranked_sample& result);

} // end-of-namespace
