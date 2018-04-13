#pragma once

namespace exploration {
  template<typename It>
  void generate_epsilon_greedy(float epsilon, uint32_t top_action, It pdf_first, It pdf_last);

  template<typename InputIt, typename OutputIt>
  void generate_softmax(float lambda, InputIt scores_begin, InputIt scores_last, OutputIt pdf_first, OutputIt pdf_last);

  template<typename InputIt, typename OutputIt>
  void generate_bag(InputIt top_actions_begin, InputIt top_actions_last, OutputIt pdf_first, OutputIt pdf_last);

  template<typename It>
  void enforce_minimum_probability(float min_prob, bool update_zero_elements, It pdf_first, It pdf_last);

  template<typename InputIt>
  uint32_t sample_from_pdf(uint64_t seed, InputIt pdf_first, InputIt pdf_last);

  template<typename InputIt>
  uint32_t sample_from_pdf(const char* seed, InputIt pdf_first, InputIt pdf_last);

  template<typename InputPdfIt, typename InputScoreIt, typename OutputIt>
  void sample_from_pdf(const char* seed, InputPdfIt pdf_begin, InputPdfIt pdf_end, InputScoreIt scores_begin, InputScoreIt scores_end, OutputIt ranking_begin, OutputIt ranking_end);

  template<typename InputPdfIt, typename InputScoreIt, typename OutputIt>
  void sample_from_pdf(uint64_t seed, InputPdfIt pdf_begin, InputPdfIt pdf_end, InputScoreIt scores_begin, InputScoreIt scores_end, OutputIt ranking_begin, OutputIt ranking_end);
}