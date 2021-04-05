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
#include "vw_string_view_fmt.h"

#include "io/logger.h"
// needed for printing ranges of objects (eg: all elements of a vector)
#include <fmt/ranges.h>

namespace logger = VW::io::logger;

label_data::label_data() { reset_to_default(); }

label_data::label_data(float label)
    : label(label)
{
}
simple_label_reduction_features::simple_label_reduction_features() { reset_to_default(); }

simple_label_reduction_features::simple_label_reduction_features(float weight, float initial)
    : weight(weight), initial(initial)
{
}

void label_data::reset_to_default()
{
  label = FLT_MAX;
}

void simple_label_reduction_features::reset_to_default()
{
  weight = 1.f;
  initial = 0.f;
}

char* bufread_simple_label(shared_data* sd, label_data& ld, simple_label_reduction_features& red_features, char* c)
{
  memcpy(&ld.label, c, sizeof(ld.label));
  c += sizeof(ld.label);
  memcpy(&red_features.weight, c, sizeof(red_features.weight));
  c += sizeof(red_features.weight);
  memcpy(&red_features.initial, c, sizeof(red_features.initial));
  c += sizeof(red_features.initial);

  count_label(sd, ld.label);
  return c;
}

size_t read_cached_simple_label(shared_data* sd, label_data& ld, reduction_features& red_features, io_buf& cache)
{
  auto& simple_red_features = red_features.get<simple_label_reduction_features>();
  char* c;
  size_t total = sizeof(ld.label) + sizeof(simple_red_features.weight) + sizeof(simple_red_features.initial);
  if (cache.buf_read(c, total) < total) return 0;
  bufread_simple_label(sd, ld, simple_red_features, c);

  return total;
}

float get_weight(label_data&, const reduction_features& red_features) {
  auto& simple_red_features = red_features.get<simple_label_reduction_features>();
  return simple_red_features.weight;
}

char* bufcache_simple_label(label_data& ld, simple_label_reduction_features& red_features, char* c)
{
  memcpy(c, &ld.label, sizeof(ld.label));
  c += sizeof(ld.label);
  memcpy(c, &red_features.weight, sizeof(red_features.weight));
  c += sizeof(red_features.weight);
  memcpy(c, &red_features.initial, sizeof(red_features.initial));
  c += sizeof(red_features.initial);
  return c;
}

void cache_simple_label(label_data& ld, reduction_features& red_features, io_buf& cache)
{
  auto& simple_red_features = red_features.get<simple_label_reduction_features>();
  char* c;
  cache.buf_write(c, sizeof(ld.label) + sizeof(simple_red_features.weight) + sizeof(simple_red_features.initial));
  bufcache_simple_label(ld, simple_red_features, c);
}

void default_simple_label(label_data& ld) { ld.reset_to_default(); }

bool test_label(label_data& ld) { return ld.label == FLT_MAX; }

// Example: 0 1 0.5 'third_house | price:.53 sqft:.32 age:.87 1924
// label := 0, weight := 1, initial := 0.5
void parse_simple_label(
    parser*, shared_data* sd, label_data& ld, std::vector<VW::string_view>& words, reduction_features& red_features)
{
  auto& simple_red_features = red_features.get<simple_label_reduction_features>();
  switch (words.size())
  {
    case 0:
      break;
    case 1:
      ld.label = float_of_string(words[0]);
      break;
    case 2:
      ld.label = float_of_string(words[0]);
      simple_red_features.weight = float_of_string(words[1]);
      break;
    case 3:
      ld.label = float_of_string(words[0]);
      simple_red_features.weight = float_of_string(words[1]);
      simple_red_features.initial = float_of_string(words[2]);
      break;
    default:
      logger::log_error("Error: {0} is too many tokens for a simple label: {1}",
			words.size(), fmt::join(words, " "));
  }
  count_label(sd, ld.label);
}

// clang-format off
label_parser simple_label_parser = {
  // default_label
  [](polylabel* v) { default_simple_label(v->simple); },
  // parse_label
  [](parser* p, shared_data* sd, polylabel* v, std::vector<VW::string_view>& words, reduction_features& red_features) {
    parse_simple_label(p, sd, v->simple, words, red_features);
  },
  // cache_label
  [](polylabel* v, reduction_features& red_features, io_buf& cache) { cache_simple_label(v->simple, red_features, cache); },
  // read_cached_label
  [](shared_data* sd, polylabel* v, reduction_features& red_features, io_buf& cache) { return read_cached_simple_label(sd, v->simple, red_features, cache); },
  // delete_label
  [](polylabel*) {},
   // get_weight
  [](polylabel* v, const reduction_features& red) { return get_weight(v->simple, red); },
  // copy_label
  nullptr,
  // test_label
  [](polylabel* v) { return test_label(v->simple); },
  // test_label
  label_type_t::simple
};
// clang-format on

void print_update(vw& all, example& ec)
{
  if (all.sd->weighted_labeled_examples + all.sd->weighted_unlabeled_examples >= all.sd->dump_interval &&
      !all.logger.quiet && !all.bfgs)
  {
    all.sd->print_update(*all.trace_message, all.holdout_set_off, all.current_pass, ec.l.simple.label, ec.pred.scalar,
        ec.num_features, all.progress_add, all.progress_arg);
  }
}

void output_and_account_example(vw& all, example& ec)
{
  const label_data& ld = ec.l.simple;

  all.sd->update(ec.test_only, ld.label != FLT_MAX, ec.loss, ec.weight, ec.num_features);
  if (ld.label != FLT_MAX && !ec.test_only) all.sd->weighted_labels += ((double)ld.label) * ec.weight;

  all.print_by_ref(all.raw_prediction.get(), ec.partial_prediction, -1, ec.tag);
  for (auto& f : all.final_prediction_sink) { all.print_by_ref(f.get(), ec.pred.scalar, 0, ec.tag); }

  print_update(all, ec);
}

void return_simple_example(vw& all, void*, example& ec)
{
  output_and_account_example(all, ec);
  VW::finish_example(all, ec);
}

bool summarize_holdout_set(vw& all, size_t& no_win_counter)
{
  float thisLoss = (all.sd->weighted_holdout_examples_since_last_pass > 0)
      ? (float)(all.sd->holdout_sum_loss_since_last_pass / all.sd->weighted_holdout_examples_since_last_pass)
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
