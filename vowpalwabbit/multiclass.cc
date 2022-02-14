// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <cstring>
#include <climits>
#include "global_data.h"
#include "vw.h"
#include "vw_exception.h"
#include "vw_string_view.h"
#include "example.h"
#include "parse_primitives.h"
#include "shared_data.h"
#include "model_utils.h"

namespace MULTICLASS
{
label_t::label_t() { reset_to_default(); }

label_t::label_t(uint32_t label, float weight) : label(label), weight(weight) {}

void label_t::reset_to_default()
{
  label = std::numeric_limits<uint32_t>::max();
  weight = 1.f;
}

void default_label(label_t& ld) { ld.reset_to_default(); }

float weight(const label_t& ld) { return (ld.weight > 0) ? ld.weight : 0.f; }
bool test_label(const label_t& ld) { return ld.label == static_cast<uint32_t>(-1); }

void parse_label(
    label_t& ld, const VW::named_labels* ldict, const std::vector<VW::string_view>& words, VW::io::logger& logger)
{
  switch (words.size())
  {
    case 0:
      break;
    case 1:
      if (ldict) { ld.label = ldict->get(words[0], logger); }
      else
      {
        char* char_after_int = nullptr;
        ld.label = int_of_string(words[0], char_after_int, logger);
        if (char_after_int != nullptr && *char_after_int != ' ' && *char_after_int != '\0')
        { THROW("Malformed example: label has trailing character(s): " << *char_after_int); }
      }
      ld.weight = 1.0;
      break;
    case 2:
      if (ldict) { ld.label = ldict->get(words[0], logger); }
      else
      {
        char* char_after_int = nullptr;
        ld.label = int_of_string(words[0], char_after_int, logger);
        if (char_after_int != nullptr && *char_after_int != ' ' && *char_after_int != '\0')
        { THROW("Malformed example: label has trailing character(s): " << *char_after_int); }
      }
      ld.weight = float_of_string(words[1], logger);
      break;
    default:
      THROW("Malformed example, words.size() = " << words.size());
  }
}

label_parser mc_label = {
    // default_label
    [](polylabel& label) { default_label(label.multi); },
    // parse_label
    [](polylabel& label, reduction_features& /* red_features */, VW::label_parser_reuse_mem& /* reuse_mem */,
        const VW::named_labels* ldict, const std::vector<VW::string_view>& words,
        VW::io::logger& logger) { parse_label(label.multi, ldict, words, logger); },
    // cache_label
    [](const polylabel& label, const reduction_features& /* red_features */, io_buf& cache,
        const std::string& upstream_name,
        bool text) { return VW::model_utils::write_model_field(cache, label.multi, upstream_name, text); },
    // read_cached_label
    [](polylabel& label, reduction_features& /* red_features */, io_buf& cache) {
      return VW::model_utils::read_model_field(cache, label.multi);
    },
    // get_weight
    [](const polylabel& label, const reduction_features& /* red_features */) { return weight(label.multi); },
    // test_label
    [](const polylabel& label) { return test_label(label.multi); },
    // label type
    VW::label_type_t::multiclass};

void print_label_pred(VW::workspace& all, example& ec, uint32_t prediction)
{
  VW::string_view sv_label = all.sd->ldict->get(ec.l.multi.label);
  VW::string_view sv_pred = all.sd->ldict->get(prediction);
  all.sd->print_update(*all.trace_message, all.holdout_set_off, all.current_pass,
      sv_label.empty() ? "unknown" : std::string{sv_label}, sv_pred.empty() ? "unknown" : std::string{sv_pred},
      ec.get_num_features(), all.progress_add, all.progress_arg);
}

void print_probability(VW::workspace& all, example& ec, uint32_t prediction)
{
  if (prediction == 0) { prediction = static_cast<uint32_t>(ec.pred.scalars.size()); }
  std::stringstream pred_ss;
  pred_ss << prediction << "(" << std::setw(2) << std::setprecision(0) << std::fixed
          << 100 * ec.pred.scalars[prediction - 1] << "%)";

  std::stringstream label_ss;
  label_ss << ec.l.multi.label;

  all.sd->print_update(*all.trace_message, all.holdout_set_off, all.current_pass, label_ss.str(), pred_ss.str(),
      ec.get_num_features(), all.progress_add, all.progress_arg);
}

void print_score(VW::workspace& all, example& ec, uint32_t prediction)
{
  std::stringstream pred_ss;
  pred_ss << prediction;

  std::stringstream label_ss;
  label_ss << ec.l.multi.label;

  all.sd->print_update(*all.trace_message, all.holdout_set_off, all.current_pass, label_ss.str(), pred_ss.str(),
      ec.get_num_features(), all.progress_add, all.progress_arg);
}

void direct_print_update(VW::workspace& all, example& ec, uint32_t prediction)
{
  all.sd->print_update(*all.trace_message, all.holdout_set_off, all.current_pass, ec.l.multi.label, prediction,
      ec.get_num_features(), all.progress_add, all.progress_arg);
}

template <void (*T)(VW::workspace&, example&, uint32_t)>
void print_update(VW::workspace& all, example& ec, uint32_t prediction)
{
  if (all.sd->weighted_examples() >= all.sd->dump_interval && !all.quiet && !all.bfgs)
  {
    if (!all.sd->ldict)
      T(all, ec, prediction);
    else
      print_label_pred(all, ec, ec.pred.multiclass);
  }
}

void print_update_with_probability(VW::workspace& all, example& ec, uint32_t pred)
{
  print_update<print_probability>(all, ec, pred);
}
void print_update_with_score(VW::workspace& all, example& ec, uint32_t pred)
{
  print_update<print_score>(all, ec, pred);
}

void finish_example(VW::workspace& all, example& ec, bool update_loss)
{
  float loss = 0;
  if (ec.l.multi.label != ec.pred.multiclass && ec.l.multi.label != static_cast<uint32_t>(-1)) loss = ec.weight;

  all.sd->update(ec.test_only, update_loss && (ec.l.multi.label != static_cast<uint32_t>(-1)), loss, ec.weight,
      ec.get_num_features());

  for (auto& sink : all.final_prediction_sink)
    if (!all.sd->ldict)
      all.print_by_ref(sink.get(), static_cast<float>(ec.pred.multiclass), 0, ec.tag, all.logger);
    else
    {
      VW::string_view sv_pred = all.sd->ldict->get(ec.pred.multiclass);
      all.print_text_by_ref(sink.get(), std::string{sv_pred}, ec.tag, all.logger);
    }

  MULTICLASS::print_update<direct_print_update>(all, ec, ec.pred.multiclass);
  VW::finish_example(all, ec);
}
}  // namespace MULTICLASS

namespace VW
{
namespace model_utils
{
size_t read_model_field(io_buf& io, MULTICLASS::label_t& multi)
{
  size_t bytes = 0;
  bytes += read_model_field(io, multi.label);
  bytes += read_model_field(io, multi.weight);
  return bytes;
}

size_t write_model_field(io_buf& io, const MULTICLASS::label_t& multi, const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  bytes += write_model_field(io, multi.label, upstream_name + "_label", text);
  bytes += write_model_field(io, multi.weight, upstream_name + "_weight", text);
  return bytes;
}
}  // namespace model_utils
}  // namespace VW
