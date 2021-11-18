// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <cfloat>
#include <algorithm>

#include "example.h"
#include "parse_primitives.h"
#include "vw.h"
#include "vw_exception.h"
#include "example.h"
#include "cb_label_parser.h"
#include "vw_string_view.h"
#include "shared_data.h"

#include "io/logger.h"

using namespace VW::LEARNER;

namespace logger = VW::io::logger;

namespace CB
{
// This return value should be treated like an optional. If first is false then the second value should not be read.
std::pair<bool, cb_class> get_observed_cost_cb(const label& ld)
{
  for (const auto& cl : ld.costs)
    if (cl.has_observed_cost()) return std::make_pair(true, cl);

  // Default value for cb_class does not have an observed cost.
  return std::make_pair(false, CB::cb_class{});
}

void parse_label(CB::label& ld, VW::label_parser_reuse_mem& reuse_mem, const std::vector<VW::string_view>& words)
{
  ld.weight = 1.0;

  for (auto const& word : words)
  {
    // Format is the following:
    // <action>:<cost>:<probability> | shared
    // for example "1:2:0.5"
    // action = 1, cost = 2, probability = 0.5
    cb_class f;
    tokenize(':', word, reuse_mem.tokens);

    if (reuse_mem.tokens.empty() || reuse_mem.tokens.size() > 3) { THROW("malformed cost specification: " << word); }

    f.partial_prediction = 0.;
    f.action = static_cast<uint32_t>(hashstring(reuse_mem.tokens[0].begin(), reuse_mem.tokens[0].length(), 0));
    f.cost = FLT_MAX;

    if (reuse_mem.tokens.size() > 1) f.cost = float_of_string(reuse_mem.tokens[1]);

    if (std::isnan(f.cost)) THROW("error NaN cost (" << reuse_mem.tokens[1] << " for action: " << reuse_mem.tokens[0]);

    f.probability = .0;
    if (reuse_mem.tokens.size() > 2) f.probability = float_of_string(reuse_mem.tokens[2]);

    if (std::isnan(f.probability))
      THROW("error NaN probability (" << reuse_mem.tokens[2] << " for action: " << reuse_mem.tokens[0]);

    if (f.probability > 1.0)
    {
      logger::errlog_warn("invalid probability > 1 specified for an action, resetting to 1.");
      f.probability = 1.0;
    }
    if (f.probability < 0.0)
    {
      logger::errlog_warn("invalid probability < 0 specified for an action, resetting to 0.");
      f.probability = .0;
    }
    if (reuse_mem.tokens[0] == "shared")
    {
      if (reuse_mem.tokens.size() == 1) { f.probability = -1.f; }
      else
      {
        logger::errlog_warn("shared feature vectors should not have costs");
      }
    }

    ld.costs.push_back(f);
  }
}

label_parser cb_label = {
    // default_label
    [](polylabel& label) { CB::default_label(label.cb); },
    // parse_label
    [](polylabel& label, reduction_features& /*red_features*/, VW::label_parser_reuse_mem& reuse_mem,
        const VW::named_labels* /*ldict*/,
        const std::vector<VW::string_view>& words) { CB::parse_label(label.cb, reuse_mem, words); },
    // cache_label
    [](const polylabel& label, const reduction_features& /*red_features*/, io_buf& cache) {
      CB::cache_label(label.cb, cache);
    },
    // read_cached_label
    [](polylabel& label, reduction_features& /*red_features*/, io_buf& cache) {
      return CB::read_cached_label(label.cb, cache);
    },
    // get_weight
    [](const polylabel& label, const reduction_features& /*red_features*/) { return label.cb.weight; },
    // test_label
    [](const polylabel& label) { return CB::is_test_label(label.cb); },
    // Label type
    VW::label_type_t::cb};

bool ec_is_example_header(example const& ec)  // example headers just have "shared"
{
  const auto& costs = ec.l.cb.costs;
  if (costs.size() != 1) return false;
  if (costs[0].probability == -1.f) return true;
  return false;
}

std::string known_cost_to_str(CB::cb_class* known_cost)
{
  if (known_cost == nullptr) return " known";

  std::stringstream label_string;
  label_string.precision(2);
  label_string << known_cost->action << ":" << known_cost->cost << ":" << known_cost->probability;
  return label_string.str();
}

void print_update(
    VW::workspace& all, bool is_test, example& ec, multi_ex* ec_seq, bool action_scores, CB::cb_class* known_cost)
{
  if (all.sd->weighted_examples() >= all.sd->dump_interval && !all.logger.quiet && !all.bfgs)
  {
    size_t num_features = ec.get_num_features();

    size_t pred = ec.pred.multiclass;
    if (ec_seq != nullptr)
    {
      num_features = 0;
      // TODO: code duplication csoaa.cc LabelDict::ec_is_example_header
      for (size_t i = 0; i < (*ec_seq).size(); i++)
      {
        if (CB::ec_is_example_header(*(*ec_seq)[i]))
        { num_features += (ec_seq->size() - 1) * (*ec_seq)[i]->get_num_features(); }
        else
        {
          num_features += (*ec_seq)[i]->get_num_features();
        }
      }
    }
    std::string label_buf;
    if (is_test)
      label_buf = " unknown";
    else
      label_buf = known_cost_to_str(known_cost);

    if (action_scores)
    {
      std::ostringstream pred_buf;
      pred_buf << std::setw(shared_data::col_current_predict) << std::right << std::setfill(' ');
      if (!ec.pred.a_s.empty())
        pred_buf << ec.pred.a_s[0].action << ":" << ec.pred.a_s[0].score << "...";
      else
        pred_buf << "no action";
      all.sd->print_update(*all.trace_message, all.holdout_set_off, all.current_pass, label_buf, pred_buf.str(),
          num_features, all.progress_add, all.progress_arg);
    }
    else
      all.sd->print_update(*all.trace_message, all.holdout_set_off, all.current_pass, label_buf,
          static_cast<uint32_t>(pred), num_features, all.progress_add, all.progress_arg);
  }
}
}  // namespace CB

