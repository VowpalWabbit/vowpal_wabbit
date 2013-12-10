/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#ifndef OAA_H
#define OAA_H

#include "io_buf.h"
#include "parse_primitives.h"
#include "global_data.h"
#include "example.h"
#include "parse_args.h"
#include "v_hashmap.h"

namespace OAA
{

  struct mc_label {
    uint32_t label;
    float weight;
  };
  
  learner* setup(vw& all, std::vector<std::string>&, po::variables_map& vm, po::variables_map& vm_file);
  
  size_t read_cached_label(shared_data*, void* v, io_buf& cache);
  void cache_label(void* v, io_buf& cache);
  void default_label(void* v);
  void parse_label(parser* p, shared_data*, void* v, v_array<substring>& words);
  void delete_label(void* v);
  float weight(void* v);
  float initial(void* v);
  const label_parser mc_label_parser = {default_label, parse_label, 
					cache_label, read_cached_label, 
					delete_label, weight, initial, 
                                        NULL,
					sizeof(mc_label)};
  
  void finish_example(vw& all, void*, example* ec);

  inline int example_is_test(example* ec)
  { return (((OAA::mc_label*)ec->ld)->label == (uint32_t)-1); }

  inline int label_is_test(mc_label* ld)
  { return ld->label == (uint32_t)-1; }

}

#endif
















