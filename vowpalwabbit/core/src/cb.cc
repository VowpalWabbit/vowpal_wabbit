// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/common/string_view.h"
#include "vw/common/text_utils.h"
#include "vw/common/vw_exception.h"
#include "vw/core/cb_label_parser.h"
#include "vw/core/example.h"
#include "vw/core/model_utils.h"
#include "vw/core/parse_primitives.h"
#include "vw/core/shared_data.h"
#include "vw/core/text_utils.h"
#include "vw/core/vw.h"
#include "vw/io/logger.h"

#include <algorithm>
#include <cfloat>

// This return value should be treated like an optional. If first is false then the second value should not be read.
std::pair<bool, VW::cb_class> VW::get_observed_cost_cb(const cb_label& ld)
{
  for (const auto& cl : ld.costs)
  {
    if (cl.has_observed_cost()) { return std::make_pair(true, cl); }
  }

  // Default value for cb_class does not have an observed cost.
  return std::make_pair(false, VW::cb_class{});
}

bool VW::is_cb_example_header(VW::example const& ec)  // example headers just have "shared"
{
  const auto& costs = ec.l.cb.costs;
  if (costs.size() != 1) { return false; }
  if (costs[0].probability == -1.f) { return true; }
  return false;
}
namespace
{
void parse_cb_label(VW::cb_label& ld, VW::label_parser_reuse_mem& reuse_mem, const std::vector<VW::string_view>& words,
    VW::io::logger& logger)
{
  ld.weight = 1.0;

  for (auto const& word : words)
  {
    // Format is the following:
    // <action>:<cost>:<probability> | shared
    // for example "1:2:0.5"
    // action = 1, cost = 2, probability = 0.5
    VW::cb_class f;
    VW::tokenize(':', word, reuse_mem.tokens);

    if (reuse_mem.tokens.empty() || reuse_mem.tokens.size() > 3) { THROW("malformed cost specification: " << word); }

    f.partial_prediction = 0.;
    f.action = static_cast<uint32_t>(hashstring(reuse_mem.tokens[0].data(), reuse_mem.tokens[0].length(), 0));
    f.cost = FLT_MAX;

    if (reuse_mem.tokens.size() > 1) { f.cost = float_of_string(reuse_mem.tokens[1], logger); }

    if (std::isnan(f.cost)) THROW("error NaN cost (" << reuse_mem.tokens[1] << " for action: " << reuse_mem.tokens[0]);

    f.probability = .0;
    if (reuse_mem.tokens.size() > 2) { f.probability = float_of_string(reuse_mem.tokens[2], logger); }

    if (std::isnan(f.probability))
      THROW("error NaN probability (" << reuse_mem.tokens[2] << " for action: " << reuse_mem.tokens[0]);

    if (f.probability > 1.0)
    {
      logger.err_warn("invalid probability > 1 specified for an action, resetting to 1.");
      f.probability = 1.0;
    }
    if (f.probability < 0.0)
    {
      logger.err_warn("invalid probability < 0 specified for an action, resetting to 0.");
      f.probability = .0;
    }
    if (reuse_mem.tokens[0] == "shared")
    {
      if (reuse_mem.tokens.size() == 1) { f.probability = -1.f; }
      else
      {
        logger.err_warn("shared feature vectors should not have costs");
      }
    }

    ld.costs.push_back(f);
  }
}

std::string known_cost_to_str(const VW::cb_class* known_cost)
{
  if (known_cost == nullptr) { return " known"; }

  std::stringstream label_string;
  label_string.precision(VW::DEFAULT_FLOAT_FORMATTING_DECIMAL_PRECISION);
  label_string << known_cost->action << ":" << known_cost->cost << ":" << known_cost->probability;
  return label_string.str();
}

float cb_eval_weight(const VW::cb_eval_label& ld) { return ld.event.weight; }

void default_cb_eval_label(VW::cb_eval_label& ld)
{
  VW::details::default_cb_label(ld.event);
  ld.action = 0;
}

bool test_cb_eval_label(const VW::cb_eval_label& ld) { return VW::details::is_test_cb_label(ld.event); }

void parse_cb_eval_label(VW::cb_eval_label& ld, VW::label_parser_reuse_mem& reuse_mem,
    const std::vector<VW::string_view>& words, VW::io::logger& logger)
{
  if (words.size() < 2) THROW("Evaluation can not happen without an action and an exploration");

  ld.action = static_cast<uint32_t>(hashstring(words[0].data(), words[0].length(), 0));

  // TODO - make this a span and there is no allocation
  const auto rest_of_tokens = std::vector<VW::string_view>(words.begin() + 1, words.end());
  parse_cb_label(ld.event, reuse_mem, rest_of_tokens, logger);
}
}  // namespace

