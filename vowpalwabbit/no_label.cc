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

namespace no_label
{
void parse_no_label(const std::vector<VW::string_view>& words)
{
  switch (words.size())
  {
    case 0:
      break;
    default:
      std::cout << "Error: " << words.size() << " is too many tokens for a simple label: ";
      for (const auto& word : words) std::cout << word;
      std::cout << std::endl;
  }
}

// clang-format off
label_parser no_label_parser = {
  // default_label
  [](polylabel* v) {},
  // parse_label
  [](parser* p, shared_data* sd, polylabel* v, std::vector<VW::string_view>& words) {
    parse_no_label(words);
  },
  // cache_label
  [](polylabel* v, io_buf& cache) {},
  // read_cached_label
  [](shared_data* sd, polylabel* v, io_buf& cache) { return 1; },
  // delete_label
  [](polylabel*) {},
   // get_weight
  [](polylabel* v) { return 1.; },
  // copy_label
  nullptr,
  // test_label
  [](polylabel* v) { return false; },
};
// clang-format on

void print_no_label_update(vw& all, example& ec)
{
  if (all.sd->weighted_labeled_examples + all.sd->weighted_unlabeled_examples >= all.sd->dump_interval &&
      !all.logger.quiet && !all.bfgs)
  {
    all.sd->print_update(all.holdout_set_off, all.current_pass, 0.f, ec.pred.scalar, ec.num_features, all.progress_add,
        all.progress_arg);
  }
}

void output_and_account_no_label_example(vw& all, example& ec)
{
  all.sd->update(ec.test_only, false, ec.loss, ec.weight, ec.num_features);

  all.print_by_ref(all.raw_prediction.get(), ec.partial_prediction, -1, ec.tag);
  for (auto& sink : all.final_prediction_sink) { all.print_by_ref(sink.get(), ec.pred.scalar, 0, ec.tag); }

  print_no_label_update(all, ec);
}

void return_no_label_example(vw& all, void*, example& ec)
{
  output_and_account_example(all, ec);
  VW::finish_example(all, ec);
}
}  // namespace no_label
