// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once
#include "io_buf.h"
#include "label_parser.h"

struct example;
namespace VW
{
struct workspace;
}

namespace MULTICLASS
{
struct label_t
{
  uint32_t label;
  float weight;

  label_t();
  label_t(uint32_t label, float weight);
  void reset_to_default();
};

extern label_parser mc_label;

void print_update_with_probability(VW::workspace& all, example& ec, uint32_t prediction);
void print_update_with_score(VW::workspace& all, example& ec, uint32_t prediction);

void finish_example(VW::workspace& all, example& ec, bool update_loss);

bool test_label(const label_t& ld);

template <class T>
void finish_example(VW::workspace& all, T&, example& ec)
{
  finish_example(all, ec, true);
}

template <class T>
void finish_example_without_loss(VW::workspace& all, T&, example& ec)
{
  finish_example(all, ec, false);
}
}  // namespace MULTICLASS

namespace VW
{
namespace model_utils
{
size_t read_model_field(io_buf&, MULTICLASS::label_t&);
size_t write_model_field(io_buf&, const MULTICLASS::label_t&, const std::string&, bool);
}  // namespace model_utils
}  // namespace VW
