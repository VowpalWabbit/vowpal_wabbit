// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/multiclass.h"

#include "vw/common/string_view.h"
#include "vw/common/vw_exception.h"
#include "vw/core/example.h"
#include "vw/core/global_data.h"
#include "vw/core/model_utils.h"
#include "vw/core/named_labels.h"
#include "vw/core/parse_primitives.h"
#include "vw/core/shared_data.h"
#include "vw/core/vw.h"

#include <climits>
#include <cstring>
#include <iomanip>

VW::multiclass_label::multiclass_label() { reset_to_default(); }

VW::multiclass_label::multiclass_label(uint32_t label, float weight) : label(label), weight(weight) {}

void VW::multiclass_label::reset_to_default()
{
  label = std::numeric_limits<uint32_t>::max();
  weight = 1.f;
}

bool VW::test_multiclass_label(const VW::multiclass_label& ld) { return !ld.is_labeled(); }

namespace
{
void default_label(VW::multiclass_label& ld) { ld.reset_to_default(); }
float multiclass_label_weight(const VW::multiclass_label& ld) { return (ld.weight > 0) ? ld.weight : 0.f; }
void parse_multiclass_label(VW::multiclass_label& ld, const VW::named_labels* ldict,
    const std::vector<VW::string_view>& words, VW::io::logger& logger)
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
        ld.label = VW::details::int_of_string(words[0], char_after_int, logger);
        if (char_after_int != nullptr && *char_after_int != ' ' && *char_after_int != '\0')
        {
          THROW("Malformed example: label has trailing character(s): " << *char_after_int);
        }
      }
      ld.weight = 1.0;
      break;
    case 2:
      if (ldict) { ld.label = ldict->get(words[0], logger); }
      else
      {
        char* char_after_int = nullptr;
        ld.label = VW::details::int_of_string(words[0], char_after_int, logger);
        if (char_after_int != nullptr && *char_after_int != ' ' && *char_after_int != '\0')
        {
          THROW("Malformed example: label has trailing character(s): " << *char_after_int);
        }
      }
      ld.weight = VW::details::float_of_string(words[1], logger);
      break;
    default:
      THROW("Malformed example, words.size() = " << words.size());
  }
}
}  // namespace

namespace VW
{
VW::label_parser multiclass_label_parser_global = {
    // default_label
    [](VW::polylabel& label) { default_label(label.multi); },
    // parse_label
    [](VW::polylabel& label, VW::reduction_features& /* red_features */, VW::label_parser_reuse_mem& /* reuse_mem */,
        const VW::named_labels* ldict, const std::vector<VW::string_view>& words, VW::io::logger& logger)
    { parse_multiclass_label(label.multi, ldict, words, logger); },
    // cache_label
    [](const VW::polylabel& label, const VW::reduction_features& /* red_features */, io_buf& cache,
        const std::string& upstream_name, bool text)
    { return VW::model_utils::write_model_field(cache, label.multi, upstream_name, text); },
    // read_cached_label
    [](VW::polylabel& label, VW::reduction_features& /* red_features */, io_buf& cache)
    { return VW::model_utils::read_model_field(cache, label.multi); },
    // get_weight
    [](const VW::polylabel& label, const VW::reduction_features& /* red_features */)
    { return multiclass_label_weight(label.multi); },
    // test_label
    [](const VW::polylabel& label) { return test_multiclass_label(label.multi); },
    // label type
    VW::label_type_t::MULTICLASS};
}

