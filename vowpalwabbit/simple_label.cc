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

char* bufread_simple_label(shared_data* sd, label_data* ld, char* c)
{
  memcpy(&ld->label, c, sizeof(ld->label));
  //  std::cout << ld->label << " " << sd->is_more_than_two_labels_observed << " " << sd->first_observed_label <<
  //  std::endl;
  c += sizeof(ld->label);
  memcpy(&ld->weight, c, sizeof(ld->weight));
  c += sizeof(ld->weight);
  memcpy(&ld->initial, c, sizeof(ld->initial));
  c += sizeof(ld->initial);

  count_label(sd, ld->label);
  return c;
}

size_t read_cached_simple_label(shared_data* sd, void* v, io_buf& cache)
{
  label_data* ld = (label_data*)v;
  char* c;
  size_t total = sizeof(ld->label) + sizeof(ld->weight) + sizeof(ld->initial);
  if (cache.buf_read(c, total) < total)
    return 0;
  bufread_simple_label(sd, ld, c);

  return total;
}

float get_weight(void* v)
{
  label_data* ld = (label_data*)v;
  return ld->weight;
}

char* bufcache_simple_label(label_data* ld, char* c)
{
  memcpy(c, &ld->label, sizeof(ld->label));
  c += sizeof(ld->label);
  memcpy(c, &ld->weight, sizeof(ld->weight));
  c += sizeof(ld->weight);
  memcpy(c, &ld->initial, sizeof(ld->initial));
  c += sizeof(ld->initial);
  return c;
}

void cache_simple_label(void* v, io_buf& cache)
{
  char* c;
  label_data* ld = (label_data*)v;
  cache.buf_write(c, sizeof(ld->label) + sizeof(ld->weight) + sizeof(ld->initial));
  bufcache_simple_label(ld, c);
}

void default_simple_label(void* v)
{
  label_data* ld = (label_data*)v;
  ld->label = FLT_MAX;
  ld->weight = 1.;
  ld->initial = 0.;
}

bool test_label(void* v)
{
  label_data* ld = (label_data*)v;
  return ld->label == FLT_MAX;
}

void delete_simple_label(void*) {}

void parse_simple_label(parser*, shared_data* sd, void* v, v_array<VW::string_view>& words)
{
  label_data* ld = (label_data*)v;

  switch (words.size())
  {
    case 0:
      break;
    case 1:
      ld->label = float_of_string(words[0]);
      break;
    case 2:
      ld->label = float_of_string(words[0]);
      ld->weight = float_of_string(words[1]);
      break;
    case 3:
      ld->label = float_of_string(words[0]);
      ld->weight = float_of_string(words[1]);
      ld->initial = float_of_string(words[2]);
      break;
    default:
      std::cout << "Error: " << words.size() << " is too many tokens for a simple label: ";
      for (const auto & word : words) std::cout << word;
      std::cout << std::endl;
  }
  count_label(sd, ld->label);
}

label_parser simple_label = {default_simple_label, parse_simple_label, cache_simple_label, read_cached_simple_label,
    delete_simple_label, get_weight, nullptr, test_label, sizeof(label_data)};

void print_update(vw& all, example& ec)
{
  if (all.sd->weighted_labeled_examples + all.sd->weighted_unlabeled_examples >= all.sd->dump_interval && !all.quiet &&
      !all.bfgs)
  {
    all.sd->print_update(all.holdout_set_off, all.current_pass, ec.l.simple.label, ec.pred.scalar, ec.num_features,
        all.progress_add, all.progress_arg);
  }
}

void output_and_account_example(vw& all, example& ec)
{
  label_data ld = ec.l.simple;

  all.sd->update(ec.test_only, ld.label != FLT_MAX, ec.loss, ec.weight, ec.num_features);
  if (ld.label != FLT_MAX && !ec.test_only)
    all.sd->weighted_labels += ((double)ld.label) * ec.weight;

  all.print_by_ref(all.raw_prediction, ec.partial_prediction, -1, ec.tag);
  for (size_t i = 0; i < all.final_prediction_sink.size(); i++)
  {
    int f = (int)all.final_prediction_sink[i];
    all.print_by_ref(f, ec.pred.scalar, 0, ec.tag);
  }

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
  if (all.all_reduce != nullptr)
    thisLoss = accumulate_scalar(all, thisLoss);

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
