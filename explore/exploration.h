#pragma once

#include <stdint.h>
#include <stdexcept>
#include <algorithm>
#include <numeric>
#include <cstring>

namespace exploration
{
  template<typename It>
  void generate_epsilon_greedy(float epsilon, uint32_t top_action, It pdf_first, It pdf_last, std::random_access_iterator_tag pdf_tag)
  {
    if (pdf_last < pdf_first)
      return;

    size_t num_actions = pdf_last - pdf_first;
    if (num_actions == 0)
      return;

	if (top_action >= num_actions)
  	  top_action = (uint32_t)num_actions - 1;

    float prob = epsilon / (float)num_actions;

    for (It d = pdf_first; d != pdf_last; ++d)
      *d = prob;

    *(pdf_first + top_action) += 1.f - epsilon;
  }

  template<typename It>
  void generate_epsilon_greedy(float epsilon, uint32_t top_action, It pdf_first, It pdf_last)
  {
    typedef typename std::iterator_traits<It>::iterator_category pdf_category;
    generate_epsilon_greedy(epsilon, top_action, pdf_first, pdf_last, pdf_category());
  }

  template<typename InputIt, typename OutputIt>
  void generate_softmax(float lambda, InputIt scores_begin, InputIt scores_last, std::input_iterator_tag scores_tag, OutputIt pdf_first, OutputIt pdf_last, std::random_access_iterator_tag pdf_tag)
  {
    if (scores_last < scores_begin || pdf_last < pdf_first)
      return;

    size_t num_actions_scores = scores_last - scores_begin;
    size_t num_actions_pdf = pdf_last - pdf_first;

    if (num_actions_scores != num_actions_pdf)
    {
      // fallback to the minimum
      scores_last = scores_begin + min(num_actions_scores, num_actions_pdf);
      pdf_last = pdf_first + min(num_actions_scores, num_actions_pdf);
    }

    if (num_actions_scores == 0)
      return;

    float norm = 0.;
    float max_score = *std::max_element(scores_begin, scores_last);

    InputIt s = scores_begin;
    for (OutputIt d = pdf_first; d != pdf_last && s != scores_last; ++d, ++s)
    {
      float prob = exp(lambda*(*s - max_score));
      norm += prob;

      *d = prob;
    }

    // normalize
    for (OutputIt d = pdf_first; d != pdf_last; ++d)
      *d /= norm;
  }

  template<typename InputIt, typename OutputIt>
  void generate_softmax(float lambda, InputIt scores_begin, InputIt scores_last, OutputIt pdf_first, OutputIt pdf_last)
  {
    typedef typename std::iterator_traits<InputIt>::iterator_category scores_category;
    typedef typename std::iterator_traits<OutputIt>::iterator_category pdf_category;

    generate_softmax(lambda, scores_begin, scores_last, scores_category(), pdf_first, pdf_last, pdf_category());
  }

  template<typename InputIt, typename OutputIt>
  void generate_bag(InputIt top_actions_begin, InputIt top_actions_last, std::input_iterator_tag top_actions_tag, OutputIt pdf_first, OutputIt pdf_last, std::random_access_iterator_tag pdf_tag)
  {
    if (pdf_first == pdf_last || pdf_last < pdf_first)
      return;

    uint32_t num_models = std::accumulate(top_actions_begin, top_actions_last, 0);
    if (num_models == 0)
    {
      *pdf_first = 1;
      return;
    }

    // divide late to improve numeric stability
    InputIt t_a = top_actions_begin;
    for (OutputIt d = pdf_first; d != pdf_last && t_a != top_actions_last; ++d, ++t_a)
      *d = *t_a / (float)num_models;
  }

  template<typename InputIt, typename OutputIt>
  void generate_bag(InputIt top_actions_begin, InputIt top_actions_last, OutputIt pdf_first, OutputIt pdf_last)
  {
    typedef typename std::iterator_traits<InputIt>::iterator_category top_actions_category;
    typedef typename std::iterator_traits<OutputIt>::iterator_category pdf_category;

    generate_bag(top_actions_begin, top_actions_last, top_actions_category(), pdf_first, pdf_last, pdf_category());
  }

  template<typename It>
  void enforce_minimum_probability(float min_prob, bool update_zero_elements, It pdf_first, It pdf_last, std::random_access_iterator_tag pdf_tag)
  {
	  size_t num_actions = pdf_last - pdf_first;

    if (min_prob > 0.999) // uniform exploration
    {
      size_t support_size = num_actions;
      if (!update_zero_elements)
      {
        for (It d = pdf_first; d != pdf_last; ++d)
          if (*d == 0)
            support_size--;
      }

	  for (It d = pdf_first; d != pdf_last; ++d)
        if (update_zero_elements || *d > 0)
          *d = 1.f / support_size;

      return;
    }

    min_prob /= num_actions;
    float touched_mass = 0.;
    float untouched_mass = 0.;
    uint16_t num_actions_touched = 0;

	  for (It d = pdf_first; d != pdf_last; ++d)
    {
      auto& prob = *d;
      if ((prob > 0 || (prob == 0 && update_zero_elements)) && prob <= min_prob)
      {
        touched_mass += min_prob;
        prob = min_prob;
        ++num_actions_touched;
      }
      else
        untouched_mass += prob;
    }

    if (touched_mass > 0.)
    {
      if (touched_mass > 0.999)
      {
        min_prob = (1.f - untouched_mass) / (float)num_actions_touched;
        for (It d = pdf_first; d != pdf_last; ++d)
        {
          auto& prob = *d;
          if ((prob > 0 || (prob == 0 && update_zero_elements)) && prob <= min_prob)
            prob = min_prob;
        }
      }
      else
      {
        float ratio = (1.f - touched_mass) / untouched_mass;
        for (It d = pdf_first; d != pdf_last; ++d)
          if (*d > min_prob)
            *d *= ratio;
      }
    }
  }

  template<typename It>
  void enforce_minimum_probability(float min_prob, bool update_zero_elements, It pdf_first, It pdf_last)
  {
	  typedef typename std::iterator_traits<It>::iterator_category pdf_category;

	  enforce_minimum_probability(min_prob, update_zero_elements, pdf_first, pdf_last, pdf_category());
  }
} // end-of-namespace
