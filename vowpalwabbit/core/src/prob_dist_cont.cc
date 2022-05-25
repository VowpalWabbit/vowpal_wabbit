// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/prob_dist_cont.h"

#include "vw/core/io_buf.h"
#include "vw/core/text_utils.h"
#include "vw/core/v_array.h"
#include "vw/core/vw.h"

using namespace std;
namespace VW
{
// Convert pdf to string of form 'begin-end:pdf_value, ... '
std::string to_string(const VW::continuous_actions::probability_density_function& pdf, int decimal_precision)
{
  std::ostringstream ss;
  for (size_t i = 0; i < pdf.size(); i++)
  {
    if (i > 0) { ss << ','; }
    ss << fmt::format("{}-{}:{}", VW::fmt_float(pdf[i].left, decimal_precision),
        VW::fmt_float(pdf[i].right, decimal_precision), VW::fmt_float(pdf[i].pdf_value, decimal_precision));
  }

  return ss.str();
}

std::string to_string(
    const VW::continuous_actions::probability_density_function_value& pdf_value, int decimal_precision)
{
  return fmt::format("{},{}", VW::fmt_float(pdf_value.action, decimal_precision),
      VW::fmt_float(pdf_value.pdf_value, decimal_precision));
}

namespace continuous_actions
{
bool is_valid_pdf(probability_density_function& pdf)
{
  float mass = 0.f;
  for (const auto& segment : pdf) { mass += (segment.right - segment.left) * segment.pdf_value; }
  if (mass < 0.9999 || mass > 1.0001) { return false; }
  return true;
}

}  // namespace continuous_actions
}  // namespace VW
