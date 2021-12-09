// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <cfloat>
#include "gd.h"
#include "vw.h"
#include "example.h"
#include "vw_string_view_fmt.h"
#include "parse_primitives.h"
#include "shared_data.h"
#include "model_utils.h"

#include "io/logger.h"
// needed for printing ranges of objects (eg: all elements of a vector)
#include <fmt/ranges.h>

namespace logger = VW::io::logger;

namespace MULTILABEL
{
float weight(const MULTILABEL::labels&) { return 1.; }

void default_label(MULTILABEL::labels& ld) { ld.label_v.clear(); }

bool test_label(const MULTILABEL::labels& ld) { return ld.label_v.size() == 0; }

void parse_label(
    MULTILABEL::labels& ld, VW::label_parser_reuse_mem& reuse_mem, const std::vector<VW::string_view>& words)
{
  switch (words.size())
  {
    case 0:
      break;
    case 1:
      tokenize(',', words[0], reuse_mem.tokens);

      for (const auto& parse_name : reuse_mem.tokens)
      {
        uint32_t n = int_of_string(parse_name);
        ld.label_v.push_back(n);
      }
      break;
    default:
      logger::errlog_error("example with an odd label, what is {}", fmt::join(words, " "));
  }
}

label_parser multilabel = {
    // default_label
    [](polylabel& label) { default_label(label.multilabels); },
    // parse_label
    [](polylabel& label, reduction_features& /* red_features */, VW::label_parser_reuse_mem& reuse_mem,
        const VW::named_labels* /* ldict */,
        const std::vector<VW::string_view>& words) { parse_label(label.multilabels, reuse_mem, words); },
    // cache_label
    [](const polylabel& label, const reduction_features& /* red_features */, io_buf& cache,
        const std::string& upstream_name,
        bool text) { return VW::model_utils::write_model_field(cache, label.multilabels, upstream_name, text); },
    // read_cached_label
    [](polylabel& label, reduction_features& /* red_features */, io_buf& cache) {
      return VW::model_utils::read_model_field(cache, label.multilabels);
    },
    // get_weight
    [](const polylabel& label, const reduction_features& /* red_features */) { return weight(label.multilabels); },
    // test_label
    [](const polylabel& label) { return test_label(label.multilabels); },
    // label type
    VW::label_type_t::multilabel};

void print_update(VW::workspace& all, bool is_test, const example& ec)
{
  if (all.sd->weighted_examples() >= all.sd->dump_interval && !all.logger.quiet && !all.bfgs)
  {
    std::stringstream label_string;
    if (is_test)
      label_string << " unknown";
    else
      for (uint32_t i : ec.l.multilabels.label_v) { label_string << " " << i; }

    std::stringstream pred_string;
    for (uint32_t i : ec.pred.multilabels.label_v) { pred_string << " " << i; }

    all.sd->print_update(*all.trace_message, all.holdout_set_off, all.current_pass, label_string.str(),
        pred_string.str(), ec.get_num_features(), all.progress_add, all.progress_arg);
  }
}

void output_example(VW::workspace& all, const example& ec)
{
  const auto& ld = ec.l.multilabels;

  float loss = 0.;
  if (!test_label(ld))
  {
    // need to compute exact loss
    const labels& preds = ec.pred.multilabels;
    const labels& given = ec.l.multilabels;

    uint32_t preds_index = 0;
    uint32_t given_index = 0;

    while (preds_index < preds.label_v.size() && given_index < given.label_v.size())
    {
      if (preds.label_v[preds_index] < given.label_v[given_index])
      {
        preds_index++;
        loss++;
      }
      else if (preds.label_v[preds_index] > given.label_v[given_index])
      {
        given_index++;
        loss++;
      }
      else
      {
        preds_index++;
        given_index++;
      }
    }
    loss += given.label_v.size() - given_index;
    loss += preds.label_v.size() - preds_index;
  }

  all.sd->update(ec.test_only, !test_label(ld), loss, 1.f, ec.get_num_features());

  for (auto& sink : all.final_prediction_sink)
  {
    if (sink != nullptr)
    {
      std::stringstream ss;

      for (size_t i = 0; i < ec.pred.multilabels.label_v.size(); i++)
      {
        if (i > 0) ss << ',';
        ss << ec.pred.multilabels.label_v[i];
      }
      ss << ' ';
      all.print_text_by_ref(sink.get(), ss.str(), ec.tag);
    }
  }

  print_update(all, test_label(ld), ec);
}
}  // namespace MULTILABEL

namespace VW
{
namespace model_utils
{
size_t read_model_field(io_buf& io, MULTILABEL::labels& multi)
{
  size_t bytes = 0;
  bytes += read_model_field(io, multi.label_v);
  return bytes;
}
size_t write_model_field(io_buf& io, const MULTILABEL::labels& multi, const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  bytes += write_model_field(io, multi.label_v, upstream_name + "_label_v", text);
  return bytes;
}
}  // namespace model_utils
}  // namespace VW
