// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw.h"
#include "v_array.h"
#include "prob_dist_cont.h"
#include "io_buf.h"

using namespace std;
namespace VW
{
namespace continuous_actions
{
std::string to_string(const probability_density_function_value& pdf_value, bool newline)
{
  std::stringstream strm;
  strm << pdf_value.action << "," << pdf_value.pdf_value;
  if (newline) strm << endl;
  return strm.str();
}

std::string to_string(const pdf_segment& seg)
{
  std::stringstream strm;
  strm << "{" << seg.left << "-" << seg.right << "," << seg.pdf_value << "}";
  return strm.str();
}

// Convert pdf to string of form 'begin-end:pdf_value, ... '
std::string to_string(const probability_density_function& pdf, bool newline, int precision)
{
  std::stringstream ss;
  if (precision >= 0)
    ss << std::setprecision(precision);

  for (size_t i = 0; i < pdf.size(); i++)
  {
    if (i > 0) ss << ',';
    ss << pdf[i].left << '-' << pdf[i].right << ':' << pdf[i].pdf_value;
  }

  if (newline) ss << endl;

  return ss.str();
}

void delete_probability_density_function(void* v)
{
  v_array<pdf_segment>* pdf = (v_array<pdf_segment>*)v;
  pdf->delete_v();
}

}  // namespace continuous_actions
}  // namespace VW
