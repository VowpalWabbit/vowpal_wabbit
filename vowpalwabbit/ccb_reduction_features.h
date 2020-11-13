#pragma once

#include "v_array.h"

#include <cstdint>

namespace CCB {
enum example_type : uint8_t
{
  unset = 0,
  shared = 1,
  action = 2,
  slot = 3
};

 
struct reduction_features
{
  example_type type;
  v_array<uint32_t> explicit_included_actions;

  reduction_features()
  {
    explicit_included_actions = v_init<uint32_t>();
  }
  ~reduction_features()
  {
    explicit_included_actions.delete_v();
  }
  void clear() { explicit_included_actions.clear(); }
};
 
}
