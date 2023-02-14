// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/simple_label.h"
#include "vw/core/vw_string_view_fmt.h"

#include "vw/common/string_view.h"
#include "vw/core/accumulate.h"
#include "vw/core/best_constant.h"
#include "vw/core/example.h"
#include "vw/core/parse_primitives.h"
#include "vw/core/vw.h"
#include "vw/io/logger.h"

#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstring>
// needed for printing ranges of objects (eg: all elements of a vector)
#include <fmt/ranges.h>

VW::simple_label::simple_label() { reset_to_default(); }

VW::simple_label::simple_label(float label) : label(label) {}

void VW::simple_label::reset_to_default() { label = FLT_MAX; }

// TODO: Delete once there are no more usages.
void VW::details::print_update(VW::workspace& all, const VW::example& ec)
{
  if (all.sd->weighted_labeled_examples + all.sd->weighted_unlabeled_examples >= all.sd->dump_interval && !all.quiet &&
      !all.bfgs)
  {
    all.sd->print_update(*all.trace_message, all.holdout_set_off, all.current_pass, ec.l.simple.label, ec.pred.scalar,
        ec.get_num_features());
  }
}

void VW::details::output_and_account_example(VW::workspace& all, const VW::example& ec)
{
  const simple_label& ld = ec.l.simple;

  all.sd->update(ec.test_only, ld.label != FLT_MAX, ec.loss, ec.weight, ec.get_num_features());
  if (ld.label != FLT_MAX && !ec.test_only) { all.sd->weighted_labels += (static_cast<double>(ld.label)) * ec.weight; }

  all.print_by_ref(all.raw_prediction.get(), ec.partial_prediction, -1, ec.tag, all.logger);
  for (auto& f : all.final_prediction_sink) { all.print_by_ref(f.get(), ec.pred.scalar, 0, ec.tag, all.logger); }

  print_update(all, ec);
}

void VW::details::return_simple_example(VW::workspace& all, void*, VW::example& ec)
{
  output_and_account_example(all, ec);
  VW::finish_example(all, ec);
}

void VW::details::update_stats_simple_label(
    const VW::workspace& /* all */, shared_data& sd, const VW::example& ec, VW::io::logger& /* logger */)
{
  const auto& ld = ec.l.simple;
  sd.update(ec.test_only, ld.label != FLT_MAX, ec.loss, ec.weight, ec.get_num_features());
  if (ld.label != FLT_MAX && !ec.test_only) { sd.weighted_labels += (static_cast<double>(ld.label)) * ec.weight; }
}

void VW::details::print_update_simple_label(
    VW::workspace& all, shared_data& sd, const VW::example& ec, VW::io::logger& /* logger */)
{
  const bool should_print_driver_update =
      all.sd->weighted_examples() >= all.sd->dump_interval && !all.quiet && !all.bfgs;

  if (should_print_driver_update)
  {
    sd.print_update(*all.trace_message, all.holdout_set_off, all.current_pass, ec.l.simple.label, ec.pred.scalar,
        ec.get_num_features());
  }
}

void VW::details::output_example_prediction_simple_label(
    VW::workspace& all, const VW::example& ec, VW::io::logger& /* logger */)
{
  all.print_by_ref(all.raw_prediction.get(), ec.partial_prediction, -1, ec.tag, all.logger);
  for (auto& f : all.final_prediction_sink) { all.print_by_ref(f.get(), ec.pred.scalar, 0, ec.tag, all.logger); }
}

bool VW::details::summarize_holdout_set(VW::workspace& all, size_t& no_win_counter)
{
  float this_loss = (all.sd->weighted_holdout_examples_since_last_pass > 0)
      ? static_cast<float>(all.sd->holdout_sum_loss_since_last_pass / all.sd->weighted_holdout_examples_since_last_pass)
      : FLT_MAX * 0.5f;
  if (all.all_reduce != nullptr) { this_loss = accumulate_scalar(all, this_loss); }

  all.sd->weighted_holdout_examples_since_last_pass = 0;
  all.sd->holdout_sum_loss_since_last_pass = 0;

  if (this_loss < all.sd->holdout_best_loss)
  {
    all.sd->holdout_best_loss = this_loss;
    all.sd->holdout_best_pass = all.current_pass;
    no_win_counter = 0;
    return true;
  }

  if ((this_loss != FLT_MAX) || (std::isfinite(all.sd->holdout_best_loss)))
  {  // it's only a loss if we're not infinite when the previous one wasn't infinite
    no_win_counter++;
  }
  return false;
}
