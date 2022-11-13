// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/common/string_view.h"
#include "vw/common/text_utils.h"
#include "vw/common/vw_exception.h"
#include "vw/core/example.h"
#include "vw/core/global_data.h"
#include "vw/core/model_utils.h"
#include "vw/core/named_labels.h"
#include "vw/core/parse_primitives.h"
#include "vw/core/shared_data.h"
#include "vw/core/vw.h"
#include "vw/io/logger.h"

#include <cfloat>
#include <cmath>

void VW::default_cs_label(cs_label& ld) { ld.costs.clear(); }

bool VW::is_cs_example_header(const VW::example& ec)  // example headers look like "shared"
{
  const auto& costs = ec.l.cs.costs;
  if (costs.size() != 1) { return false; }
  if (costs[0].class_index != 0) { return false; }
  if (costs[0].x != -FLT_MAX) { return false; }
  return true;
}

namespace
{
void name_value(VW::string_view s, std::vector<VW::string_view>& name, float& v, VW::io::logger& logger)
{
  VW::tokenize(':', s, name);

  switch (name.size())
  {
    case 0:
    case 1:
      v = 1.;
      break;
    case 2:
      v = float_of_string(name[1], logger);
      if (std::isnan(v)) THROW("error NaN value for: " << name[0]);
      break;
    default:
      logger.err_error("example with a weird name. What is '{}'?", s);
  }
}

float cs_weight(const VW::cs_label&) { return 1.; }

bool test_label_internal(const VW::cs_label& ld)
{
  if (ld.costs.empty()) { return true; }
  for (auto cost : ld.costs)
  {
    if (FLT_MAX != cost.x) { return false; }
  }
  return true;
}

bool test_label(const VW::cs_label& ld) { return test_label_internal(ld); }

void parse_label(VW::cs_label& ld, VW::label_parser_reuse_mem& reuse_mem, const VW::named_labels* ldict,
    const std::vector<VW::string_view>& words, VW::io::logger& logger)
{
  ld.costs.clear();

  // handle shared and label first
  if (words.size() == 1)
  {
    float fx;
    name_value(words[0], reuse_mem.tokens, fx, logger);
    bool eq_shared = reuse_mem.tokens[0] == "***shared***";
    bool eq_label = reuse_mem.tokens[0] == "***label***";
    if (ldict == nullptr)
    {
      eq_shared |= reuse_mem.tokens[0] == "shared";
      eq_label |= reuse_mem.tokens[0] == "label";
    }
    if (eq_shared || eq_label)
    {
      if (eq_shared)
      {
        if (reuse_mem.tokens.size() != 1)
        { logger.err_error("shared feature vectors should not have costs on: {}", words[0]); }
        else
        {
          VW::cs_class f = {-FLT_MAX, 0, 0., 0.};
          ld.costs.push_back(f);
        }
      }
      if (eq_label)
      {
        if (reuse_mem.tokens.size() != 2)
        { logger.err_error("label feature vectors should have exactly one cost on: {}", words[0]); }
        else
        {
          VW::cs_class f = {float_of_string(reuse_mem.tokens[1], logger), 0, 0., 0.};
          ld.costs.push_back(f);
        }
      }
      return;
    }
  }

  // otherwise this is a "real" example
  for (auto word : words)
  {
    VW::cs_class f = {0., 0, 0., 0.};
    name_value(word, reuse_mem.tokens, f.x, logger);

    if (reuse_mem.tokens.empty()) THROW(" invalid cost: specification -- no names on: " << word);

    if (reuse_mem.tokens.size() == 1 || reuse_mem.tokens.size() == 2 || reuse_mem.tokens.size() == 3)
    {
      f.class_index = ldict
          ? ldict->get(reuse_mem.tokens[0], logger)
          : static_cast<uint32_t>(hashstring(reuse_mem.tokens[0].data(), reuse_mem.tokens[0].length(), 0));
      if (reuse_mem.tokens.size() == 1 && f.x >= 0)
      {  // test examples are specified just by un-valued class #s
        f.x = FLT_MAX;
      }
    }
    else
      THROW("malformed cost specification on '" << (reuse_mem.tokens[0]) << "'");

    ld.costs.push_back(f);
  }
}
}  // namespace

void VW::details::print_cs_update(VW::workspace& all, bool is_test, const VW::example& ec, const VW::multi_ex* ec_seq,
    bool action_scores, uint32_t prediction)
{
  if (all.sd->weighted_examples() >= all.sd->dump_interval && !all.quiet && !all.bfgs)
  {
    size_t num_current_features = ec.get_num_features();
    // for csoaa_ldf we want features from the whole (multiline example),
    // not only from one line (the first one) represented by ec
    if (ec_seq != nullptr)
    {
      num_current_features = 0;
      for (const auto& ecc : *ec_seq)
      {
        if (VW::is_cs_example_header(*ecc))
        {
          num_current_features += (ec_seq->size() - 1) *
              (ecc->get_num_features() - ecc->feature_space[VW::details::CONSTANT_NAMESPACE].size());
        }
        else
        {
          num_current_features += ecc->get_num_features();
        }
      }
    }

    std::string label_buf;
    if (is_test) { label_buf = "unknown"; }
    else
    {
      label_buf = "known";
    }

    if (action_scores || all.sd->ldict)
    {
      std::ostringstream pred_buf;
      if (all.sd->ldict)
      {
        if (action_scores) { pred_buf << all.sd->ldict->get(ec.pred.a_s[0].action); }
        else
        {
          pred_buf << all.sd->ldict->get(prediction);
        }
      }
      else
      {
        pred_buf << ec.pred.a_s[0].action;
      }
      if (action_scores) { pred_buf << "....."; }
      all.sd->print_update(*all.trace_message, all.holdout_set_off, all.current_pass, label_buf, pred_buf.str(),
          num_current_features, all.progress_add, all.progress_arg);
      ;
    }
    else
    {
      all.sd->print_update(*all.trace_message, all.holdout_set_off, all.current_pass, label_buf, prediction,
          num_current_features, all.progress_add, all.progress_arg);
    }
  }
}

