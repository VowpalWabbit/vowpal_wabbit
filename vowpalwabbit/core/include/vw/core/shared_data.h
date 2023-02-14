// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "vw/core/vw_fwd.h"

#include <cfloat>
#include <cstdint>
#include <memory>
#include <string>

namespace VW
{
class shared_data
{
public:
  shared_data();
  ~shared_data();
  shared_data(const shared_data& other);
  shared_data& operator=(const shared_data& other);
  shared_data(shared_data&& other) noexcept;
  shared_data& operator=(shared_data&& other) noexcept;

  size_t queries = 0;

  uint64_t example_number = 0;
  uint64_t total_features = 0;

  double t = 0.0;
  double weighted_labeled_examples = 0.0;
  double old_weighted_labeled_examples = 0.0;
  double weighted_unlabeled_examples = 0.0;
  double weighted_labels = 0.0;
  double sum_loss = 0.0;
  double sum_loss_since_last_dump = 0.0;
  float dump_interval = 1.;  // when should I update for the user.
  double gravity = 0.0;
  double contraction = 1.;
  float min_label = 0.f;  // minimum label encountered
  float max_label = 0.f;  // maximum label encountered

  std::unique_ptr<VW::named_labels> ldict;

  // for holdout
  double weighted_holdout_examples = 0.0;
  double weighted_holdout_examples_since_last_dump = 0.0;
  double holdout_sum_loss_since_last_dump = 0.0;
  double holdout_sum_loss = 0.0;
  // for best model selection
  double holdout_best_loss = 0.0;
  double weighted_holdout_examples_since_last_pass = 0.0;  // reserved for best predictor selection
  double holdout_sum_loss_since_last_pass = 0.0;
  size_t holdout_best_pass = 0;
  // for --probabilities
  bool report_multiclass_log_loss = false;
  double multiclass_log_loss = 0.0;
  double holdout_multiclass_log_loss = 0.0;

  bool is_more_than_two_labels_observed = false;
  float first_observed_label = FLT_MAX;
  float second_observed_label = FLT_MAX;

  // Set by --progress <arg>
  bool progress_add = false;  // additive (rather than multiplicative) progress dumps
  float progress_arg = 2.0;   // next update progress dump multiplier

  double weighted_examples() const;
  void update(bool test_example, bool labeled_example, float loss, float weight, size_t num_features);
  void update_dump_interval();
  /// progressive validation header
  void print_update_header(std::ostream& trace_message);
  void print_update(std::ostream& output_stream, bool holdout_set_off, size_t current_pass, float label,
      float prediction, size_t num_features);
  void print_update(std::ostream& output_stream, bool holdout_set_off, size_t current_pass, uint32_t label,
      uint32_t prediction, size_t num_features);
  void print_update(std::ostream& output_stream, bool holdout_set_off, size_t current_pass, const std::string& label,
      uint32_t prediction, size_t num_features);
  void print_update(std::ostream& output_stream, bool holdout_set_off, size_t current_pass, const std::string& label,
      const std::string& prediction, size_t num_features);
  void print_summary(std::ostream& output, const shared_data& sd, const VW::loss_function& loss_func,
      uint64_t current_pass, bool holdout_set_off) const;
};
}  // namespace VW
