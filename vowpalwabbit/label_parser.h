// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw_string_view.h"

#include <vector>

struct parser;
struct shared_data;
struct polylabel;
class io_buf;
class reduction_features;

enum class label_type_t
{
  simple,
  cb,       // contextual-bandit
  cb_eval,  // contextual-bandit evaluation
  cs,       // cost-sensitive
  multilabel,
  multiclass,
  ccb,  // conditional contextual-bandit
  slates,
  nolabel,
  continuous  // continuous actions
};

struct example;
struct label_parser
{
  void (*default_label)(polylabel*);
  void (*parse_label)(parser*, shared_data*, polylabel*, std::vector<VW::string_view>&, reduction_features&);
  void (*cache_label)(polylabel*, reduction_features&, io_buf& cache);
  size_t (*read_cached_label)(shared_data*, polylabel*, reduction_features&, io_buf& cache);
  void (*delete_label)(polylabel*);
  float (*get_weight)(polylabel*, const reduction_features&);
  void (*copy_label)(
      polylabel*, polylabel*);  // copy_label(dst,src) performs a DEEP copy of src into dst (dst is allocated
                                // correctly).  if this function is nullptr, then we assume that a memcpy of size
                                // label_size is sufficient, so you need only specify this function if your label
                                // constains, for instance, pointers (otherwise you'll get double-free errors)
  bool (*test_label)(polylabel*);
  void (*post_parse_setup)(example*);
  label_type_t label_type;
};
