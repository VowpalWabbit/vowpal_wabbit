// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "vw/common/random_details.h"

#include <cstdint>

namespace VW
{

class rand_state
{
public:
  rand_state() = default;

  /**
   * @brief Construct a new rand state object with the given seed
   *
   * @param initial initial seed
   */
  rand_state(uint64_t initial) : _random_state(initial) {}

  /**
   * @brief Get the current random state
   *
   * @return uint64_t state value of PRG
   */
  constexpr uint64_t get_current_state() const noexcept { return _random_state; }

  /**
   * @brief Get the next random value in the range [0,1] and update the PRG state
   *
   * @return float random number in range [0,1]
   */
  float get_and_update_random() { return details::merand48(_random_state); }

  /**
   * @brief Generate a random number from the gaussian distribution using the Box-Muller transform
   *
   * @return float number sampled from gaussian distribution
   */
  float get_and_update_gaussian() { return details::merand48_boxmuller(_random_state); }

  /**
   * @brief Get the next random value in the range [0,1], but do not update the PRG state.
   * This means that if this is called successively, the same value will be returned.
   *
   * @return float
   */
  float get_random() const { return details::merand48_noadvance(_random_state); }

  /**
   * @brief Override the current PRG state value
   *
   * @param new_state new state value
   */
  void set_random_state(uint64_t new_state) noexcept { _random_state = new_state; }

private:
  uint64_t _random_state = 0;
};

}  // namespace VW
