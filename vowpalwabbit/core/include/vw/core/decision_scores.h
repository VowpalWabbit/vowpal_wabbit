// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/core/constant.h"
#include "vw/core/v_array.h"

#include <vector>

namespace ACTION_SCORE
{
struct action_score;
using action_scores = VW::v_array<action_score>;
}  // namespace ACTION_SCORE

namespace VW
{
struct example;
struct workspace;
namespace io
{
struct logger;
struct writer;
}  // namespace io
}  // namespace VW

namespace VW
{
// Each position in outer array is implicitly the decision corresponding to that index. Each inner array is the result
// of CB for that call.
using decision_scores_t = std::vector<ACTION_SCORE::action_scores>;

void print_decision_scores(VW::io::writer* f, const VW::decision_scores_t& decision_scores, VW::io::logger& logger);
std::string to_string(
    const VW::decision_scores_t& decision_scores, int decimal_precision = VW::DEFAULT_FLOAT_PRECISION);

void print_update_ccb(VW::workspace& all, std::vector<example*>& slots, const VW::decision_scores_t& decision_scores,
    size_t num_features);
void print_update_slates(VW::workspace& all, std::vector<example*>& slots, const VW::decision_scores_t& decision_scores,
    size_t num_features);
}  // namespace VW
