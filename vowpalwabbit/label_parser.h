// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw_string_view.h"
#include "label_type.h"

#include <vector>

struct parser;
struct shared_data;
struct polylabel;
class io_buf;
class reduction_features;
struct example;
namespace VW
{
class named_labels;

// To avoid allocating memory for each parse call each label parser has access to this struct as a helper.
struct label_parser_reuse_mem
{
  std::vector<VW::string_view> tokens;
};
}  // namespace VW

struct label_parser
{
  void (*default_label)(polylabel& label);
  void (*parse_label)(polylabel& label, reduction_features& red_features, VW::label_parser_reuse_mem& reuse_mem,
      const VW::named_labels* ldict, const std::vector<VW::string_view>& words);
  void (*cache_label)(const polylabel& label, const reduction_features& red_features, io_buf& cache);
  size_t (*read_cached_label)(
      polylabel& label, reduction_features& red_features, const VW::named_labels* ldict, io_buf& cache);
  float (*get_weight)(const polylabel& label, const reduction_features& red_features);
  bool (*test_label)(const polylabel& label);
  VW::label_type_t label_type;
};

namespace VW
{
label_parser get_label_parser(VW::label_type_t label_type);
}  // namespace VW