void VW::details::output_cs_example(
    VW::workspace& all, const VW::example& ec, const VW::cs_label& label, uint32_t multiclass_prediction)
{
  float loss = 0.;
  if (!test_label(label))
  {
    // need to compute exact loss
    auto pred = static_cast<size_t>(multiclass_prediction);

    float chosen_loss = FLT_MAX;
    float min = FLT_MAX;
    for (const auto& cl : label.costs)
    {
      if (cl.class_index == pred) { chosen_loss = cl.x; }
      if (cl.x < min) { min = cl.x; }
    }
    if (chosen_loss == FLT_MAX)
    { all.logger.err_warn("csoaa predicted an invalid class. Are all multi-class labels in the {{1..k}} range?"); }

    loss = (chosen_loss - min) * ec.weight;
    // TODO(alberto): add option somewhere to allow using absolute loss instead?
    // loss = chosen_loss;
  }

  all.sd->update(ec.test_only, !test_label(label), loss, ec.weight, ec.get_num_features());

  for (auto& sink : all.final_prediction_sink)
  {
    if (!all.sd->ldict)
    { all.print_by_ref(sink.get(), static_cast<float>(multiclass_prediction), 0, ec.tag, all.logger); }
    else
    {
      VW::string_view sv_pred = all.sd->ldict->get(multiclass_prediction);
      all.print_text_by_ref(sink.get(), std::string{sv_pred}, ec.tag, all.logger);
    }
  }

  if (all.raw_prediction != nullptr)
  {
    std::stringstream output_string_stream;
    for (unsigned int i = 0; i < label.costs.size(); i++)
    {
      VW::cs_class cl = label.costs[i];
      if (i > 0) { output_string_stream << ' '; }
      output_string_stream << cl.class_index << ':' << cl.partial_prediction;
    }
    all.print_text_by_ref(all.raw_prediction.get(), output_string_stream.str(), ec.tag, all.logger);
  }

  print_cs_update(all, test_label(label), ec, nullptr, false, multiclass_prediction);
}

void VW::details::output_cs_example(VW::workspace& all, const VW::example& ec)
{
  output_cs_example(all, ec, ec.l.cs, ec.pred.multiclass);
}

void VW::details::finish_cs_example(VW::workspace& all, VW::example& ec)
{
  output_cs_example(all, ec, ec.l.cs, ec.pred.multiclass);
  VW::finish_example(all, ec);
}

namespace VW
{
VW::label_parser cs_label_parser_global = {
    // default_label
    [](VW::polylabel& label) { default_cs_label(label.cs); },
    // parse_label
    [](VW::polylabel& label, VW::reduction_features& /* red_features */, VW::label_parser_reuse_mem& reuse_mem,
        const VW::named_labels* ldict, const std::vector<VW::string_view>& words,
        VW::io::logger& logger) { parse_label(label.cs, reuse_mem, ldict, words, logger); },
    // cache_label
    [](const VW::polylabel& label, const VW::reduction_features& /* red_features */, io_buf& cache,
        const std::string& upstream_name,
        bool text) { return VW::model_utils::write_model_field(cache, label.cs, upstream_name, text); },
    // read_cached_label
    [](VW::polylabel& label, VW::reduction_features& /* red_features */, io_buf& cache) {
      return VW::model_utils::read_model_field(cache, label.cs);
    },
    // get_weight
    [](const VW::polylabel& label, const VW::reduction_features& /* red_features */) { return cs_weight(label.cs); },
    // test_label
    [](const VW::polylabel& label) { return test_label(label.cs); },
    // label type
    VW::label_type_t::CS};

namespace model_utils
{
size_t read_model_field(io_buf& io, cs_class& wc)
{
  size_t bytes = 0;
  bytes += read_model_field(io, wc.x);
  bytes += read_model_field(io, wc.class_index);
  bytes += read_model_field(io, wc.partial_prediction);
  bytes += read_model_field(io, wc.wap_value);
  return bytes;
}

size_t write_model_field(io_buf& io, const cs_class& wc, const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  bytes += write_model_field(io, wc.x, upstream_name + "_x", text);
  bytes += write_model_field(io, wc.class_index, upstream_name + "_class_index", text);
  bytes += write_model_field(io, wc.partial_prediction, upstream_name + "_partial_prediction", text);
  bytes += write_model_field(io, wc.wap_value, upstream_name + "_wap_value", text);
  return bytes;
}

size_t read_model_field(io_buf& io, VW::cs_label& cs)
{
  size_t bytes = 0;
  bytes += read_model_field(io, cs.costs);
  return bytes;
}

size_t write_model_field(io_buf& io, const VW::cs_label& cs, const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  bytes += write_model_field(io, cs.costs, upstream_name + "_costs", text);
  return bytes;
}
}  // namespace model_utils
}  // namespace VW
