// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <cstring>
#include <cfloat>
#include <cmath>
#include <cstdio>

#include "cache.h"
#include "accumulate.h"
#include "best_constant.h"
#include "vw_string_view.h"
#include "example.h"
#include "parse_primitives.h"
#include "vw.h"
#include "vw_string_view_fmt.h"
#include "model_utils.h"

#include "io/logger.h"
// needed for printing ranges of objects (eg: all elements of a vector)
#include <fmt/ranges.h>

namespace logger = VW::io::logger;

label_data::label_data() { reset_to_default(); }

label_data::label_data(float label) : label(label) {}

void label_data::reset_to_default()
{
  label = FLT_MAX;
}

void print_update(VW::workspace& all, const example& ec)
{
  if (all.sd->weighted_labeled_examples + all.sd->weighted_unlabeled_examples >= all.sd->dump_interval &&
      !all.logger.quiet && !all.bfgs)
  {
    all.sd->print_update(*all.trace_message, all.holdout_set_off, all.current_pass, ec.l.simple.label, ec.pred.scalar,
        ec.get_num_features(), all.progress_add, all.progress_arg);
  }
}

void output_and_account_example(VW::workspace& all, const example& ec)
{
  const label_data& ld = ec.l.simple;

  all.sd->update(ec.test_only, ld.label != FLT_MAX, ec.loss, ec.weight, ec.get_num_features());
  if (ld.label != FLT_MAX && !ec.test_only) all.sd->weighted_labels += (static_cast<double>(ld.label)) * ec.weight;

  all.print_by_ref(all.raw_prediction.get(), ec.partial_prediction, -1, ec.tag);
  for (auto& f : all.final_prediction_sink) { all.print_by_ref(f.get(), ec.pred.scalar, 0, ec.tag); }

  print_update(all, ec);
}

void return_simple_example(VW::workspace& all, void*, example& ec)
{
  output_and_account_example(all, ec);
  VW::finish_example(all, ec);
}

bool summarize_holdout_set(VW::workspace& all, size_t& no_win_counter)
{
  float thisLoss = (all.sd->weighted_holdout_examples_since_last_pass > 0)
      ? static_cast<float>(all.sd->holdout_sum_loss_since_last_pass / all.sd->weighted_holdout_examples_since_last_pass)
      : FLT_MAX * 0.5f;
  if (all.all_reduce != nullptr) thisLoss = accumulate_scalar(all, thisLoss);

  all.sd->weighted_holdout_examples_since_last_pass = 0;
  all.sd->holdout_sum_loss_since_last_pass = 0;

  if (thisLoss < all.sd->holdout_best_loss)
  {
    all.sd->holdout_best_loss = thisLoss;
    all.sd->holdout_best_pass = all.current_pass;
    no_win_counter = 0;
    return true;
  }

  if ((thisLoss != FLT_MAX) ||
      (std::isfinite(
          all.sd->holdout_best_loss)))  // it's only a loss if we're not infinite when the previous one wasn't infinite
    no_win_counter++;
  return false;
}

namespace VW
{
namespace model_utils
{
size_t read_model_field(io_buf& io, label_data& ld)
{
  size_t bytes = 0;
  bytes += read_model_field(io, ld.label);
  return bytes;
}
size_t write_model_field(io_buf& io, const label_data& ld, const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  bytes += write_model_field(io, ld.label, upstream_name + "_label", text);
  return bytes;
}
size_t read_model_field(io_buf& io, simple_label_reduction_features& slrf)
{
  size_t bytes = 0;
  bytes += read_model_field(io, slrf.weight);
  bytes += read_model_field(io, slrf.initial);
  return bytes;
}
size_t write_model_field(
    io_buf& io, const simple_label_reduction_features& slrf, const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  bytes += write_model_field(io, slrf.weight, upstream_name + "_weight", text);
  bytes += write_model_field(io, slrf.initial, upstream_name + "_initial", text);
  return bytes;
}
}  // namespace model_utils
}  // namespace VW