namespace CB_EVAL
{
float weight(CB_EVAL::label& ld) { return ld.event.weight; }

size_t read_cached_label(CB_EVAL::label& ld, io_buf& cache)
{
  char* c;
  size_t total = sizeof(uint32_t);
  if (cache.buf_read(c, total) < total) return 0;
  ld.action = *reinterpret_cast<uint32_t*>(c);

  return total + CB::read_cached_label(ld.event, cache);
}

void cache_label(const CB_EVAL::label& ld, io_buf& cache)
{
  char* c;
  cache.buf_write(c, sizeof(uint32_t));
  *reinterpret_cast<uint32_t*>(c) = ld.action;

  CB::cache_label(ld.event, cache);
}

void default_label(CB_EVAL::label& ld)
{
  CB::default_label(ld.event);
  ld.action = 0;
}

bool test_label(const CB_EVAL::label& ld) { return CB::is_test_label(ld.event); }

void parse_label(CB_EVAL::label& ld, VW::label_parser_reuse_mem& reuse_mem, const std::vector<VW::string_view>& words)
{
  if (words.size() < 2) THROW("Evaluation can not happen without an action and an exploration");

  ld.action = static_cast<uint32_t>(hashstring(words[0].begin(), words[0].length(), 0));

  // TODO - make this a span and there is no allocation
  const auto rest_of_tokens = std::vector<VW::string_view>(words.begin() + 1, words.end());
  CB::parse_label(ld.event, reuse_mem, rest_of_tokens);
}

label_parser cb_eval = {
    // default_label
    [](polylabel& label) { CB_EVAL::default_label(label.cb_eval); },
    // parse_label
    [](polylabel& label, reduction_features& /*red_features*/, VW::label_parser_reuse_mem& reuse_mem,
        const VW::named_labels* /*ldict*/,
        const std::vector<VW::string_view>& words) { CB_EVAL::parse_label(label.cb_eval, reuse_mem, words); },
    // cache_label
    [](const polylabel& label, const reduction_features& /*red_features*/, io_buf& cache) {
      CB_EVAL::cache_label(label.cb_eval, cache);
    },
    // read_cached_label
    [](polylabel& label, reduction_features& /*red_features*/, io_buf& cache) {
      return CB_EVAL::read_cached_label(label.cb_eval, cache);
    },
    // get_weight
    [](const polylabel& /*label*/, const reduction_features& /*red_features*/) { return 1.f; },
    // test_label
    [](const polylabel& label) { return CB_EVAL::test_label(label.cb_eval); },
    // Label type
    VW::label_type_t::cb_eval};

}  // namespace CB_EVAL
