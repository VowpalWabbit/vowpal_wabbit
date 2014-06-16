/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#ifndef SIMPLE_LABEL_H
#define SIMPLE_LABEL_H

#include "io_buf.h"
#include "parse_primitives.h"
#include "global_data.h"
#include "example.h"

struct label_data {
  float label;
  float weight;
  float initial;
  float prediction;
};

void return_simple_example(vw& all, void*, example& ec);

extern label_parser simple_label;

float query_decision(vw& all, example& ec, float k);
bool summarize_holdout_set(vw& all, size_t& no_win_counter);
void print_update(vw& all, example &ec);
void output_and_account_example(vw& all, example& ec);

#endif
