/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#ifndef BS_H
#define BS_H

#include "io_buf.h"
#include "parse_primitives.h"
#include "global_data.h"
#include "example.h"
#include "parse_args.h"
#include "v_hashmap.h"

namespace BS
{

  struct mc_label {
    float label;
    float weight;
  };
  
  learner setup(vw& all, std::vector<std::string>&, po::variables_map& vm, po::variables_map& vm_file);
  
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
  
  void output_example(vw& all, example* ec);

  inline int example_is_newline(example* ec)
  {
    // if only index is constant namespace or no index
    return ((ec->indices.size() == 0) || 
            ((ec->indices.size() == 1) &&
             (ec->indices.last() == constant_namespace)));
  }

}

#endif
