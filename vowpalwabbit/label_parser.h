// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "v_array.h"
#include "parse_primitives.h"
#include "io_buf.h"

#include "vw_string_view.h"

struct parser;
struct shared_data;
struct polylabel;

struct label_parser
{
  void (*default_label)(polylabel&);
  void (*parse_label)(parser*, shared_data*, polylabel&, v_array<VW::string_view>&);
  void (*cache_label)(polylabel&, io_buf& cache);
  size_t (*read_cached_label)(shared_data*, polylabel&, io_buf& cache);
  float (*get_weight)(polylabel&);
  bool (*test_label)(polylabel&);
  size_t label_size;
};
