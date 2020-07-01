// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
namespace VW { namespace actions_pdf
{

struct action_pdf_value
{
  float action;     // continuous action
  float pdf_value;  // pdf value
};

struct pdf_segment
{
  float left;       // starting point
  float right;      // ending point
  float pdf_value;  // height
};

typedef v_array<pdf_segment> pdf;

void delete_prob_dist(void* v);

std::string to_string(const action_pdf_value& seg, bool print_newline = false);
std::string to_string(const v_array<pdf_segment>& pdf, bool print_newline = false);
}}  // namespace VW::actions_pdf