namespace VW
{
VW::label_parser cb_label_parser_global = {
    // default_label
    [](VW::polylabel& label) { VW::details::default_cb_label(label.cb); },
    // parse_label
    [](VW::polylabel& label, VW::reduction_features& /*red_features*/, VW::label_parser_reuse_mem& reuse_mem,
        const VW::named_labels* /*ldict*/, const std::vector<VW::string_view>& words,
        VW::io::logger& logger) { parse_cb_label(label.cb, reuse_mem, words, logger); },
    // cache_label
    [](const VW::polylabel& label, const VW::reduction_features& /*red_features*/, io_buf& cache,
        const std::string& upstream_name,
        bool text) { return VW::model_utils::write_model_field(cache, label.cb, upstream_name, text); },
    // read_cached_label
    [](VW::polylabel& label, VW::reduction_features& /*red_features*/, io_buf& cache) {
      return VW::model_utils::read_model_field(cache, label.cb);
    },
    // get_weight
    [](const VW::polylabel& label, const VW::reduction_features& /*red_features*/) { return label.cb.weight; },
    // test_label
    [](const VW::polylabel& label) { return VW::details::is_test_cb_label(label.cb); },
    // Label type
    VW::label_type_t::cb};

VW::label_parser cb_eval_label_parser_global = {
    // default_label
    [](VW::polylabel& label) { cb_eval_weight(label.cb_eval); },
    // parse_label
    [](VW::polylabel& label, VW::reduction_features& /*red_features*/, VW::label_parser_reuse_mem& reuse_mem,
        const VW::named_labels* /*ldict*/, const std::vector<VW::string_view>& words,
        VW::io::logger& logger) { parse_cb_eval_label(label.cb_eval, reuse_mem, words, logger); },
    // cache_label
    [](const VW::polylabel& label, const VW::reduction_features& /*red_features*/, io_buf& cache,
        const std::string& upstream_name,
        bool text) { return VW::model_utils::write_model_field(cache, label.cb_eval, upstream_name, text); },
    // read_cached_label
    [](VW::polylabel& label, VW::reduction_features& /*red_features*/, io_buf& cache) {
      return VW::model_utils::read_model_field(cache, label.cb_eval);
    },
    // get_weight
    [](const VW::polylabel& /*label*/, const VW::reduction_features& /*red_features*/) { return 1.f; },
    // test_label
    [](const VW::polylabel& label) { return test_cb_eval_label(label.cb_eval); },
    // Label type
    VW::label_type_t::cb_eval};

namespace details
{
void print_cb_update(VW::workspace& all, bool is_test, const VW::example& ec, const VW::multi_ex* ec_seq,
    bool action_scores, const VW::cb_class* known_cost)
{
  if (all.sd->weighted_examples() >= all.sd->dump_interval && !all.quiet && !all.bfgs)
  {
    size_t num_features = ec.get_num_features();

    size_t pred = ec.pred.multiclass;
    if (ec_seq != nullptr)
    {
      num_features = 0;
      // TODO: code duplication csoaa.cc LabelDict::ec_is_example_header
      for (size_t i = 0; i < (*ec_seq).size(); i++)
      {
        if (VW::is_cb_example_header(*(*ec_seq)[i]))
        {
          num_features += (ec_seq->size() - 1) *
              ((*ec_seq)[i]->get_num_features() - (*ec_seq)[i]->feature_space[constant_namespace].size());
        }
        else
        {
          num_features += (*ec_seq)[i]->get_num_features();
        }
      }
    }
    std::string label_buf;
    if (is_test) { label_buf = "unknown"; }
    else
    {
      label_buf = known_cost_to_str(known_cost);
    }

    if (action_scores)
    {
      std::ostringstream pred_buf;
      if (!ec.pred.a_s.empty())
      {
        pred_buf << fmt::format("{}:{}", ec.pred.a_s[0].action,
            VW::fmt_float(ec.pred.a_s[0].score, VW::DEFAULT_FLOAT_FORMATTING_DECIMAL_PRECISION));
      }
      else
      {
        pred_buf << "no action";
      }
      all.sd->print_update(*all.trace_message, all.holdout_set_off, all.current_pass, label_buf, pred_buf.str(),
          num_features, all.progress_add, all.progress_arg);
    }
    else
    {
      all.sd->print_update(*all.trace_message, all.holdout_set_off, all.current_pass, label_buf,
          static_cast<uint32_t>(pred), num_features, all.progress_add, all.progress_arg);
    }
  }
}
}  // namespace details

namespace model_utils
{
size_t read_model_field(io_buf& io, VW::cb_class& cbc)
{
  size_t bytes = 0;
  bytes += read_model_field(io, cbc.cost);
  bytes += read_model_field(io, cbc.action);
  bytes += read_model_field(io, cbc.probability);
  bytes += read_model_field(io, cbc.partial_prediction);
  return bytes;
}

size_t write_model_field(io_buf& io, const VW::cb_class& cbc, const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  bytes += write_model_field(io, cbc.cost, upstream_name + "_cost", text);
  bytes += write_model_field(io, cbc.action, upstream_name + "_action", text);
  bytes += write_model_field(io, cbc.probability, upstream_name + "_probability", text);
  bytes += write_model_field(io, cbc.partial_prediction, upstream_name + "_partial_prediction", text);
  return bytes;
}

size_t read_model_field(io_buf& io, VW::cb_label& cb)
{
  size_t bytes = 0;
  bytes += read_model_field(io, cb.costs);
  bytes += read_model_field(io, cb.weight);
  return bytes;
}

size_t write_model_field(io_buf& io, const VW::cb_label& cb, const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  bytes += write_model_field(io, cb.costs, upstream_name + "_costs", text);
  bytes += write_model_field(io, cb.weight, upstream_name + "_weight", text);
  return bytes;
}

size_t read_model_field(io_buf& io, VW::cb_eval_label& cbe)
{
  size_t bytes = 0;
  bytes += read_model_field(io, cbe.action);
  bytes += read_model_field(io, cbe.event);
  return bytes;
}

size_t write_model_field(io_buf& io, const VW::cb_eval_label& cbe, const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  bytes += write_model_field(io, cbe.action, upstream_name + "_action", text);
  bytes += write_model_field(io, cbe.event, upstream_name + "_event", text);
  return bytes;
}
}  // namespace model_utils
}  // namespace VW
