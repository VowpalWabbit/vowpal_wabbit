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

void polylabel_copy_label(polylabel& left, polylabel& right);
void polylabel_delete_label(polylabel& label);

struct label_parser
{
  void (*default_label)(polylabel&);
  void (*parse_label)(parser*, shared_data*, polylabel&, v_array<VW::string_view>&);
  void (*cache_label)(polylabel&, io_buf& cache);
  size_t (*read_cached_label)(shared_data*, polylabel&, io_buf& cache);
  VW_DEPRECATED("Removed")
  void (*delete_label)(polylabel&);
  float (*get_weight)(polylabel&);
  VW_DEPRECATED("Removed")
  void (*copy_label)(polylabel&, polylabel&);  // copy_label(dst,src) performs a DEEP copy of src into dst (dst is allocated
                                     // correctly).  if this function is nullptr, then we assume that a memcpy of size
                                     // label_size is sufficient, so you need only specify this function if your label
                                     // constains, for instance, pointers (otherwise you'll get double-free errors)  size_t label_size;
  bool (*test_label)(polylabel&);
  VW_DEPRECATED("Removed")
  size_t label_size;
};
