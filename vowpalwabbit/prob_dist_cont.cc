// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw.h"
#include "v_array.h"
#include "prob_dist_cont.h"
#include "io_buf.h"
#include "model_utils.h"

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
  if (precision >= 0) ss << std::setprecision(precision);

  for (size_t i = 0; i < pdf.size(); i++)
  {
    if (i > 0) ss << ',';
    ss << pdf[i].left << '-' << pdf[i].right << ':' << pdf[i].pdf_value;
  }

  if (newline) ss << endl;

  return ss.str();
}

bool is_valid_pdf(probability_density_function& pdf)
{
  float mass = 0.f;
  for (const auto& segment : pdf) { mass += (segment.right - segment.left) * segment.pdf_value; }
  if (mass < 0.9999 || mass > 1.0001) { return false; }
  return true;
}

}  // namespace continuous_actions

namespace model_utils
{
size_t read_model_field(io_buf& io, VW::continuous_actions::pdf_segment& pdf)
{
  size_t bytes = 0;
  bytes += read_model_field(io, pdf.left);
  bytes += read_model_field(io, pdf.right);
  bytes += read_model_field(io, pdf.pdf_value);
  return bytes;
}
size_t write_model_field(io_buf& io, const VW::continuous_actions::pdf_segment& pdf, const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  bytes += write_model_field(io, pdf.left, upstream_name + "_left", text);
  bytes += write_model_field(io, pdf.right, upstream_name + "_right", text);
  bytes += write_model_field(io, pdf.pdf_value, upstream_name + "_pdf_value", text);
  return bytes;
}
}  // namespace model_utils
}  // namespace VW
