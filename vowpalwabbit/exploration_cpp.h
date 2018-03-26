#pragma once

#include <vector>
#include "exploration.h"

namespace exploration
{
  std::vector<float> epsilon_greedy(float epsilon, uint32_t top_action, uint32_t num_actions)
  {
    std::vector<float> probability_distribution(num_actions);
    epsilon_greedy(epsilon, top_action, &probability_distribution[0], num_actions);
    return probability_distribution;
  }

  std::vector<float> softmax(float lambda, const std::vector<float>& scores)
  {
    std::vector<float> probability_distribution(scores.size());
    softmax(lambda, &scores[0], &probability_distribution[0], scores.size());
    return probability_distribution;
  }

  std::vector<float> bag(const std::vector<uint32_t>& top_actions)
  {
    std::vector<float> probability_distribution(top_actions.size());
    bag(&top_actions[0], &probability_distribution[0], top_actions.size());
    return probability_distribution;
  }
}
