// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/cb.h"

#include "vw/common/string_view.h"
#include "vw/common/text_utils.h"
#include "vw/common/vw_exception.h"
#include "vw/core/cb_graph_feedback_reduction_features.h"
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
std::pair<bool, VW::cb_class> VW::get_observed_cost_cb(const VW::cb_label& ld)
{
  for (const auto& cl : ld.costs)
  {
    if (cl.has_observed_cost()) { return std::make_pair(true, cl); }
  }

  // Default value for cb_class does not have an observed cost.
  return std::make_pair(false, VW::cb_class{});
}

// this is an experimental input for the experimental graph feedback reduction to be added
void parse_graph_feedback_matrix(const std::vector<VW::string_view>& words, size_t current_word_index,
    VW::label_parser_reuse_mem& reuse_mem, VW::reduction_features& red_features, VW::io::logger& logger)
{
  auto& graph_reduction_features = red_features.template get<VW::cb_graph_feedback::reduction_features>();

  // example: shared graph 0,1,5 2,4,5 0,2,5 |
  // here we only process the triplets

  for (size_t i = current_word_index; i < words.size(); i++)
  {
    VW::tokenize(',', words[i], reuse_mem.tokens);

    if (reuse_mem.tokens.empty() ||
        (reuse_mem.tokens.size() == 1 && reuse_mem.tokens[0].size() > 0 &&
            (reuse_mem.tokens[0][0] == ' ' || reuse_mem.tokens[0][0] == '|')))
    {
      // this check is necessary in order to check if an incomplete tripet was given or if we have reached the end of
      // the label
      return;
    }
    if (reuse_mem.tokens.size() != 3)
    {
      THROW("malformed example, graph expects triplets but was given an input with: " << reuse_mem.tokens.size()
                                                                                      << " elements");
    }

    auto row = VW::details::int_of_string(reuse_mem.tokens[0], logger);
    auto col = VW::details::int_of_string(reuse_mem.tokens[1], logger);
    auto val = VW::details::float_of_string(reuse_mem.tokens[2], logger);
    graph_reduction_features.push_triplet(row, col, val);
  }
}

static void parse_label_cb(VW::cb_label& ld, VW::reduction_features& red_features,
    VW::label_parser_reuse_mem& reuse_mem, const std::vector<VW::string_view>& words, VW::io::logger& logger)
{
  ld.weight = 1.0;

  for (size_t i = 0; i < words.size(); i++)
  {
    // Format is the following:
    // <action>:<cost>:<probability> | shared
    // for example "1:2:0.5"
    // action = 1, cost = 2, probability = 0.5
    const auto& word = words[i];
    VW::cb_class f;
    VW::tokenize(':', word, reuse_mem.tokens);

    if (reuse_mem.tokens.empty() || reuse_mem.tokens.size() > 3) { THROW("malformed cost specification: " << word); }

    f.partial_prediction = 0.;
    f.action =
        static_cast<uint32_t>(VW::details::hashstring(reuse_mem.tokens[0].data(), reuse_mem.tokens[0].length(), 0));
    f.cost = FLT_MAX;

    if (reuse_mem.tokens.size() > 1) { f.cost = VW::details::float_of_string(reuse_mem.tokens[1], logger); }

    if (std::isnan(f.cost)) THROW("error NaN cost (" << reuse_mem.tokens[1] << " for action: " << reuse_mem.tokens[0]);

    f.probability = .0;
    if (reuse_mem.tokens.size() > 2) { f.probability = VW::details::float_of_string(reuse_mem.tokens[2], logger); }

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
    if (reuse_mem.tokens[0] == VW::details::SHARED_TYPE)
    {
      if (reuse_mem.tokens.size() == 1) { f.probability = -1.f; }
      else { logger.err_warn("shared feature vectors should not have costs"); }
    }
    if (reuse_mem.tokens[0] == VW::details::GRAPH_FEEDBACK_TYPE)
    {
      if (ld.costs.size() < 1 || ld.costs.back().probability != -1.f)
      {
        THROW("malformed example, graph should only be present after shared")
      }
      // graph provided in shared example label
      // example: shared graph 0,1,5 2,4,5 0,2,5 |
      parse_graph_feedback_matrix(words, i + 1, reuse_mem, red_features, logger);
      return;
    }

    ld.costs.push_back(f);
  }
}

VW::label_parser VW::cb_label_parser_global = {
    // default_label
    [](VW::polylabel& label) { label.cb.reset_to_default(); },
    // parse_label
    [](VW::polylabel& label, VW::reduction_features& red_features, VW::label_parser_reuse_mem& reuse_mem,
        const VW::named_labels* /*ldict*/, const std::vector<VW::string_view>& words, VW::io::logger& logger)
    { ::parse_label_cb(label.cb, red_features, reuse_mem, words, logger); },
    // cache_label
    [](const VW::polylabel& label, const VW::reduction_features& /*red_features*/, VW::io_buf& cache,
        const std::string& upstream_name, bool text)
    { return VW::model_utils::write_model_field(cache, label.cb, upstream_name, text); },
    // read_cached_label
    [](VW::polylabel& label, VW::reduction_features& /*red_features*/, VW::io_buf& cache)
    { return VW::model_utils::read_model_field(cache, label.cb); },
    // get_weight
    [](const VW::polylabel& label, const VW::reduction_features& /*red_features*/) { return label.cb.weight; },
    // test_label
    [](const VW::polylabel& label) { return label.cb.is_test_label(); },
    // Label type
    VW::label_type_t::CB};

