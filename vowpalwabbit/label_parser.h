// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "v_array.h"
#include "parse_primitives.h"
#include "io_buf.h"

struct parser;
struct shared_data;
struct new_polylabel;

struct label_parser
{
  void (*default_label)(new_polylabel&);
  void (*parse_label)(parser*, shared_data*, new_polylabel&, v_array<substring>&);
  void (*cache_label)(new_polylabel&, io_buf& cache);
  size_t (*read_cached_label)(shared_data*, new_polylabel&, io_buf& cache);
  void (*delete_label)(new_polylabel&);
  float (*get_weight)(new_polylabel&);
  void (*copy_label)(new_polylabel&,
      new_polylabel&);  // copy_label(dst,src) performs a DEEP copy of src into dst (dst is allocated
                        // correctly).  if this function is nullptr, then we assume that a memcpy of size
                        // label_size is sufficient, so you need only specify this function if your label
                        // constains, for instance, pointers (otherwise you'll get double-free errors)
  bool (*test_label)(new_polylabel&);
  size_t label_size;
};

struct v_label_parser
{
  virtual void default_label(new_polylabel&) = 0;
  virtual void parse_label(parser*, shared_data*, new_polylabel&, v_array<substring>&) = 0;
  virtual void cache_label(new_polylabel&, io_buf& cache) = 0;
  virtual void read_cached_label(shared_data*, new_polylabel&, io_buf& cache) = 0;
  virtual void delete_label(new_polylabel&) = 0;
  virtual float get_weight(new_polylabel&) = 0;
  virtual void copy_label(new_polylabel&, new_polylabel&) = 0;
  virtual bool test_label(new_polylabel&) = 0;
  virtual size_t get_label_size() = 0;
};
