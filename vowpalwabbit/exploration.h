#pragma once

#include <stdint.h>

namespace exploration
{
      // distribution must be caller allocated and of size 'num_actions'
      void epsilon_greedy(float epsilon, uint32_t top_action, float* probability_distribution, uint32_t num_actions);

      void softmax(float lambda, const float* scores, float* probability_distribution, uint32_t num_actions);

      void bag(const uint32_t* top_actions, float* probability_distribution, uint32_t num_actions);

      void enforce_minimum_probability(float min_prob, float* probability_distribution, uint32_t num_actions);

} // end-of-namespace
