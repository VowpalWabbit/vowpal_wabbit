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
void cache_label_additional_fields<VW::cb_continuous::continuous_label>(VW::cb_continuous::continuous_label&, io_buf&)
{
}

template <>
void default_label_additional_fields<VW::cb_continuous::continuous_label>(VW::cb_continuous::continuous_label&)
{
}

}  // namespace CB

void parse_pdf(
    const std::vector<VW::string_view>& words, size_t words_index, parser* p, reduction_features& red_features)
{
  auto& cats_reduction_features = red_features.template get<VW::continuous_actions::reduction_features>();
  for (size_t i = words_index; i < words.size(); i++)
  {
    if (words[i] == CHOSEN_ACTION) { break; /* no more pdf to parse*/ }
    tokenize(':', words[i], p->parse_name);
    if (p->parse_name.empty() || p->parse_name.size() < 3) { continue; }
    VW::continuous_actions::pdf_segment seg;
    seg.left = float_of_string(p->parse_name[0]);
    seg.right = float_of_string(p->parse_name[1]);
    seg.pdf_value = float_of_string(p->parse_name[2]);
    cats_reduction_features.pdf.push_back(seg);
  }
  if (!VW::continuous_actions::is_valid_pdf(cats_reduction_features.pdf)) { cats_reduction_features.pdf.clear(); }
}

void parse_chosen_action(
    const std::vector<VW::string_view>& words, size_t words_index, parser* p, reduction_features& red_features)
{
  auto& cats_reduction_features = red_features.template get<VW::continuous_actions::reduction_features>();
  for (size_t i = words_index; i < words.size(); i++)
  {
    tokenize(':', words[i], p->parse_name);
    if (p->parse_name.empty() || p->parse_name.size() < 1) { continue; }
    cats_reduction_features.chosen_action = float_of_string(p->parse_name[0]);
    break;  // there can only be one chosen action
  }
}

namespace VW
{
namespace cb_continuous
{
////////////////////////////////////////////////////
// Begin: parse a,c,p label format
void parse_label(parser* p, shared_data*, continuous_label& ld, std::vector<VW::string_view>& words,
    reduction_features& red_features)
{
  ld.costs.clear();

  if (words.empty()) { return; }

  if (!(words[0] == CA_LABEL)) { THROW("Continuous actions labels require the first word to be ca"); }

  for (size_t i = 1; i < words.size(); i++)
  {
    if (words[i] == PDF) { parse_pdf(words, i + 1, p, red_features); }
    else if (words[i] == CHOSEN_ACTION)
    {
      parse_chosen_action(words, i + 1, p, red_features);
    }
    else if (words[i - 1] == CA_LABEL)
    {
      continuous_label_elm f{0.f, FLT_MAX, 0.f};
      tokenize(':', words[i], p->parse_name);

      if (p->parse_name.empty() || p->parse_name.size() > 4)
        THROW("malformed cost specification: "
            << "p->parse_name");

      f.action = float_of_string(p->parse_name[0]);

      if (p->parse_name.size() > 1) f.cost = float_of_string(p->parse_name[1]);

      if (std::isnan(f.cost)) THROW("error NaN cost (" << p->parse_name[1] << " for action: " << p->parse_name[0]);

      f.pdf_value = .0;
      if (p->parse_name.size() > 2) f.pdf_value = float_of_string(p->parse_name[2]);

      if (std::isnan(f.pdf_value))
        THROW("error NaN pdf_value (" << p->parse_name[2] << " for action: " << p->parse_name[0]);

      if (f.pdf_value < 0.0)
      {
        logger::errlog_warn("invalid pdf_value < 0 specified for an action, resetting to 0.");
        f.pdf_value = .0;
      }

      ld.costs.push_back(f);
    }
  }
}

// clang-format off
label_parser the_label_parser = {
  // default_label
  [](polylabel* v) { CB::default_label<continuous_label>(v->cb_cont); },
  // parse_label
  [](parser* p, shared_data* sd, polylabel* v, std::vector<VW::string_view>& words, reduction_features& red_features) {
    parse_label(p, sd, v->cb_cont, words, red_features);
  },
  // cache_label
  [](polylabel* v, reduction_features&, io_buf& cache) { CB::cache_label<continuous_label, continuous_label_elm>(v->cb_cont, cache); },
  // read_cached_label
  [](shared_data* sd, polylabel* v, reduction_features&, io_buf& cache) { return CB::read_cached_label<continuous_label, continuous_label_elm>(sd, v->cb_cont, cache); },
  // get_weight
  // CB::weight just returns 1.f? This seems like it could be a bug...
  [](polylabel*, const reduction_features&) { return 1.f; },
  // test_label
  [](polylabel* v) { return CB::is_test_label<continuous_label, continuous_label_elm>(v->cb_cont); },
  VW::label_type_t::continuous
};

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
