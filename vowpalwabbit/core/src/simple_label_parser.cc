// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/simple_label_parser.h"
#include "vw/core/vw_string_view_fmt.h"

#include "vw/common/string_view.h"
#include "vw/core/best_constant.h"
#include "vw/core/example.h"
#include "vw/core/model_utils.h"
#include "vw/core/parse_primitives.h"
#include "vw/io/logger.h"

#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstring>
// needed for printing ranges of objects (eg: all elements of a vector)
#include <fmt/ranges.h>

namespace
{
float get_weight(const VW::reduction_features& red_features)
{
  const auto& simple_red_features = red_features.template get<VW::simple_label_reduction_features>();
  return simple_red_features.weight;
}

void default_simple_label(VW::simple_label& ld) { ld.reset_to_default(); }

bool test_label(const VW::simple_label& ld) { return ld.label == FLT_MAX; }

// Example: 0 1 0.5 'third_house | price:.53 sqft:.32 age:.87 1924
// label := 0, weight := 1, initial := 0.5
void parse_simple_label(VW::simple_label& ld, VW::reduction_features& red_features,
    const std::vector<VW::string_view>& words, VW::io::logger& logger)
{
  auto& simple_red_features = red_features.template get<VW::simple_label_reduction_features>();
  switch (words.size())
  {
    case 0:
      break;
    case 1:
      ld.label = VW::details::float_of_string(words[0], logger);
      break;
    case 2:
      ld.label = VW::details::float_of_string(words[0], logger);
      simple_red_features.weight = VW::details::float_of_string(words[1], logger);
      break;
    case 3:
      ld.label = VW::details::float_of_string(words[0], logger);
      simple_red_features.weight = VW::details::float_of_string(words[1], logger);
      simple_red_features.initial = VW::details::float_of_string(words[2], logger);
      break;
    default:
      logger.out_error("Error: {0} is too many tokens for a simple label: {1}", words.size(), fmt::join(words, " "));
  }
}
}  // namespace

namespace VW
{
VW::label_parser simple_label_parser_global = {
    // default_label
    [](VW::polylabel& label) { default_simple_label(label.simple); },
    // parse_label
    [](VW::polylabel& label, VW::reduction_features& red_features, VW::label_parser_reuse_mem& /*reuse_mem*/,
        const VW::named_labels* /*ldict*/, const std::vector<VW::string_view>& words, VW::io::logger& logger)
    { parse_simple_label(label.simple, red_features, words, logger); },
    // cache_label
    [](const VW::polylabel& label, const VW::reduction_features& red_feats, io_buf& cache,
        const std::string& upstream_name, bool text)
    {
      size_t bytes = 0;
      bytes += VW::model_utils::write_model_field(cache, label.simple, upstream_name, text);
      bytes += VW::model_utils::write_model_field(
          cache, red_feats.template get<VW::simple_label_reduction_features>(), upstream_name, text);
      return bytes;
    },
    // read_cached_label
    [](VW::polylabel& label, VW::reduction_features& red_feats, io_buf& cache)
    {
      size_t bytes = 0;
      bytes += VW::model_utils::read_model_field(cache, label.simple);
      bytes += VW::model_utils::read_model_field(cache, red_feats.template get<VW::simple_label_reduction_features>());
      return bytes;
    },
    // get_weight
    [](const VW::polylabel& /*label*/, const VW::reduction_features& red_features)
    { return ::get_weight(red_features); },
    // test_label
    [](const VW::polylabel& label) { return test_label(label.simple); },
    // label type
    VW::label_type_t::SIMPLE};

namespace model_utils
{
size_t read_model_field(io_buf& io, VW::simple_label& ld)
{
  size_t bytes = 0;
  bytes += read_model_field(io, ld.label);
  return bytes;
}

size_t write_model_field(io_buf& io, const VW::simple_label& ld, const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  bytes += write_model_field(io, ld.label, upstream_name + "_label", text);
  return bytes;
}

size_t read_model_field(io_buf& io, simple_label_reduction_features& slrf)
{
  size_t bytes = 0;
  bytes += read_model_field(io, slrf.weight);
  bytes += read_model_field(io, slrf.initial);
  return bytes;
}

size_t write_model_field(
    io_buf& io, const simple_label_reduction_features& slrf, const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  bytes += write_model_field(io, slrf.weight, upstream_name + "_weight", text);
  bytes += write_model_field(io, slrf.initial, upstream_name + "_initial", text);
  return bytes;
}
}  // namespace model_utils
}  // namespace VW
