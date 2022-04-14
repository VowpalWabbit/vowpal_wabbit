// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/common/string_view.h"
#include "vw/core/accumulate.h"
#include "vw/core/best_constant.h"
#include "vw/core/cache.h"
#include "vw/core/example.h"
#include "vw/core/vw.h"
#include "vw/core/vw_string_view_fmt.h"
#include "vw/io/logger.h"

#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstring>
// needed for printing ranges of objects (eg: all elements of a vector)
#include <fmt/ranges.h>

namespace no_label
{
void parse_no_label(const std::vector<VW::string_view>& words, VW::io::logger& logger)
{
  switch (words.size())
  {
    case 0:
      break;
    default:
      logger.out_error("Error: {0} is too many tokens for a simple label: {1}", words.size(), fmt::join(words, " "));
  }
}

VW::label_parser no_label_parser = {
    // default_label
    [](VW::polylabel& /* label */) {},
    // parse_label
    [](VW::polylabel& /* label */, VW::reduction_features& /* red_features */,
        VW::label_parser_reuse_mem& /* reuse_mem */, const VW::named_labels* /* ldict */,
        const std::vector<VW::string_view>& words, VW::io::logger& logger) { parse_no_label(words, logger); },
    // cache_label
    [](const VW::polylabel& /* label */, const VW::reduction_features& /* red_features */, io_buf& /* cache */,
        const std::string&, bool) -> size_t { return 1; },
    // read_cached_label
    [](VW::polylabel& /* label */, VW::reduction_features& /* red_features */, io_buf &
        /* cache */) -> size_t { return 1; },
    // get_weight
    [](const VW::polylabel& /* label */, const VW::reduction_features& /* red_features */) { return 1.f; },
    // test_label
    [](const VW::polylabel& /* label */) { return false; },
    // label type
    VW::label_type_t::nolabel};

void print_no_label_update(VW::workspace& all, VW::example& ec)
{
  if (all.sd->weighted_labeled_examples + all.sd->weighted_unlabeled_examples >= all.sd->dump_interval && !all.quiet &&
      !all.bfgs)
  {
    all.sd->print_update(*all.trace_message, all.holdout_set_off, all.current_pass, 0.f, ec.pred.scalar,
        ec.get_num_features(), all.progress_add, all.progress_arg);
  }
}

void output_and_account_no_label_example(VW::workspace& all, VW::example& ec)
{
  all.sd->update(ec.test_only, false, ec.loss, ec.weight, ec.get_num_features());

  all.print_by_ref(all.raw_prediction.get(), ec.partial_prediction, -1, ec.tag, all.logger);
  for (auto& sink : all.final_prediction_sink) { all.print_by_ref(sink.get(), ec.pred.scalar, 0, ec.tag, all.logger); }

  print_no_label_update(all, ec);
}

void return_no_label_example(VW::workspace& all, void*, VW::example& ec)
{
  output_and_account_example(all, ec);
  VW::finish_example(all, ec);
}
}  // namespace no_label
