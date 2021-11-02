// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "label_parser.h"
#include "v_array.h"

struct example;

namespace VW
{
namespace cb_continuous
{
struct continuous_label_elm
{
  float action;     // the continuous action
  float cost;       // the cost of this class
  float pdf_value;  // the pdf density of the chosen location, specifies the probability the data collection policy
                    // chose this action

  bool operator==(const continuous_label_elm&& j) const { return action == j.action; }
};

struct continuous_label
{
  v_array<continuous_label_elm> costs;
};

extern label_parser the_label_parser;

std::string to_string(const continuous_label_elm& elm);
std::string to_string(const continuous_label& lbl);

}  // namespace cb_continuous
}  // namespace VW
