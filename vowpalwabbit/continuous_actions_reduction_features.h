// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "v_array.h"
#include "prob_dist_cont.h"
#include "io_buf.h"

#include <cmath>
#include <limits>

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
    chosen_action = std::numeric_limits<float>::quiet_NaN();
  }

  void clear()
  {
    pdf.clear();
    chosen_action = std::numeric_limits<float>::quiet_NaN();
  }
};

}  // namespace continuous_actions

namespace model_utils
{
size_t read_model_field(io_buf&, VW::continuous_actions::reduction_features&);
size_t write_model_field(io_buf&, const VW::continuous_actions::reduction_features&, const std::string&, bool);
}  // namespace model_utils
}  // namespace VW