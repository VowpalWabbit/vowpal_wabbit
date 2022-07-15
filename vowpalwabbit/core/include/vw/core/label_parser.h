// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/common/string_view.h"
#include "vw/core/label_type.h"
#include "vw/io/logger.h"

#include <string>
#include <vector>

struct parser;
struct shared_data;
class io_buf;
namespace VW
{
class reduction_features;
struct example;
struct polylabel;
class named_labels;

// To avoid allocating memory for each parse call each label parser has access to this struct as a helper.
struct label_parser_reuse_mem
{
  std::vector<VW::string_view> tokens;
};

struct label_parser
{
  void (*default_label)(polylabel& label);
  void (*parse_label)(polylabel& label, reduction_features& red_features, VW::label_parser_reuse_mem& reuse_mem,
      const VW::named_labels* ldict, const std::vector<VW::string_view>& words, VW::io::logger& logger);
  size_t (*cache_label)(const polylabel& label, const reduction_features& red_features, io_buf& cache,
      const std::string& upstream_name, bool text);
  size_t (*read_cached_label)(polylabel& label, reduction_features& red_features, io_buf& cache);
  float (*get_weight)(const polylabel& label, const reduction_features& red_features);
  bool (*test_label)(const polylabel& label);
  VW::label_type_t label_type;
};

label_parser get_label_parser(VW::label_type_t label_type);
}  // namespace VW

using label_parser VW_DEPRECATED("label_parser moved into VW namespace") = VW::label_parser;