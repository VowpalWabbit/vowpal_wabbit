// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <cstdint>

#include "label_parser.h"
#include "action_score.h"
namespace VW
{
namespace slates
{
enum example_type : uint8_t
{
  unset = 0,
  shared = 1,
  action = 2,
  slot = 3
};

struct label
{
  // General data
  example_type type ;
  float weight;
  // Because these labels provide both structural information as well as a
  // label, this field will only be true is there is a label attached (label in
  // the sense of cost)
  bool labeled;

  // For shared examples
  // Only valid if labeled
  float cost;

  // For action examples
  uint32_t slot_id;

  // For slot examples
  // Only valid if labeled
  ACTION_SCORE::action_scores probabilities;

  label() {
    reset_to_default();
  }

  void reset_to_default() {
    type = example_type::unset;
    weight = 1.f;
    labeled = false;
    cost = 0.f;
    slot_id = 0;
    probabilities.clear();
  }
};

void default_label(VW::slates::label& v);
void parse_label(
    parser* p, shared_data* /*sd*/, VW::slates::label& ld, std::vector<VW::string_view>& words, reduction_features&);
void delete_label(VW::slates::label& ld);
void cache_label(VW::slates::label& ld, io_buf& cache);
size_t read_cached_label(shared_data* /*sd*/, VW::slates::label& ld, io_buf& cache);
void copy_label(VW::slates::label& dst, VW::slates::label& src);

extern label_parser slates_label_parser;
}  // namespace slates
}  // namespace VW
