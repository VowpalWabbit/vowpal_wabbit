#include "exploration.h"

#include <stdexcept>
#include <algorithm>
#include <numeric>
#include <cstring>

using namespace std;

namespace exploration
{
  void epsilon_greedy(float epsilon, uint32_t top_action, float* probability_distribution, uint32_t num_actions)
  {
    if (num_actions == 0)
      return;

    if (top_action >= num_actions)
      throw out_of_range("top_action must be smaller than num_actions");

    float prob = epsilon / (float)num_actions;

    // size & initialize vector to prob
    float* prob_end = probability_distribution + num_actions;
    for (float* d = probability_distribution; d != prob_end; ++d)
      *d = prob;

    probability_distribution[top_action] += 1.f - epsilon;
  }

  void softmax(float lambda, const float* scores, float* probability_distribution, uint32_t num_actions)
  {
    if (num_actions == 0)
      return;

    float norm = 0.;
    float max_score = *std::max_element(scores, scores + num_actions);

    float* prob_end = probability_distribution + num_actions;
    const float *s = scores;
    for (float *d = probability_distribution; d != prob_end; ++d, ++s)
    {
      float prob = exp(lambda*(*s - max_score));
      norm += prob;

      *d = prob;
    }

    // normalize
    if (norm > 0)
      for (float* d = probability_distribution; d != prob_end; ++d)
        *d /= norm;
  }

  void bag(const uint32_t* top_actions, float* probability_distribution, uint32_t num_actions)
  {
    if (num_actions == 0)
      return;

    uint32_t num_models = std::accumulate(top_actions, top_actions + num_actions, 0);
    if (num_models == 0)
      throw out_of_range("must supply at least one top_action from a model");

    // determine probability per model
    float prob = 1.f / (float)num_models;

    for (size_t i = 0;i < num_actions;i++)
      probability_distribution[i] = top_actions[i] * prob;
  }

  void enforce_minimum_probability(float min_prob, float* probability_distribution, uint32_t num_actions)
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
        throw invalid_argument("Cannot safety this distribution");

      float ratio = (1.f - touched_mass) / untouched_mass;
      for (float* d = probability_distribution; d != prob_end; ++d)
        if (*d > min_prob)
          *d *= ratio;
    }
  }
}
