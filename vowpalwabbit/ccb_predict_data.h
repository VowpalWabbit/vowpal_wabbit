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

 
struct predict_data
{
  example_type type;
  v_array<uint32_t> explicit_included_actions;

  void clear() { explicit_included_actions.clear(); }
};
 
}
