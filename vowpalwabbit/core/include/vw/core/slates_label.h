// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/common/future_compat.h"
#include "vw/core/action_score.h"
#include "vw/core/label_parser.h"
#include "vw/core/vw_fwd.h"

#include <cstdint>

namespace VW
{
enum class SlatesExampleType : uint8_t
{
  UNSET = 0,
  SHARED = 1,
  ACTION = 2,
  SLOT = 3
};

struct SlatesLabel
{
  // General data
  SlatesExampleType type;
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

  SlatesLabel() { reset_to_default(); }

  void reset_to_default()
  {
    type = SlatesExampleType::UNSET;
    weight = 1.f;
    labeled = false;
    cost = 0.f;
    slot_id = 0;
    probabilities.clear();
  }
};

void slates_default_label(VW::SlatesLabel& v);
void slates_parse_label(SlatesLabel& ld, VW::label_parser_reuse_mem& reuse_mem,
    const std::vector<VW::string_view>& words, VW::io::logger& logger);

extern VW::label_parser slates_label_parser;

VW::string_view to_string(VW::SlatesExampleType);

namespace slates
{
using label VW_DEPRECATED("VW::slates::label renamed to VW::SlatesLabel") = VW::SlatesLabel;
}

namespace model_utils
{
size_t read_model_field(io_buf&, VW::SlatesLabel&);
size_t write_model_field(io_buf&, const VW::SlatesLabel&, const std::string&, bool);
}  // namespace model_utils
}  // namespace VW

namespace fmt
{
template <>
struct formatter<VW::SlatesExampleType> : formatter<std::string>
{
  auto format(VW::SlatesExampleType c, format_context& ctx) -> decltype(ctx.out())
  {
    return formatter<std::string>::format(std::string{VW::to_string(c)}, ctx);
  }
};
}  // namespace fmt