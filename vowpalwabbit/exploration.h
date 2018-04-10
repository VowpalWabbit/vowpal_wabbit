#pragma once

#include <stdint.h>
#include <stdexcept>
#include <algorithm>
#include <numeric>
#include <cstring>

namespace exploration
{
  template<typename OutputIt>
  void generate_epsilon_greedy(float epsilon, uint32_t top_action, OutputIt pdf_first, OutputIt pdf_last, std::output_iterator_tag pdf_tag)
  {
    size_t num_actions = pdf_last - pdf_first;
    if (num_actions == 0)
      return;

    if (top_action >= num_actions)
      throw std::out_of_range("top_action must be smaller than num_actions");

    float prob = epsilon / (float)num_actions;

    // size & initialize vector to prob
    for (OutputIt d = pdf_first; d != pdf_last; ++d)
      *d = prob;

    *(pdf_first + top_action) += 1.f - epsilon;
  }

  template<typename OutputIt>
  void generate_epsilon_greedy(float epsilon, uint32_t top_action, OutputIt pdf_first, OutputIt pdf_last)
  {
    typedef typename std::iterator_traits<OutputIt>::iterator_category pdf_category;
    generate_epsilon_greedy(epsilon, top_action, pdf_first, pdf_last, pdf_category());
  }

  template<typename InputIt, typename OutputIt>
  void generate_softmax(float lambda, InputIt scores_begin, InputIt scores_last, std::input_iterator_tag scores_tag, OutputIt pdf_first, OutputIt pdf_last, std::output_iterator_tag pdf_tag)
  {
    size_t num_actions_scores = scores_last - scores_begin;
    size_t num_actions_pdf = pdf_last - pdf_first;

    if (num_actions_scores != num_actions_pdf)
      throw std::invalid_argument("length of scores must be equal to length of pdf");

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
  void generate_bag(InputIt top_actions_begin, InputIt top_actions_last, std::input_iterator_tag top_actions_tag, OutputIt pdf_first, OutputIt pdf_last, std::output_iterator_tag pdf_tag)
  {
    if (pdf_first == pdf_last)
      return;

    uint32_t num_models = std::accumulate(top_actions_begin, top_actions_last, 0);
    if (num_models == 0)
      throw std::out_of_range("must supply at least one top_action from a model");

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

  // Note: must be inline to compile on Windows VS2017
  inline void enforce_minimum_probability(float min_prob, float* probability_distribution, uint32_t num_actions)
  {
    float* prob_end = probability_distribution + num_actions;
    bool zeros = false;

    //input: a probability distribution
    //output: a probability distribution with all events having probability > min_prob.  This includes events with probability 0 if zeros = true
    if (min_prob > 0.999) // uniform exploration
    {
      size_t support_size = num_actions;
      if (!zeros)
      {
        for (float* d = probability_distribution; d != prob_end; ++d)
          if (*d == 0)
            support_size--;
      }

      for (float* d = probability_distribution; d != prob_end; ++d)
        if (zeros || *d > 0)
          *d = 1.f / support_size;

      return;
    }

    min_prob /= num_actions;
    float touched_mass = 0.;
    float untouched_mass = 0.;

    for (float* d = probability_distribution; d != prob_end; ++d)
    {
      float& prob = *d;
      if ((prob > 0 || (prob == 0 && zeros)) && prob <= min_prob)
      {
        touched_mass += min_prob;
        prob = min_prob;
      }
      else
        untouched_mass += prob;
    }

    if (touched_mass > 0.)
    {
      if (touched_mass > 0.999)
        throw std::invalid_argument("Cannot safety this distribution");

      float ratio = (1.f - touched_mass) / untouched_mass;
      for (float* d = probability_distribution; d != prob_end; ++d)
        if (*d > min_prob)
          *d *= ratio;
    }
  }
} // end-of-namespace
