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
  memcpy(&ld->serialized_weight, c, sizeof(ld->serialized_weight));
  c += sizeof(ld->serialized_weight);
  memcpy(&ld->serialized_initial, c, sizeof(ld->serialized_initial));
  c += sizeof(ld->serialized_initial);

  count_label(sd, ld->label);
  return c;
}

size_t read_cached_simple_label(shared_data* sd, void* v, io_buf& cache)
{
  label_data* ld = (label_data*)v;
  char* c;
  size_t total = sizeof(ld->label) + sizeof(ld->serialized_weight) + sizeof(ld->serialized_initial);
  if (cache.buf_read(c, total) < total)
    return 0;
  bufread_simple_label(sd, ld, c);

  return total;
}

float get_weight(void* v)
{
  label_data* ld = (label_data*)v;
  return ld->serialized_weight;
}

char* bufcache_simple_label(label_data* ld, char* c)
{
  memcpy(c, &ld->label, sizeof(ld->label));
  c += sizeof(ld->label);
  memcpy(c, &ld->serialized_weight, sizeof(ld->serialized_weight));
  c += sizeof(ld->serialized_weight);
  memcpy(c, &ld->serialized_initial, sizeof(ld->serialized_initial));
  c += sizeof(ld->serialized_initial);
  return c;
}

void cache_simple_label(void* v, io_buf& cache)
{
  char* c;
  label_data* ld = (label_data*)v;
  cache.buf_write(c, sizeof(ld->label) + sizeof(ld->serialized_weight) + sizeof(ld->serialized_initial));
  bufcache_simple_label(ld, c);
}

void default_simple_label(void* v)
{
  label_data* ld = (label_data*)v;
  ld->label = FLT_MAX;
  ld->serialized_weight = 1.;
  ld->serialized_initial = 0.;
}

bool test_label(void* v)
{
  label_data* ld = (label_data*)v;
  return ld->label == FLT_MAX;
}

void delete_simple_label(void*) {}

// Example: 0 1 0.5 'third_house | price:.53 sqft:.32 age:.87 1924
// label := 0, weight := 1, initial := 0.5
void parse_simple_label(parser*, shared_data* sd, void* v, std::vector<VW::string_view>& words)
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
      ld->serialized_weight = float_of_string(words[1]);
      break;
    case 3:
      ld->label = float_of_string(words[0]);
      ld->serialized_weight = float_of_string(words[1]);
      ld->serialized_initial = float_of_string(words[2]);
      break;
    default:
      std::cout << "Error: " << words.size() << " is too many tokens for a simple label: ";
      for (const auto & word : words) std::cout << word;
      std::cout << std::endl;
  }
  count_label(sd, ld->label);
}

void post_parse_setup(example* ec) { ec->initial = ec->l.simple.serialized_initial; }

label_parser simple_label_parser = {
    default_simple_label,      // label_data default constructor
    parse_simple_label,        // parse input stream of words into label_data
    cache_simple_label,        // write label to cache
    read_cached_simple_label,  // read label from cache
    delete_simple_label, get_weight,
    nullptr,     // deep copy of label
    test_label,  // is ths a test label?
    sizeof(label_data),
    post_parse_setup  // called after example is completely parsed so that
                      // label specific fixups can happen
                      // (for example serialized_initial used by gd)
};

void print_update(vw& all, example& ec)
{
  if (all.sd->weighted_labeled_examples + all.sd->weighted_unlabeled_examples >= all.sd->dump_interval &&
      !all.logger.quiet && !all.bfgs)
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

  all.print_by_ref(all.raw_prediction.get(), ec.partial_prediction, -1, ec.tag);
  for (auto& f : all.final_prediction_sink)
  {
    all.print_by_ref(f.get(), ec.pred.scalar, 0, ec.tag);
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
