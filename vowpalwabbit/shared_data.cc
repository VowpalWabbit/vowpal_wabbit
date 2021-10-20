// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#include <iomanip>

#include "shared_data.h"
#include "memory.h"

#include <cfloat>
#include <climits>

shared_data::shared_data(const shared_data& other)
{
  queries = other.queries;
  example_number = other.example_number;
  total_features = other.total_features;
  t = other.t;
  weighted_labeled_examples = other.weighted_labeled_examples;
  old_weighted_labeled_examples = other.old_weighted_labeled_examples;
  weighted_unlabeled_examples = other.weighted_unlabeled_examples;
  weighted_labels = other.weighted_labels;
  sum_loss = other.sum_loss;
  sum_loss_since_last_dump = other.sum_loss_since_last_dump;
  dump_interval = other.dump_interval;
  gravity = other.gravity;
  contraction = other.contraction;
  min_label = other.min_label;
  max_label = other.max_label;
  if (other.ldict) { ldict = VW::make_unique<VW::named_labels>(*other.ldict); }
  weighted_holdout_examples = other.weighted_holdout_examples;
  weighted_holdout_examples_since_last_dump = other.weighted_holdout_examples_since_last_dump;
  holdout_sum_loss_since_last_dump = other.holdout_sum_loss_since_last_dump;
  holdout_sum_loss = other.holdout_sum_loss;
  holdout_best_loss = other.holdout_best_loss;
  weighted_holdout_examples_since_last_pass = other.weighted_holdout_examples_since_last_pass;
  holdout_sum_loss_since_last_pass = other.holdout_sum_loss_since_last_pass;
  holdout_best_pass = other.holdout_best_pass;
  report_multiclass_log_loss = other.report_multiclass_log_loss;
  multiclass_log_loss = other.multiclass_log_loss;
  holdout_multiclass_log_loss = other.holdout_multiclass_log_loss;
  is_more_than_two_labels_observed = other.is_more_than_two_labels_observed;
  first_observed_label = other.first_observed_label;
  second_observed_label = other.second_observed_label;
}

shared_data& shared_data::operator=(const shared_data& other)
{
  if (this == &other) { return *this; }
  queries = other.queries;
  example_number = other.example_number;
  total_features = other.total_features;
  t = other.t;
  weighted_labeled_examples = other.weighted_labeled_examples;
  old_weighted_labeled_examples = other.old_weighted_labeled_examples;
  weighted_unlabeled_examples = other.weighted_unlabeled_examples;
  weighted_labels = other.weighted_labels;
  sum_loss = other.sum_loss;
  sum_loss_since_last_dump = other.sum_loss_since_last_dump;
  dump_interval = other.dump_interval;
  gravity = other.gravity;
  contraction = other.contraction;
  min_label = other.min_label;
  max_label = other.max_label;
  if (other.ldict) { ldict = VW::make_unique<VW::named_labels>(*other.ldict); }
  weighted_holdout_examples = other.weighted_holdout_examples;
  weighted_holdout_examples_since_last_dump = other.weighted_holdout_examples_since_last_dump;
  holdout_sum_loss_since_last_dump = other.holdout_sum_loss_since_last_dump;
  holdout_sum_loss = other.holdout_sum_loss;
  holdout_best_loss = other.holdout_best_loss;
  weighted_holdout_examples_since_last_pass = other.weighted_holdout_examples_since_last_pass;
  holdout_sum_loss_since_last_pass = other.holdout_sum_loss_since_last_pass;
  holdout_best_pass = other.holdout_best_pass;
  report_multiclass_log_loss = other.report_multiclass_log_loss;
  multiclass_log_loss = other.multiclass_log_loss;
  holdout_multiclass_log_loss = other.holdout_multiclass_log_loss;
  is_more_than_two_labels_observed = other.is_more_than_two_labels_observed;
  first_observed_label = other.first_observed_label;
  second_observed_label = other.second_observed_label;
  return *this;
}

