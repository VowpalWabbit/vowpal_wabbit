/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#ifndef COST_SENSITIVE_H
#define COST_SENSITIVE_H

#include "io_buf.h"
#include "parse_primitives.h"
#include "global_data.h"
#include "example.h"
#include "parser.h"
#include "parse_args.h"

namespace COST_SENSITIVE {
  struct wclass {
    float x;
    uint32_t weight_index;
    float partial_prediction;  // a partial prediction: new!
    float wap_value;  // used for wap to store values derived from costs
    bool operator==(wclass j){return weight_index == j.weight_index;}
  };

  
  struct label {
    v_array<wclass> costs;
  };
  
  void output_example(vw& all, example& ec);
  size_t read_cached_label(shared_data* sd, void* v, io_buf& cache);
  void cache_label(void* v, io_buf& cache);
  void default_label(void* v);
  void parse_label(parser* p, shared_data* sd, void* v, v_array<substring>& words);
  void delete_label(void* v);
  void copy_label(void*&dst,void*src);
  float weight(void* v);
  const label_parser cs_label_parser = {default_label, parse_label, 
					cache_label, read_cached_label, 
					delete_label, weight, 
                                        copy_label,
					sizeof(label)};

  bool example_is_test(example& ec);

  void print_update(vw& all, bool is_test, example& ec);
}

namespace CSOAA_AND_WAP_LDF {
  void global_print_newline(vw& all);
  void output_example(vw& all, example& ec, bool& hit_loss);
}

#endif
