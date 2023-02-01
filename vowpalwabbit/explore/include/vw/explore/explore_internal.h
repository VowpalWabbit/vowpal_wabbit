// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/common/hash.h"
#include "vw/common/random.h"
#include "vw/common/random_details.h"
#include "vw/explore/explore_error_codes.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <numeric>
#include <stdexcept>
#include <vector>

namespace VW
{
namespace explore
{
namespace details
{
template <typename It>
int generate_epsilon_greedy(
    float epsilon, uint32_t top_action, It pmf_first, It pmf_last, std::random_access_iterator_tag /* pmf_tag */)
{
  if (pmf_last < pmf_first) { return E_EXPLORATION_BAD_RANGE; }

  size_t num_actions = pmf_last - pmf_first;
  if (num_actions == 0) { return E_EXPLORATION_BAD_RANGE; }

  if (top_action >= num_actions) { top_action = static_cast<uint32_t>(num_actions) - 1; }

  float prob = epsilon / static_cast<float>(num_actions);

  for (It d = pmf_first; d != pmf_last; ++d) { *d = prob; }

  *(pmf_first + top_action) += 1.f - epsilon;

  return S_EXPLORATION_OK;
}

template <typename InputIt, typename OutputIt>
int generate_softmax(float lambda, InputIt scores_first, InputIt scores_last, std::input_iterator_tag /* scores_tag */,
    OutputIt pmf_first, OutputIt pmf_last, std::random_access_iterator_tag /* pmf_tag */)
{
  if (scores_last < scores_first || pmf_last < pmf_first) { return E_EXPLORATION_BAD_RANGE; }

  size_t num_actions_scores = scores_last - scores_first;
  size_t num_actions_pmf = pmf_last - pmf_first;

  if (num_actions_scores != num_actions_pmf)
  {
    // fallback to the minimum
    scores_last = scores_first + std::min(num_actions_scores, num_actions_pmf);
    OutputIt pmf_new_last = pmf_first + std::min(num_actions_scores, num_actions_pmf);

    // zero out pmf
    for (OutputIt d = pmf_new_last; d != pmf_last; ++d) { *d = 0; }

    pmf_last = pmf_new_last;
  }

  if (pmf_last - pmf_first == 0) { return E_EXPLORATION_BAD_RANGE; }

  float norm = 0.;
  float max_score =
      lambda > 0 ? *std::max_element(scores_first, scores_last) : *std::min_element(scores_first, scores_last);

  InputIt s = scores_first;
  for (OutputIt d = pmf_first; d != pmf_last && s != scores_last; ++d, ++s)
  {
    float prob = std::exp(lambda * (*s - max_score));
    norm += prob;

    *d = prob;
  }

  // normalize
  for (OutputIt d = pmf_first; d != pmf_last; ++d) { *d /= norm; }

  return S_EXPLORATION_OK;
}

template <typename InputIt, typename OutputIt>
int generate_bag(InputIt top_actions_first, InputIt top_actions_last, std::input_iterator_tag /* top_actions_tag */,
    OutputIt pmf_first, OutputIt pmf_last, std::random_access_iterator_tag /* pmf_tag */)
{
  // iterators don't support <= in general
  if (pmf_first == pmf_last || pmf_last < pmf_first) { return E_EXPLORATION_BAD_RANGE; }

  float num_models = static_cast<float>(std::accumulate(top_actions_first, top_actions_last, 0.));
  if (num_models <= 1e-6)
  {
    // based on above checks we have at least 1 element in pmf
    *pmf_first = 1;
    for (OutputIt d = pmf_first + 1; d != pmf_last; ++d) { *d = 0; }

    return S_EXPLORATION_OK;
  }

  // divide late to improve numeric stability
  InputIt t_a = top_actions_first;
  float normalizer = 1.f / num_models;
  for (OutputIt d = pmf_first; d != pmf_last && t_a != top_actions_last; ++d, ++t_a) { *d = *t_a * normalizer; }

  return S_EXPLORATION_OK;
}

template <typename It>
int enforce_minimum_probability(float uniform_epsilon, bool consider_zero_valued_elements, It pmf_first, It pmf_last,
    std::random_access_iterator_tag /* pmf_tag */)
{
  if (pmf_first == pmf_last || pmf_last < pmf_first) { return E_EXPLORATION_BAD_RANGE; }

  // Nothing to do
  if (uniform_epsilon == 0.f) { return S_EXPLORATION_OK; }

  if (uniform_epsilon < 0.f || uniform_epsilon > 1.f) { return E_EXPLORATION_BAD_EPSILON; }

  const auto num_actions = std::distance(pmf_first, pmf_last);

  size_t support_size = num_actions;
  if (!consider_zero_valued_elements) { support_size -= std::count(pmf_first, pmf_last, 0.f); }

  if (uniform_epsilon > 0.999f)  // uniform exploration
  {
    std::for_each(pmf_first, pmf_last,
        [support_size, consider_zero_valued_elements](float& prob)
        {
          if (consider_zero_valued_elements || prob > 0) { prob = 1.f / support_size; }
        });

    return S_EXPLORATION_OK;
  }

  // When consider_zero_valued_elements == false, uniform_epsilon is divided by the total
  // number of actions but only nonzero elements are updated.
  const auto minimum_probability = uniform_epsilon / support_size;

  std::vector<float> sorted_probs(pmf_first, pmf_last);
  std::sort(sorted_probs.begin(), sorted_probs.end(), std::greater<float>());

  size_t idx = 0;
  float running_sum = 0.f;
  size_t rho_idx = 0;  // rho = 0 should always trigger if statement below if uniform_epsilon < 1.
  float rho_sum = sorted_probs[0];

  for (const auto prob : sorted_probs)
  {
    if (!consider_zero_valued_elements && prob == 0.f) { break; }

    running_sum += prob;
    if (prob > ((support_size - idx - 1) * minimum_probability + running_sum - 1.f) / (idx + 1.f) + minimum_probability)
    {
      rho_idx = idx;
      rho_sum = running_sum;
    }

    idx += 1;
  }

  const auto tau = ((support_size - rho_idx - 1.f) * minimum_probability + rho_sum - 1.f) / (rho_idx + 1.f);

  std::for_each(pmf_first, pmf_last,
      [tau, minimum_probability, consider_zero_valued_elements](float& prob)
      {
        if (consider_zero_valued_elements || prob > 0) { prob = std::max(prob - tau, minimum_probability); }
      });
  return S_EXPLORATION_OK;
}

template <typename It>
int mix_with_uniform(float uniform_epsilon, It pmf_first, It pmf_last, std::random_access_iterator_tag /* pmf_tag */)
{
  if (pmf_first == pmf_last || pmf_last < pmf_first) { return E_EXPLORATION_BAD_RANGE; }

  size_t num_actions = std::distance(pmf_first, pmf_last);
  const auto scale = (1.f - uniform_epsilon);
  for (It d = pmf_first; d != pmf_last; ++d)
  {
    auto& prob = *d;
    prob *= scale;
    prob += uniform_epsilon / num_actions;
  }

  return S_EXPLORATION_OK;
}

// Warning: `seed` must be sufficiently random for the PRNG to produce uniform random values. Using sequential seeds
// will result in a very biased distribution. If unsure how to update seed between calls, merand48 (in random_details.h)
// can be used to inplace mutate it.
template <typename It>
int sample_after_normalizing(
    uint64_t seed, It pmf_first, It pmf_last, uint32_t& chosen_index, std::input_iterator_tag /* pmf_category */)
{
  if (pmf_first == pmf_last || pmf_last < pmf_first) { return E_EXPLORATION_BAD_RANGE; }
  // Create a discrete_distribution based on the returned weights. This class handles the
  // case where the sum of the weights is < or > 1, by normalizing agains the sum.
  float total = 0.f;
  for (It pmf = pmf_first; pmf != pmf_last; ++pmf)
  {
    if (*pmf < 0) { *pmf = 0; }

    total += *pmf;
  }

  // assume the first is the best
  if (total == 0)
  {
    chosen_index = 0;
    *pmf_first = 1;
    return S_EXPLORATION_OK;
  }

  float draw = total * VW::details::merand48_noadvance(seed);
  if (draw > total)
  {  // make very sure that draw can not be greater than total.
    draw = total;
  }

  bool index_found = false;  // found chosen action
  float sum = 0.f;
  uint32_t i = 0;
  for (It pmf = pmf_first; pmf != pmf_last; ++pmf, ++i)
  {
    sum += *pmf;
    if (!index_found && sum > draw)
    {
      chosen_index = i;
      index_found = true;
    }
    *pmf /= total;
  }

  if (!index_found) { chosen_index = i - 1; }

  return S_EXPLORATION_OK;
}

// Warning: `seed` must be sufficiently random for the PRNG to produce uniform random values. Using sequential seeds
// will result in a very biased distribution.
// If unsure how to update seed between calls, merand48 (in random_details.h) can be used to inplace mutate it.
template <typename It>
int sample_after_normalizing(
    const char* seed, It pmf_first, It pmf_last, uint32_t& chosen_index, std::random_access_iterator_tag pmf_category)
{
  uint64_t seed_hash = VW::uniform_hash(seed, strlen(seed), 0);
  return sample_after_normalizing(seed_hash, pmf_first, pmf_last, chosen_index, pmf_category);
}

template <typename ActionIt>
int swap_chosen(
    ActionIt action_first, ActionIt action_last, std::forward_iterator_tag /* action_category */, uint32_t chosen_index)
{
  if (action_last < action_first) { return E_EXPLORATION_BAD_RANGE; }

  size_t action_size = action_last - action_first;

  if (action_size == 0) { return E_EXPLORATION_BAD_RANGE; }

  if (chosen_index >= action_size) { return E_EXPLORATION_BAD_RANGE; }

  // swap top element with chosen one
  if (chosen_index != 0) { std::iter_swap(action_first, action_first + chosen_index); }

  return S_EXPLORATION_OK;
}

// Pick a discrete action in proportion to the scores.
// Notes:
// 1) Random seed is advanced
// 2) Does not normalize the scores (unlike sample_after_normalization)
// 3) Scores need not add up to one.
template <typename It>
int sample_scores(
    uint64_t* p_seed, It scores_first, It scores_last, uint32_t& chosen_index, std::random_access_iterator_tag)
{
  if (scores_first == scores_last || scores_last < scores_first) { return E_EXPLORATION_BAD_RANGE; }
  // Create a discrete_distribution based on the returned weights. This class handles the
  // case where the sum of the weights is < or > 1, by normalizing agains the sum.
  float total = 0.f;
  for (It scores = scores_first; scores != scores_last; ++scores)
  {
    if (*scores < 0) { *scores = 0; }

    total += *scores;
  }

  // assume the first is the best
  if (total == 0)
  {
    chosen_index = 0;
    *scores_first = 1;
    return S_EXPLORATION_OK;
  }

  float draw = total * VW::details::merand48(*p_seed);
  if (draw > total)
  {  // make very sure that draw can not be greater than total.
    draw = total;
  }

  float sum = 0.f;
  uint32_t i = 0;
  for (It scores = scores_first; scores != scores_last; ++scores, ++i)
  {
    sum += *scores;
    if (sum > draw)
    {
      chosen_index = i;
      return S_EXPLORATION_OK;
    }
  }

  chosen_index = i - 1;
  return S_EXPLORATION_OK;
}

template <typename It>
int sample_pdf(
    uint64_t* p_seed, It pdf_first, It pdf_last, float& chosen_value, float& pdf_value, std::random_access_iterator_tag)
{
  if (std::distance(pdf_first, pdf_last) == 0) { return E_EXPLORATION_BAD_PDF; }

  float total_pdf_mass = 0.f;
  for (It pdf_it = pdf_first; pdf_it != pdf_last; ++pdf_it)
  {
    total_pdf_mass += (pdf_it->right - pdf_it->left) * pdf_it->pdf_value;
  }
  if (total_pdf_mass == 0.f) { return E_EXPLORATION_BAD_PDF; }

  constexpr float edge_avoid_factor = 1.0001f;
  float draw = 0.f;
  do {
    draw = edge_avoid_factor * total_pdf_mass * VW::details::merand48(*p_seed);
  } while (draw >= total_pdf_mass);

  float acc_mass = 0.f;

  chosen_value = pdf_first->left;
  pdf_value = pdf_first->pdf_value;

  for (It pdf_it = pdf_first; pdf_it != pdf_last; ++pdf_it)
  {
    float seg_mass = pdf_it->pdf_value * (pdf_it->right - pdf_it->left);
    if (draw <= seg_mass + acc_mass)
    {
      float mass_in_region = draw - acc_mass;
      chosen_value = pdf_it->left + mass_in_region / pdf_it->pdf_value;
      pdf_value = pdf_it->pdf_value;
      return S_EXPLORATION_OK;
    }
    acc_mass += seg_mass;
  }
  return S_EXPLORATION_OK;
}

}  // namespace details

template <typename It>
int generate_epsilon_greedy(float epsilon, uint32_t top_action, It pmf_first, It pmf_last)
{
  using pmf_category = typename std::iterator_traits<It>::iterator_category;
  return details::generate_epsilon_greedy(epsilon, top_action, pmf_first, pmf_last, pmf_category());
}

template <typename InputIt, typename OutputIt>
int generate_softmax(float lambda, InputIt scores_first, InputIt scores_last, OutputIt pmf_first, OutputIt pmf_last)
{
  using scores_category = typename std::iterator_traits<InputIt>::iterator_category;
  using pmf_category = typename std::iterator_traits<OutputIt>::iterator_category;

  return details::generate_softmax(
      lambda, scores_first, scores_last, scores_category(), pmf_first, pmf_last, pmf_category());
}

template <typename InputIt, typename OutputIt>
int generate_bag(InputIt top_actions_first, InputIt top_actions_last, OutputIt pmf_first, OutputIt pmf_last)
{
  using top_actions_category = typename std::iterator_traits<InputIt>::iterator_category;
  using pmf_category = typename std::iterator_traits<OutputIt>::iterator_category;

  return details::generate_bag(
      top_actions_first, top_actions_last, top_actions_category(), pmf_first, pmf_last, pmf_category());
}

template <typename It>
int enforce_minimum_probability(float uniform_epsilon, bool consider_zero_valued_elements, It pmf_first, It pmf_last)
{
  using pmf_category = typename std::iterator_traits<It>::iterator_category;

  return details::enforce_minimum_probability(
      uniform_epsilon, consider_zero_valued_elements, pmf_first, pmf_last, pmf_category());
}
template <typename It>
int mix_with_uniform(float uniform_epsilon, It pmf_first, It pmf_last)
{
  using pmf_category = typename std::iterator_traits<It>::iterator_category;

  return details::mix_with_uniform(uniform_epsilon, pmf_first, pmf_last, pmf_category());
}

// Warning: `seed` must be sufficiently random for the PRNG to produce uniform random values. Using sequential seeds
// will result in a very biased distribution. If unsure how to update seed between calls, merand48 (in random_details.h)
// can be used to inplace mutate it.
template <typename It>
int sample_after_normalizing(uint64_t seed, It pmf_first, It pmf_last, uint32_t& chosen_index)
{
  using pmf_category = typename std::iterator_traits<It>::iterator_category;
  return details::sample_after_normalizing(seed, pmf_first, pmf_last, chosen_index, pmf_category());
}

// Warning: `seed` must be sufficiently random for the PRNG to produce uniform random values. Using sequential seeds
// will result in a very biased distribution. If unsure how to update seed between calls, merand48 (in random_details.h)
// can be used to inplace mutate it.
template <typename It>
int sample_after_normalizing(const char* seed, It pmf_first, It pmf_last, uint32_t& chosen_index)
{
  using pmf_category = typename std::iterator_traits<It>::iterator_category;
  return details::sample_after_normalizing(seed, pmf_first, pmf_last, chosen_index, pmf_category());
}

template <typename ActionsIt>
int swap_chosen(ActionsIt action_first, ActionsIt action_last, uint32_t chosen_index)
{
  using actionit_category = typename std::iterator_traits<ActionsIt>::iterator_category;
  return details::swap_chosen(action_first, action_last, actionit_category(), chosen_index);
}

template <typename It>
int sample_pdf(uint64_t* p_seed, It pdf_first, It pdf_last, float& chosen_value, float& pdf_value)
{
  using pdf_category = typename std::iterator_traits<It>::iterator_category;
  return details::sample_pdf(p_seed, pdf_first, pdf_last, chosen_value, pdf_value, pdf_category());
}
}  // namespace explore
}  // namespace VW