shared_data::shared_data(shared_data&& other) noexcept
{
  queries = other.queries;
  example_number = other.example_number;
  total_features = other.total_features;
  t = other.t;
  weighted_labeled_examples = other.weighted_labeled_examples;
  old_weighted_labeled_examples = other.old_weighted_labeled_examples;
  weighted_unlabeled_examples = other.weighted_unlabeled_examples;
  weighted_labels = other.weighted_labels;
  sum_loss = other.sum_loss;
  sum_loss_since_last_dump = other.sum_loss_since_last_dump;
  dump_interval = other.dump_interval;
  gravity = other.gravity;
  contraction = other.contraction;
  min_label = other.min_label;
  max_label = other.max_label;
  ldict = std::move(other.ldict);
  weighted_holdout_examples = other.weighted_holdout_examples;
  weighted_holdout_examples_since_last_dump = other.weighted_holdout_examples_since_last_dump;
  holdout_sum_loss_since_last_dump = other.holdout_sum_loss_since_last_dump;
  holdout_sum_loss = other.holdout_sum_loss;
  holdout_best_loss = other.holdout_best_loss;
  weighted_holdout_examples_since_last_pass = other.weighted_holdout_examples_since_last_pass;
  holdout_sum_loss_since_last_pass = other.holdout_sum_loss_since_last_pass;
  holdout_best_pass = other.holdout_best_pass;
  report_multiclass_log_loss = other.report_multiclass_log_loss;
  multiclass_log_loss = other.multiclass_log_loss;
  holdout_multiclass_log_loss = other.holdout_multiclass_log_loss;
  is_more_than_two_labels_observed = other.is_more_than_two_labels_observed;
  first_observed_label = other.first_observed_label;
  second_observed_label = other.second_observed_label;
}

shared_data& shared_data::operator=(shared_data&& other) noexcept
{
  queries = other.queries;
  example_number = other.example_number;
  total_features = other.total_features;
  t = other.t;
  weighted_labeled_examples = other.weighted_labeled_examples;
  old_weighted_labeled_examples = other.old_weighted_labeled_examples;
  weighted_unlabeled_examples = other.weighted_unlabeled_examples;
  weighted_labels = other.weighted_labels;
  sum_loss = other.sum_loss;
  sum_loss_since_last_dump = other.sum_loss_since_last_dump;
  dump_interval = other.dump_interval;
  gravity = other.gravity;
  contraction = other.contraction;
  min_label = other.min_label;
  max_label = other.max_label;
  ldict = std::move(other.ldict);
  weighted_holdout_examples = other.weighted_holdout_examples;
  weighted_holdout_examples_since_last_dump = other.weighted_holdout_examples_since_last_dump;
  holdout_sum_loss_since_last_dump = other.holdout_sum_loss_since_last_dump;
  holdout_sum_loss = other.holdout_sum_loss;
  holdout_best_loss = other.holdout_best_loss;
  weighted_holdout_examples_since_last_pass = other.weighted_holdout_examples_since_last_pass;
  holdout_sum_loss_since_last_pass = other.holdout_sum_loss_since_last_pass;
  holdout_best_pass = other.holdout_best_pass;
  report_multiclass_log_loss = other.report_multiclass_log_loss;
  multiclass_log_loss = other.multiclass_log_loss;
  holdout_multiclass_log_loss = other.holdout_multiclass_log_loss;
  is_more_than_two_labels_observed = other.is_more_than_two_labels_observed;
  first_observed_label = other.first_observed_label;
  second_observed_label = other.second_observed_label;
  return *this;
}

double shared_data::weighted_examples() const { return weighted_labeled_examples + weighted_unlabeled_examples; }

void shared_data::update(bool test_example, bool labeled_example, float loss, float weight, size_t num_features)
{
  t += weight;
  if (test_example && labeled_example)
  {
    weighted_holdout_examples += weight;  // test weight seen
    weighted_holdout_examples_since_last_dump += weight;
    weighted_holdout_examples_since_last_pass += weight;
    holdout_sum_loss += loss;
    holdout_sum_loss_since_last_dump += loss;
    holdout_sum_loss_since_last_pass += loss;  // since last pass
  }
  else
  {
    if (labeled_example)
      weighted_labeled_examples += weight;
    else
      weighted_unlabeled_examples += weight;
    sum_loss += loss;
    sum_loss_since_last_dump += loss;
    total_features += num_features;
    example_number++;
  }
}

void shared_data::update_dump_interval(bool progress_add, float progress_arg)
{
  sum_loss_since_last_dump = 0.0;
  old_weighted_labeled_examples = weighted_labeled_examples;
  if (progress_add) { dump_interval = static_cast<float>(weighted_examples()) + progress_arg; }
  else
  {
    dump_interval = static_cast<float>(weighted_examples()) * progress_arg;
  }
}

// progressive validation header
void shared_data::print_update_header(std::ostream& trace_message)
{
  trace_message << std::left << std::setw(col_avg_loss) << std::left << "average"
                << " " << std::setw(col_since_last) << std::left << "since"
                << " " << std::right << std::setw(col_example_counter) << "example"
                << " " << std::setw(col_example_weight) << "example"
                << " " << std::setw(col_current_label) << "current"
                << " " << std::setw(col_current_predict) << "current"
                << " " << std::setw(col_current_features) << "current" << std::endl;
  trace_message << std::left << std::setw(col_avg_loss) << std::left << "loss"
                << " " << std::setw(col_since_last) << std::left << "last"
                << " " << std::right << std::setw(col_example_counter) << "counter"
                << " " << std::setw(col_example_weight) << "weight"
                << " " << std::setw(col_current_label) << "label"
                << " " << std::setw(col_current_predict) << "predict"
                << " " << std::setw(col_current_features) << "features" << std::endl;
}

