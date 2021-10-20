// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <cfloat>
#include "example.h"
#include "parse_primitives.h"
#include "vw.h"
#include "vw_exception.h"
#include "cb_label_parser.h"
#include "cb_continuous_label.h"
#include "debug_print.h"

#include "io/logger.h"

using namespace LEARNER;

namespace logger = VW::io::logger;

namespace CB
{
template <>
size_t read_cached_label_additional_fields<VW::cb_continuous::continuous_label>(
    VW::cb_continuous::continuous_label&, io_buf&)
{
  return 0;
}

template <>
void cache_label_additional_fields<VW::cb_continuous::continuous_label>(
    const VW::cb_continuous::continuous_label&, io_buf&)
{
}

template <>
void default_label_additional_fields<VW::cb_continuous::continuous_label>(VW::cb_continuous::continuous_label&)
{
}

}  // namespace CB

void parse_pdf(const std::vector<VW::string_view>& words, size_t words_index, VW::label_parser_reuse_mem& reuse_mem,
    reduction_features& red_features)
{
  auto& cats_reduction_features = red_features.template get<VW::continuous_actions::reduction_features>();
  for (size_t i = words_index; i < words.size(); i++)
  {
    if (words[i] == CHOSEN_ACTION) { break; /* no more pdf to parse*/ }
    tokenize(':', words[i], reuse_mem.tokens);
    if (reuse_mem.tokens.empty() || reuse_mem.tokens.size() < 3) { continue; }
    VW::continuous_actions::pdf_segment seg;
    seg.left = float_of_string(reuse_mem.tokens[0]);
    seg.right = float_of_string(reuse_mem.tokens[1]);
    seg.pdf_value = float_of_string(reuse_mem.tokens[2]);
    cats_reduction_features.pdf.push_back(seg);
  }
  if (!VW::continuous_actions::is_valid_pdf(cats_reduction_features.pdf)) { cats_reduction_features.pdf.clear(); }
}

void parse_chosen_action(const std::vector<VW::string_view>& words, size_t words_index,
    VW::label_parser_reuse_mem& reuse_mem, reduction_features& red_features)
{
  auto& cats_reduction_features = red_features.template get<VW::continuous_actions::reduction_features>();
  for (size_t i = words_index; i < words.size(); i++)
  {
    tokenize(':', words[i], reuse_mem.tokens);
    if (reuse_mem.tokens.empty() || reuse_mem.tokens.size() < 1) { continue; }
    cats_reduction_features.chosen_action = float_of_string(reuse_mem.tokens[0]);
    break;  // there can only be one chosen action
  }
}

namespace VW
{
namespace cb_continuous
{
////////////////////////////////////////////////////
// Begin: parse a,c,p label format
void parse_label(continuous_label& ld, reduction_features& red_features, VW::label_parser_reuse_mem& reuse_mem,
    const std::vector<VW::string_view>& words)
{
  ld.costs.clear();

  if (words.empty()) { return; }

  if (!(words[0] == CA_LABEL)) { THROW("Continuous actions labels require the first word to be ca"); }

  for (size_t i = 1; i < words.size(); i++)
  {
    if (words[i] == PDF) { parse_pdf(words, i + 1, reuse_mem, red_features); }
    else if (words[i] == CHOSEN_ACTION)
    {
      parse_chosen_action(words, i + 1, reuse_mem, red_features);
    }
    else if (words[i - 1] == CA_LABEL)
    {
      continuous_label_elm f{0.f, FLT_MAX, 0.f};
      tokenize(':', words[i], reuse_mem.tokens);

      if (reuse_mem.tokens.empty() || reuse_mem.tokens.size() > 4)
        THROW("malformed cost specification: "
            << "reuse_mem.tokens");

      f.action = float_of_string(reuse_mem.tokens[0]);

      if (reuse_mem.tokens.size() > 1) f.cost = float_of_string(reuse_mem.tokens[1]);

      if (std::isnan(f.cost))
        THROW("error NaN cost (" << reuse_mem.tokens[1] << " for action: " << reuse_mem.tokens[0]);

      f.pdf_value = .0;
      if (reuse_mem.tokens.size() > 2) f.pdf_value = float_of_string(reuse_mem.tokens[2]);

      if (std::isnan(f.pdf_value))
        THROW("error NaN pdf_value (" << reuse_mem.tokens[2] << " for action: " << reuse_mem.tokens[0]);

      if (f.pdf_value < 0.0)
      {
        logger::errlog_warn("invalid pdf_value < 0 specified for an action, resetting to 0.");
        f.pdf_value = .0;
      }

      ld.costs.push_back(f);
    }
  }
}

label_parser the_label_parser = {
    // default_label
    [](polylabel& label) { CB::default_label<continuous_label>(label.cb_cont); },
    // parse_label
    [](polylabel& label, reduction_features& red_features, VW::label_parser_reuse_mem& reuse_mem,
        const VW::named_labels* /*ldict*/,
        const std::vector<VW::string_view>& words) { parse_label(label.cb_cont, red_features, reuse_mem, words); },
    // cache_label
    [](const polylabel& label, const reduction_features& /*red_features*/, io_buf& cache) {
      CB::cache_label<continuous_label, continuous_label_elm>(label.cb_cont, cache);
    },
    // read_cached_label
    [](polylabel& label, reduction_features& /*red_features*/, const VW::named_labels*, io_buf& cache) {
      return CB::read_cached_label<continuous_label, continuous_label_elm>(label.cb_cont, cache);
    },
    // get_weight
    // CB::weight just returns 1.f? This seems like it could be a bug...
    [](const polylabel& /*label*/, const reduction_features& /*red_features*/) { return 1.f; },
    // test_label
    [](const polylabel& label) { return CB::is_test_label<continuous_label, continuous_label_elm>(label.cb_cont); },
    // label type
    VW::label_type_t::continuous};

// End: parse a,c,p label format
////////////////////////////////////////////////////

std::string to_string(const continuous_label_elm& elm)
{
  std::stringstream strm;
  strm << "{" << elm.action << "," << elm.cost << "," << elm.pdf_value << "}";
  return strm.str();
}

std::string to_string(const continuous_label& lbl)
{
  std::stringstream strstream;
  strstream << "[l.cb_cont={";
  for (const auto cost : lbl.costs) strstream << to_string(cost);
  strstream << "}]";
  return strstream.str();
}
}  // namespace cb_continuous
}  // namespace VW
