// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <cstring>
#include <cfloat>
#include <cmath>
#include <cstdio>

#include "cache.h"
#include "best_constant.h"
#include "vw_string_view.h"
#include "parse_primitives.h"
#include "vw_string_view_fmt.h"
#include "simple_label_parser.h"
#include "model_utils.h"

#include "io/logger.h"
// needed for printing ranges of objects (eg: all elements of a vector)
#include <fmt/ranges.h>

namespace logger = VW::io::logger;

char* bufread_simple_label(label_data& ld, simple_label_reduction_features& red_features, char* c)
{
  memcpy(&ld.label, c, sizeof(ld.label));
  c += sizeof(ld.label);
  memcpy(&red_features.weight, c, sizeof(red_features.weight));
  c += sizeof(red_features.weight);
  memcpy(&red_features.initial, c, sizeof(red_features.initial));
  c += sizeof(red_features.initial);
  return c;
}

size_t read_cached_simple_label(label_data& ld, reduction_features& red_features, io_buf& cache)
{
  auto& simple_red_features = red_features.template get<simple_label_reduction_features>();
  char* c;
  size_t total = sizeof(ld.label) + sizeof(simple_red_features.weight) + sizeof(simple_red_features.initial);
  if (cache.buf_read(c, total) < total) { return 0; }
  bufread_simple_label(ld, simple_red_features, c);

  return total;
}

float get_weight(const reduction_features& red_features)
{
  const auto& simple_red_features = red_features.template get<simple_label_reduction_features>();
  return simple_red_features.weight;
}

char* bufcache_simple_label(const label_data& ld, const simple_label_reduction_features& red_features, char* c)
{
  memcpy(c, &ld.label, sizeof(ld.label));
  c += sizeof(ld.label);
  memcpy(c, &red_features.weight, sizeof(red_features.weight));
  c += sizeof(red_features.weight);
  memcpy(c, &red_features.initial, sizeof(red_features.initial));
  c += sizeof(red_features.initial);
  return c;
}

void cache_simple_label(const label_data& ld, const reduction_features& red_features, io_buf& cache)
{
  const auto& simple_red_features = red_features.template get<simple_label_reduction_features>();
  char* c;
  cache.buf_write(c, sizeof(ld.label) + sizeof(simple_red_features.weight) + sizeof(simple_red_features.initial));
  bufcache_simple_label(ld, simple_red_features, c);
}

void default_simple_label(label_data& ld) { ld.reset_to_default(); }

bool test_label(const label_data& ld) { return ld.label == FLT_MAX; }

// Example: 0 1 0.5 'third_house | price:.53 sqft:.32 age:.87 1924
// label := 0, weight := 1, initial := 0.5
void parse_simple_label(label_data& ld, reduction_features& red_features, const std::vector<VW::string_view>& words)
{
  auto& simple_red_features = red_features.template get<simple_label_reduction_features>();
  switch (words.size())
  {
    case 0:
      break;
    case 1:
      ld.label = float_of_string(words[0]);
      break;
    case 2:
      ld.label = float_of_string(words[0]);
      simple_red_features.weight = float_of_string(words[1]);
      break;
    case 3:
      ld.label = float_of_string(words[0]);
      simple_red_features.weight = float_of_string(words[1]);
      simple_red_features.initial = float_of_string(words[2]);
      break;
    default:
      logger::log_error("Error: {0} is too many tokens for a simple label: {1}", words.size(), fmt::join(words, " "));
  }
}

label_parser simple_label_parser = {
    // default_label
    [](polylabel& label) { default_simple_label(label.simple); },
    // parse_label
    [](polylabel& label, reduction_features& red_features, VW::label_parser_reuse_mem& /*reuse_mem*/,
        const VW::named_labels* /*ldict*/,
        const std::vector<VW::string_view>& words) { parse_simple_label(label.simple, red_features, words); },
    // cache_label
    [](const polylabel& label, const reduction_features&, io_buf& cache, const std::string& upstream_name, bool text) {
      return VW::model_utils::write_model_field(cache, label.simple, upstream_name, text);
    },
    // read_cached_label
    [](polylabel& label, reduction_features&, io_buf& cache) {
      return VW::model_utils::read_model_field(cache, label.simple);
    },
    // get_weight
    [](const polylabel& /*label*/, const reduction_features& red_features) { return get_weight(red_features); },
    // test_label
    [](const polylabel& label) { return test_label(label.simple); },
    // label type
    VW::label_type_t::simple};

namespace VW
{
namespace model_utils
{
size_t read_model_field(io_buf& io, label_data& ld)
{
  size_t bytes = 0;
  bytes += read_model_field(io, ld.label);
  return bytes;
}
size_t write_model_field(io_buf& io, const label_data& ld, const std::string& upstream_name, bool text)
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
