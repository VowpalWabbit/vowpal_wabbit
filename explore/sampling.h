#pragma once

#include "sampling_internal.h"

namespace exploration
{
  // sample from pdf using seed
  template<typename InputIt>
  uint32_t sample_from_pdf(uint64_t seed, InputIt pdf_first, InputIt pdf_last)
  {
    typedef typename std::iterator_traits<InputIt>::iterator_category pdf_category;
    return sample_from_pdf(seed, pdf_first, pdf_last, pdf_category());
  }

  // sample from pdf using string seed
  template<typename InputIt>
  uint32_t sample_from_pdf(const char* seed, InputIt pdf_first, InputIt pdf_last)
  {
    typedef typename std::iterator_traits<InputIt>::iterator_category pdf_category;
    return sample_from_pdf(seed, pdf_first, pdf_last, pdf_category());
  }

  // sample ranking using top-slot swap from pdf using string seed
  template<typename InputPdfIt, typename InputScoreIt, typename OutputIt>
  int sample_from_pdf(const char* seed, InputPdfIt pdf_begin, InputPdfIt pdf_end, InputScoreIt scores_begin, InputScoreIt scores_end, OutputIt ranking_begin, OutputIt ranking_end)
  {
    uint64_t seed_hash = uniform_hash(seed, strlen(seed), 0);
    return sample_from_pdf(seed_hash, pdf_begin, pdf_end, scores_begin, scores_end, ranking_begin, ranking_end);
  }

// sample ranking using top-slot swap from pdf using seed
  template<typename InputPdfIt, typename InputScoreIt, typename OutputIt>
  int sample_from_pdf(uint64_t seed, InputPdfIt pdf_begin, InputPdfIt pdf_end, InputScoreIt scores_begin, InputScoreIt scores_end, OutputIt ranking_begin, OutputIt ranking_end)
  {
    typedef typename std::iterator_traits<InputPdfIt>::iterator_category pdf_category;
    typedef typename std::iterator_traits<InputScoreIt>::iterator_category scores_category;
    typedef typename std::iterator_traits<OutputIt>::iterator_category ranking_category;

    return sample_from_pdf(seed, pdf_begin, pdf_end, pdf_category(), scores_begin, scores_end, scores_category(), ranking_begin, ranking_end, ranking_category());
  }
} // end-of-namespace
