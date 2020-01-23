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

struct label_parser
{
  void (*default_label)(void*);
  void (*parse_label)(parser*, shared_data*, void*, v_array<VW::string_view>&);
  void (*cache_label)(void*, io_buf& cache);
  size_t (*read_cached_label)(shared_data*, void*, io_buf& cache);
  void (*delete_label)(void*);
  float (*get_weight)(void*);
  void (*copy_label)(void*, void*);  // copy_label(dst,src) performs a DEEP copy of src into dst (dst is allocated
                                     // correctly).  if this function is nullptr, then we assume that a memcpy of size
                                     // label_size is sufficient, so you need only specify this function if your label
                                     // constains, for instance, pointers (otherwise you'll get double-free errors)
  bool (*test_label)(void*);
  size_t label_size;
};
