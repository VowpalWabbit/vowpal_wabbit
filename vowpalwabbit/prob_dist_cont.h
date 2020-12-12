// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include <string>
#include "v_array.h"

namespace VW
{
namespace continuous_actions
{
struct probability_density_function_value
{
  float action;     // continuous action
  float pdf_value;  // pdf value
};

struct pdf_segment
{
  float left;       // starting point
  float right;      // ending point
  float pdf_value;  // height
};

using probability_density_function = v_array<pdf_segment>;

std::string to_string(const probability_density_function_value& pdf_value, bool print_newline = false);
std::string to_string(const probability_density_function& pdf, bool print_newline = false, int precision = -1);
void delete_probability_density_function(void* v);

}  // namespace continuous_actions
}  // namespace VW
