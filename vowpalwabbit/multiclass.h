// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once
#include "label_parser.h"

struct example;
struct vw;

namespace MULTICLASS
{
struct label_t
{
  uint32_t label;
  float weight;
};

//void default_label(label_t& ld);
//void parse_label(parser*, shared_data* sd, label_t& ld, std::vector<VW::string_view>& words);
//void cache_label(const label_t& ld, io_buf& cache);
//size_t read_cached_label(shared_data*, label_t& ld, io_buf& cache);
//float weight(label_t& ld);
//bool test_label(const label_t& ld);

extern label_parser mc_label;

void print_update_with_probability(vw& all, example& ec, uint32_t prediction);
void print_update_with_score(vw& all, example& ec, uint32_t prediction);

void finish_example(vw& all, example& ec, bool update_loss);

template <class T>
void finish_example(vw& all, T&, example& ec)
{
  finish_example(all, ec, true);
}

template <class T>
void finish_example_without_loss(vw& all, T&, example& ec)
{
  finish_example(all, ec, false);
}
}  // namespace MULTICLASS
