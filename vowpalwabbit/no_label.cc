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

namespace no_label
{
char* bufread_no_label(shared_data*, label_data*, char* c) { return c; }

size_t read_cached_no_label(shared_data*, void*, io_buf&) { return 1; }

float get_weight(void*) { return 1.; }

char* bufcache_no_label(label_data*, char* c) { return c; }

void cache_no_label(void*, io_buf&) {}

void default_no_label(void*) {}

bool test_label(void*) { return false; }

void delete_no_label(void*) {}

void parse_no_label(parser*, shared_data*, void*, v_array<VW::string_view>& words)
{
  switch (words.size())
  {
    case 0:
      break;
    default:
      std::cout << "Error: " << words.size() << " is too many tokens for a simple label: ";
      for (const auto & word : words) std::cout << word;
      std::cout << std::endl;
  }
}

label_parser no_label_parser = {default_no_label, parse_no_label, cache_no_label, read_cached_no_label, delete_no_label,
    get_weight, nullptr, test_label, sizeof(nullptr)};

void print_no_label_update(vw& all, example& ec)
{
  if (all.sd->weighted_labeled_examples + all.sd->weighted_unlabeled_examples >= all.sd->dump_interval && !all.quiet &&
      !all.bfgs)
  {
    all.sd->print_update(all.holdout_set_off, all.current_pass, 0.f, ec.pred.scalar, ec.num_features, all.progress_add,
        all.progress_arg);
  }
}

void output_and_account_no_label_example(vw& all, example& ec)
{
  all.sd->update(ec.test_only, false, ec.loss, ec.weight, ec.num_features);

  all.print_by_ref(all.raw_prediction, ec.partial_prediction, -1, ec.tag);
  for (size_t i = 0; i < all.final_prediction_sink.size(); i++)
  {
    int f = (int)all.final_prediction_sink[i];
    all.print_by_ref(f, ec.pred.scalar, 0, ec.tag);
  }

  print_no_label_update(all, ec);
}

void return_no_label_example(vw& all, void*, example& ec)
{
  output_and_account_example(all, ec);
  VW::finish_example(all, ec);
}
}  // namespace no_label
