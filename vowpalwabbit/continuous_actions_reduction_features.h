// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "v_array.h"
#include "prob_dist_cont.h"

#include <cmath>

namespace VW
{
namespace continuous_actions
{
struct reduction_features
{
  probability_density_function pdf;
  float chosen_action;
  bool is_chosen_action_set() const { return !std::isnan(chosen_action); }
  bool is_pdf_set() const { return pdf.size() > 0; }

  reduction_features()
  {
    pdf = v_init<pdf_segment>();
    chosen_action = std::numeric_limits<float>::quiet_NaN();
  }

  ~reduction_features() { pdf.delete_v(); }

  void clear()
  {
    pdf.clear();
    chosen_action = std::numeric_limits<float>::quiet_NaN();
  }

  void check_valid_pdf_or_clear()
  {
    float mass = 0.f;
    for (const auto& segment : pdf) { mass += (segment.right - segment.left) * segment.pdf_value; }
    if (mass < 0.9999 || mass > 1.0001)
    {
      // not using pdf provided as it does not sum to 1
      pdf.clear();
    }
  }
};

}  // namespace continuous_actions
}  // namespace VW