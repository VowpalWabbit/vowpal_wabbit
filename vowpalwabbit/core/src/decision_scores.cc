// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/decision_scores.h"

#include "vw/core/action_score.h"
#include "vw/core/global_data.h"
#include "vw/core/io_buf.h"
#include "vw/core/reductions/conditional_contextual_bandit.h"
#include "vw/core/reductions/slates.h"
#include "vw/core/shared_data.h"
#include "vw/core/text_utils.h"
#include "vw/io/errno_handling.h"
#include "vw/io/logger.h"

#include <fmt/ostream.h>

#include <iostream>

template <typename LabelPrintFunc>
void print_update(VW::workspace& all, const VW::multi_ex& slots, const VW::decision_scores_t& decision_scores,
    size_t num_features, LabelPrintFunc label_print_func)
{
  std::ostringstream pred_ss;
  std::string delim;
  for (const auto& slot : decision_scores)
  {
    pred_ss << delim << slot[0].action;
    delim = ",";
  }
  all.sd->print_update(
      *all.trace_message, all.holdout_set_off, all.current_pass, label_print_func(slots), pred_ss.str(), num_features);
}

namespace VW
{
void print_decision_scores(VW::io::writer* f, const VW::decision_scores_t& decision_scores, VW::io::logger& logger)
{
  if (f != nullptr)
  {
    std::stringstream ss;
    ss << to_string(decision_scores);
    const auto str = ss.str();
    ssize_t len = str.size();
    ssize_t t = f->write(str.c_str(), static_cast<unsigned int>(len));
    if (t != len) { logger.err_error("write error: {}", VW::io::strerror_to_string(errno)); }
  }
}

std::string to_string(const VW::decision_scores_t& decision_scores, int decimal_precision)
{
  std::ostringstream ss;

  for (const auto& slot : decision_scores)
  {
    std::string delimiter;
    for (const auto& action_score : slot)
    {
      ss << delimiter
         << fmt::format("{}:{}", action_score.action, VW::fmt_float(action_score.score, decimal_precision));
      delimiter = ",";
    }
    ss << '\n';
  }
  return ss.str();
}

void print_update_ccb(VW::workspace& all, const std::vector<example*>& slots,
    const VW::decision_scores_t& decision_scores, size_t num_features)
{
  print_update(all, slots, decision_scores, num_features, reductions::ccb::generate_ccb_label_printout);
}

void print_update_slates(VW::workspace& all, const std::vector<example*>& slots,
    const VW::decision_scores_t& decision_scores, size_t num_features)
{
  print_update(all, slots, decision_scores, num_features, VW::reductions::generate_slates_label_printout);
}
}  // namespace VW