bool VW::ec_is_example_header_cb(VW::example const& ec)  // example headers just have "shared"
{
  const auto& costs = ec.l.cb.costs;
  if (costs.size() != 1) { return false; }
  if (costs[0].probability == -1.f) { return true; }
  return false;
}

static std::string known_cost_to_str(const VW::cb_class* known_cost)
{
  if (known_cost == nullptr) { return " known"; }

  std::stringstream label_string;
  label_string.precision(VW::details::DEFAULT_FLOAT_FORMATTING_DECIMAL_PRECISION);
  label_string << known_cost->action << ":" << known_cost->cost << ":" << known_cost->probability;
  return label_string.str();
}

bool VW::cb_label::is_test_label() const
{
  if (costs.empty()) { return true; }
  for (const auto& cost : costs)
  {
    const auto probability = cost.probability;
    if (FLT_MAX != cost.cost && probability > 0.) { return false; }
  }
  return true;
}
bool VW::cb_label::is_labeled() const { return !is_test_label(); }
void VW::cb_label::reset_to_default()
{
  costs.clear();
  weight = 1.f;
}

void ::VW::details::print_update_cb(VW::workspace& all, bool is_test, const VW::example& ec, const VW::multi_ex* ec_seq,
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
        if (VW::ec_is_example_header_cb(*(*ec_seq)[i]))
        {
          num_features += (ec_seq->size() - 1) *
              ((*ec_seq)[i]->get_num_features() - (*ec_seq)[i]->feature_space[VW::details::CONSTANT_NAMESPACE].size());
        }
        else { num_features += (*ec_seq)[i]->get_num_features(); }
      }
    }
    std::string label_buf;
    if (is_test) { label_buf = "unknown"; }
    else { label_buf = known_cost_to_str(known_cost); }

    if (action_scores)
    {
      std::ostringstream pred_buf;
      if (!ec.pred.a_s.empty())
      {
        pred_buf << fmt::format("{}:{}", ec.pred.a_s[0].action,
            VW::fmt_float(ec.pred.a_s[0].score, VW::details::DEFAULT_FLOAT_FORMATTING_DECIMAL_PRECISION));
      }
      else { pred_buf << "no action"; }
      all.sd->print_update(
          *all.trace_message, all.holdout_set_off, all.current_pass, label_buf, pred_buf.str(), num_features);
    }
    else
    {
      all.sd->print_update(*all.trace_message, all.holdout_set_off, all.current_pass, label_buf,
          static_cast<uint32_t>(pred), num_features);
    }
  }
}

namespace
{
float weight_cb_eval(const VW::cb_eval_label& ld) { return ld.event.weight; }

void default_label_cb_eval(VW::cb_eval_label& ld)
{
  ld.event.reset_to_default();
  ld.action = 0;
}

bool test_label_cb_eval(const VW::cb_eval_label& ld) { return ld.event.is_test_label(); }

void parse_label_cb_eval(VW::cb_eval_label& ld, VW::reduction_features& red_features,
    VW::label_parser_reuse_mem& reuse_mem, const std::vector<VW::string_view>& words, VW::io::logger& logger)
{
  if (words.size() < 2) THROW("Evaluation can not happen without an action and an exploration");

  ld.action = static_cast<uint32_t>(VW::details::hashstring(words[0].data(), words[0].length(), 0));

  // TODO - make this a span and there is no allocation
  const auto rest_of_tokens = std::vector<VW::string_view>(words.begin() + 1, words.end());
  ::parse_label_cb(ld.event, red_features, reuse_mem, rest_of_tokens, logger);
}
}  // namespace

VW::label_parser VW::cb_eval_label_parser_global = {
    // default_label
    [](VW::polylabel& label) { default_label_cb_eval(label.cb_eval); },
    // parse_label
    [](VW::polylabel& label, VW::reduction_features& red_features, VW::label_parser_reuse_mem& reuse_mem,
        const VW::named_labels* /*ldict*/, const std::vector<VW::string_view>& words, VW::io::logger& logger)
    { parse_label_cb_eval(label.cb_eval, red_features, reuse_mem, words, logger); },
    // cache_label
    [](const VW::polylabel& label, const VW::reduction_features& /*red_features*/, VW::io_buf& cache,
        const std::string& upstream_name, bool text)
    { return VW::model_utils::write_model_field(cache, label.cb_eval, upstream_name, text); },
    // read_cached_label
    [](VW::polylabel& label, VW::reduction_features& /*red_features*/, VW::io_buf& cache)
    { return VW::model_utils::read_model_field(cache, label.cb_eval); },
    // get_weight
    [](const VW::polylabel& label, const VW::reduction_features& /*red_features*/)
    { return weight_cb_eval(label.cb_eval); },
    // test_label
    [](const VW::polylabel& label) { return test_label_cb_eval(label.cb_eval); },
    // Label type
    VW::label_type_t::CB_EVAL};

namespace VW
{
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
