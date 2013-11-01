/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#ifndef CSOAA_H
#define CSOAA_H

#include "io_buf.h"
#include "parse_primitives.h"
#include "global_data.h"
#include "example.h"
#include "oaa.h"
#include "parser.h"
#include "parse_args.h"

namespace CSOAA {
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
  
  learner* setup(vw& all, std::vector<std::string>&, po::variables_map& vm, po::variables_map& vm_file);

  void finish_example(vw& all, void*, example* ec);
  size_t read_cached_label(shared_data* sd, void* v, io_buf& cache);
  void cache_label(void* v, io_buf& cache);
  void default_label(void* v);
  void parse_label(parser* p, shared_data* sd, void* v, v_array<substring>& words);
  void delete_label(void* v);
  void copy_label(void*&dst,void*src);
  float weight(void* v);
  float initial(void* v);
  const label_parser cs_label_parser = {default_label, parse_label, 
					cache_label, read_cached_label, 
					delete_label, weight, initial, 
                                        copy_label,
					sizeof(label)};

  bool example_is_test(example* ec);
}

namespace CSOAA_AND_WAP_LDF {
  typedef CSOAA::label label;

  learner* setup(vw& all, std::vector<std::string>&, po::variables_map& vm, po::variables_map& vm_file);
  void global_print_newline(vw& all);
  void output_example(vw& all, example* ec, bool&hit_loss);

  const label_parser cs_label_parser = CSOAA::cs_label_parser;
}

#endif
