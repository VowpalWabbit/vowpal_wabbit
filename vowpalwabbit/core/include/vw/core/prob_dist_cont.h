// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "vw/core/constant.h"

#include <string>
#include <vector>

namespace VW
{
namespace continuous_actions
{
class probability_density_function_value
{
public:
  float action = 0.f;     // continuous action
  float pdf_value = 0.f;  // pdf value

  probability_density_function_value() = default;
  probability_density_function_value(float action, float pdf_value) : action(action), pdf_value(pdf_value) {}
};

class pdf_segment
{
public:
  float left = 0.f;       // starting point
  float right = 0.f;      // ending point
  float pdf_value = 0.f;  // height

  pdf_segment() = default;
  pdf_segment(float left, float right, float pdf_value) : left(left), right(right), pdf_value(pdf_value) {}
};

using probability_density_function = std::vector<pdf_segment>;

bool is_valid_pdf(probability_density_function& pdf);

}  // namespace continuous_actions
std::string to_string(const continuous_actions::probability_density_function_value& pdf_value,
    int decimal_precision = details::DEFAULT_FLOAT_PRECISION);
std::string to_string(const continuous_actions::probability_density_function& pdf,
    int decimal_precision = details::DEFAULT_FLOAT_PRECISION);
}  // namespace VW
