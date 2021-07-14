// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include <cstdint>

#include "rand48.h"

struct rand_state
{
private:
  uint64_t random_state;

public:
  constexpr rand_state() : random_state(0) {}
  rand_state(uint64_t initial) : random_state(initial) {}
  constexpr uint64_t get_current_state() const noexcept { return random_state; }
  float get_and_update_random() { return merand48(random_state); }
  float get_and_update_gaussian() { return merand48_boxmuller(random_state); }
  float get_random() const { return merand48_noadvance(random_state); }
  void set_random_state(uint64_t initial) noexcept { random_state = initial; }
};