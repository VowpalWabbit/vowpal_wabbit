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

label_data::label_data(float label, float weight, float initial)
    : label(label), serialized_weight(weight), serialized_initial(initial)
{
}

void label_data::reset_to_default()
{
  label = FLT_MAX;
  serialized_weight = 1.f;
  serialized_initial = 0.f;
}

char* bufread_simple_label(shared_data* sd, label_data& ld, char* c)
{
  memcpy(&ld.label, c, sizeof(ld.label));
  c += sizeof(ld.label);
  memcpy(&ld.serialized_weight, c, sizeof(ld.serialized_weight));
  c += sizeof(ld.serialized_weight);
  memcpy(&ld.serialized_initial, c, sizeof(ld.serialized_initial));
  c += sizeof(ld.serialized_initial);

  count_label(sd, ld.label);
  return c;
}

size_t read_cached_simple_label(shared_data* sd, label_data& ld, io_buf& cache)
{
  char* c;
  size_t total = sizeof(ld.label) + sizeof(ld.serialized_weight) + sizeof(ld.serialized_initial);
  if (cache.buf_read(c, total) < total) return 0;
  bufread_simple_label(sd, ld, c);

  return total;
}

float get_weight(label_data& ld) { return ld.serialized_weight; }

char* bufcache_simple_label(label_data& ld, char* c)
{
  memcpy(c, &ld.label, sizeof(ld.label));
  c += sizeof(ld.label);
  memcpy(c, &ld.serialized_weight, sizeof(ld.serialized_weight));
  c += sizeof(ld.serialized_weight);
  memcpy(c, &ld.serialized_initial, sizeof(ld.serialized_initial));
  c += sizeof(ld.serialized_initial);
  return c;
}

void cache_simple_label(label_data& ld, io_buf& cache)
{
  char* c;
  cache.buf_write(c, sizeof(ld.label) + sizeof(ld.serialized_weight) + sizeof(ld.serialized_initial));
  bufcache_simple_label(ld, c);
}

void default_simple_label(label_data& ld) { ld.reset_to_default(); }

bool test_label(label_data& ld) { return ld.label == FLT_MAX; }

// Example: 0 1 0.5 'third_house | price:.53 sqft:.32 age:.87 1924
// label := 0, weight := 1, initial := 0.5
void parse_simple_label(
    parser*, shared_data* sd, label_data& ld, std::vector<VW::string_view>& words, reduction_features&)
{
  switch (words.size())
  {
    case 0:
      break;
    case 1:
      ld.label = float_of_string(words[0]);
      break;
    case 2:
      ld.label = float_of_string(words[0]);
      ld.serialized_weight = float_of_string(words[1]);
      break;
    case 3:
      ld.label = float_of_string(words[0]);
      ld.serialized_weight = float_of_string(words[1]);
      ld.serialized_initial = float_of_string(words[2]);
      break;
    default:
      logger::log_error("Error: {0} is too many tokens for a simple label: {1}",
			words.size(), fmt::join(words, " "));
  }
  count_label(sd, ld.label);
}

void post_parse_setup(example* ec) { ec->initial = ec->l.simple.serialized_initial; }

// clang-format off
label_parser simple_label_parser = {
  // default_label
  [](polylabel* v) { default_simple_label(v->simple); },
  // parse_label
  [](parser* p, shared_data* sd, polylabel* v, std::vector<VW::string_view>& words, reduction_features& red_features) {
    parse_simple_label(p, sd, v->simple, words, red_features);
  },
  // cache_label
  [](polylabel* v, reduction_features&, io_buf& cache) { cache_simple_label(v->simple, cache); },
  // read_cached_label
  [](shared_data* sd, polylabel* v, reduction_features&, io_buf& cache) { return read_cached_simple_label(sd, v->simple, cache); },
  // get_weight
  [](polylabel* v, const reduction_features&) { return get_weight(v->simple); },
  // test_label
  [](polylabel* v) { return test_label(v->simple); },
  // test_label
  post_parse_setup,
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