void shared_data::print_update(std::ostream& output_stream, bool holdout_set_off, size_t current_pass, float label,
    float prediction, size_t num_features, bool progress_add, float progress_arg)
{
  std::ostringstream label_buf, pred_buf;

  label_buf << std::setw(col_current_label) << std::setfill(' ');
  if (label < FLT_MAX)
    label_buf << std::setprecision(prec_current_label) << std::fixed << std::right << label;
  else
    label_buf << std::left << " unknown";

  pred_buf << std::setw(col_current_predict) << std::setprecision(prec_current_predict) << std::fixed << std::right
           << std::setfill(' ') << prediction;

  print_update(output_stream, holdout_set_off, current_pass, label_buf.str(), pred_buf.str(), num_features,
      progress_add, progress_arg);
}

void shared_data::print_update(std::ostream& output_stream, bool holdout_set_off, size_t current_pass, uint32_t label,
    uint32_t prediction, size_t num_features, bool progress_add, float progress_arg)
{
  std::ostringstream label_buf, pred_buf;

  label_buf << std::setw(col_current_label) << std::setfill(' ');
  if (label < INT_MAX)
    label_buf << std::right << label;
  else
    label_buf << std::left << " unknown";

  pred_buf << std::setw(col_current_predict) << std::right << std::setfill(' ') << prediction;

  print_update(output_stream, holdout_set_off, current_pass, label_buf.str(), pred_buf.str(), num_features,
      progress_add, progress_arg);
}

void shared_data::print_update(std::ostream& output_stream, bool holdout_set_off, size_t current_pass,
    const std::string& label, uint32_t prediction, size_t num_features, bool progress_add, float progress_arg)
{
  std::ostringstream pred_buf;

  pred_buf << std::setw(col_current_predict) << std::right << std::setfill(' ') << prediction;

  print_update(
      output_stream, holdout_set_off, current_pass, label, pred_buf.str(), num_features, progress_add, progress_arg);
}

void shared_data::print_update(std::ostream& output_stream, bool holdout_set_off, size_t current_pass,
    const std::string& label, const std::string& prediction, size_t num_features, bool progress_add, float progress_arg)
{
  std::streamsize saved_w = output_stream.width();
  std::streamsize saved_prec = output_stream.precision();
  std::ostream::fmtflags saved_f = output_stream.flags();
  bool holding_out = false;

  if (!holdout_set_off && current_pass >= 1)
  {
    if (holdout_sum_loss == 0. && weighted_holdout_examples == 0.)
      output_stream << std::setw(col_avg_loss) << std::left << " unknown";
    else
      output_stream << std::setw(col_avg_loss) << std::setprecision(prec_avg_loss) << std::fixed << std::right
                    << (holdout_sum_loss / weighted_holdout_examples);

    output_stream << " ";

    if (holdout_sum_loss_since_last_dump == 0. && weighted_holdout_examples_since_last_dump == 0.)
      output_stream << std::setw(col_since_last) << std::left << " unknown";
    else
      output_stream << std::setw(col_since_last) << std::setprecision(prec_since_last) << std::fixed << std::right
                    << (holdout_sum_loss_since_last_dump / weighted_holdout_examples_since_last_dump);

    weighted_holdout_examples_since_last_dump = 0;
    holdout_sum_loss_since_last_dump = 0.0;

    holding_out = true;
  }
  else
  {
    output_stream << std::setw(col_avg_loss) << std::setprecision(prec_avg_loss) << std::right << std::fixed;
    if (weighted_labeled_examples > 0.)
      output_stream << (sum_loss / weighted_labeled_examples);
    else
      output_stream << "n.a.";
    output_stream << " " << std::setw(col_since_last) << std::setprecision(prec_avg_loss) << std::right << std::fixed;
    if (weighted_labeled_examples == old_weighted_labeled_examples)
      output_stream << "n.a.";
    else
      output_stream << (sum_loss_since_last_dump / (weighted_labeled_examples - old_weighted_labeled_examples));
  }
  output_stream << " " << std::setw(col_example_counter) << std::right << example_number << " "
                << std::setw(col_example_weight) << std::setprecision(prec_example_weight) << std::right
                << weighted_examples() << " " << std::setw(col_current_label) << std::right << label << " "
                << std::setw(col_current_predict) << std::right << prediction << " " << std::setw(col_current_features)
                << std::right << num_features;

  if (holding_out) output_stream << " h";

  output_stream << std::endl;
  output_stream.flush();

  output_stream.width(saved_w);
  output_stream.precision(saved_prec);
  output_stream.setf(saved_f);

  update_dump_interval(progress_add, progress_arg);
}