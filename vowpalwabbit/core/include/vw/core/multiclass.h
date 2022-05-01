// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "vw/core/label_parser.h"

class io_buf;
namespace VW
{
struct example;
struct workspace;
}  // namespace VW

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

extern VW::label_parser mc_label;

void print_update_with_probability(VW::workspace& all, VW::example& ec, uint32_t prediction);
void print_update_with_score(VW::workspace& all, VW::example& ec, uint32_t prediction);

void finish_example(VW::workspace& all, VW::example& ec, bool update_loss);

bool test_label(const label_t& ld);

template <class T>
void finish_example(VW::workspace& all, T&, VW::example& ec)
{
  finish_example(all, ec, true);
}

template <class T>
void finish_example_without_loss(VW::workspace& all, T&, VW::example& ec)
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