namespace
{
void print_label_pred(VW::workspace& all, const VW::example& ec, uint32_t prediction)
{
  VW::string_view sv_label = all.sd->ldict->get(ec.l.multi.label);
  VW::string_view sv_pred = all.sd->ldict->get(prediction);
  all.sd->print_update(*all.trace_message, all.holdout_set_off, all.current_pass,
      sv_label.empty() ? "unknown" : std::string{sv_label}, sv_pred.empty() ? "unknown" : std::string{sv_pred},
      ec.get_num_features());
}

void print_probability(VW::workspace& all, const VW::example& ec, uint32_t prediction)
{
  std::stringstream pred_ss;
  uint32_t pred_ind = (all.indexing == 0) ? prediction : prediction - 1;
  pred_ss << prediction << "(" << std::setw(VW::details::DEFAULT_FLOAT_FORMATTING_DECIMAL_PRECISION)
          << std::setprecision(0) << std::fixed << 100 * ec.pred.scalars[pred_ind] << "%)";

  std::stringstream label_ss;
  label_ss << ec.l.multi.label;

  all.sd->print_update(
      *all.trace_message, all.holdout_set_off, all.current_pass, label_ss.str(), pred_ss.str(), ec.get_num_features());
}

void print_score(VW::workspace& all, const VW::example& ec, uint32_t prediction)
{
  std::stringstream pred_ss;
  pred_ss << prediction;

  std::stringstream label_ss;
  label_ss << ec.l.multi.label;

  all.sd->print_update(
      *all.trace_message, all.holdout_set_off, all.current_pass, label_ss.str(), pred_ss.str(), ec.get_num_features());
}

void direct_print_update(VW::workspace& all, const VW::example& ec, uint32_t prediction)
{
  all.sd->print_update(
      *all.trace_message, all.holdout_set_off, all.current_pass, ec.l.multi.label, prediction, ec.get_num_features());
}

template <void (*T)(VW::workspace&, const VW::example&, uint32_t)>
void print_update(VW::workspace& all, const VW::example& ec, uint32_t prediction)
{
  if (all.sd->weighted_examples() >= all.sd->dump_interval && !all.quiet && !all.bfgs)
  {
    if (!all.sd->ldict) { T(all, ec, prediction); }
    else { print_label_pred(all, ec, ec.pred.multiclass); }
  }
}
}  // namespace

void VW::details::print_multiclass_update_with_probability(VW::workspace& all, const VW::example& ec, uint32_t pred)
{
  ::print_update<print_probability>(all, ec, pred);
}
void VW::details::print_multiclass_update_with_score(VW::workspace& all, const VW::example& ec, uint32_t pred)
{
  ::print_update<print_score>(all, ec, pred);
}

void VW::details::finish_multiclass_example(VW::workspace& all, VW::example& ec, bool update_loss)
{
  float loss = 0;
  if (ec.l.multi.label != ec.pred.multiclass && ec.l.multi.is_labeled()) { loss = ec.weight; }

  all.sd->update(ec.test_only, update_loss && (ec.l.multi.is_labeled()), loss, ec.weight, ec.get_num_features());

  for (auto& sink : all.final_prediction_sink)
  {
    if (!all.sd->ldict) { all.print_by_ref(sink.get(), static_cast<float>(ec.pred.multiclass), 0, ec.tag, all.logger); }
    else
    {
      VW::string_view sv_pred = all.sd->ldict->get(ec.pred.multiclass);
      all.print_text_by_ref(sink.get(), std::string{sv_pred}, ec.tag, all.logger);
    }
  }

  ::print_update<direct_print_update>(all, ec, ec.pred.multiclass);
  VW::finish_example(all, ec);
}

void VW::details::update_stats_multiclass_label(
    const VW::workspace& /* all */, shared_data& sd, const VW::example& ec, VW::io::logger& /* logger */)
{
  float loss = 0;
  if (ec.l.multi.label != ec.pred.multiclass && ec.l.multi.is_labeled()) { loss = ec.weight; }

  sd.update(ec.test_only, ec.l.multi.is_labeled(), loss, ec.weight, ec.get_num_features());
}
void VW::details::output_example_prediction_multiclass_label(
    VW::workspace& all, const VW::example& ec, VW::io::logger& /* logger */)
{
  for (auto& sink : all.final_prediction_sink)
  {
    if (!all.sd->ldict) { all.print_by_ref(sink.get(), static_cast<float>(ec.pred.multiclass), 0, ec.tag, all.logger); }
    else
    {
      VW::string_view sv_pred = all.sd->ldict->get(ec.pred.multiclass);
      all.print_text_by_ref(sink.get(), std::string{sv_pred}, ec.tag, all.logger);
    }
  }
}

void VW::details::print_update_multiclass_label(
    VW::workspace& all, shared_data& /* sd */, const VW::example& ec, VW::io::logger& /* logger */)
{
  ::print_update<direct_print_update>(all, ec, ec.pred.multiclass);
}

size_t VW::model_utils::read_model_field(io_buf& io, VW::multiclass_label& multi)
{
  size_t bytes = 0;
  bytes += read_model_field(io, multi.label);
  bytes += read_model_field(io, multi.weight);
  return bytes;
}

size_t VW::model_utils::write_model_field(
    io_buf& io, const VW::multiclass_label& multi, const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  bytes += write_model_field(io, multi.label, upstream_name + "_label", text);
  bytes += write_model_field(io, multi.weight, upstream_name + "_weight", text);
  return bytes;
}
