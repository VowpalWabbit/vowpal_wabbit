// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <cstdint>

#include "label_parser.h"
#include "action_score.h"
#include "io_buf.h"

namespace VW
{
namespace slates
{
enum class example_type : uint8_t
{
  unset = 0,
  shared = 1,
  action = 2,
  slot = 3
};

struct label
{
  // General data
  example_type type;
  float weight;
  // Because these labels provide both structural information as well as a
  // label, this field will only be true is there is a label attached (label in
  // the sense of cost)
  bool labeled;

  // For shared examples
  // Only valid if labeled
  float cost;

  // For action examples
  uint32_t slot_id;

  // For slot examples
  // Only valid if labeled
  ACTION_SCORE::action_scores probabilities;

  label() { reset_to_default(); }

  void reset_to_default()
  {
    type = example_type::unset;
    weight = 1.f;
    labeled = false;
    cost = 0.f;
    slot_id = 0;
    probabilities.clear();
  }
};

void default_label(VW::slates::label& v);
void parse_label(slates::label& ld, VW::label_parser_reuse_mem& reuse_mem, const std::vector<VW::string_view>& words);

extern label_parser slates_label_parser;
}  // namespace slates

VW::string_view to_string(VW::slates::example_type);

namespace model_utils
{
size_t read_model_field(io_buf&, VW::slates::label&);
size_t write_model_field(io_buf&, const VW::slates::label&, const std::string&, bool);
}  // namespace model_utils
}  // namespace VW
