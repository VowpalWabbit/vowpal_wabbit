#ifndef RL_H
#define RL_H

#include "io.h"
#include "parse_primitives.h"
#include "global_data.h"
#include "example.h"
#include "parse_args.h"

namespace RL
{

  struct reward_label {
    float label;
    float weight;
  };
  
  typedef float prediction_t;
  
  void parse_flags(vw& all, std::vector<std::string>&, po::variables_map& vm, double l, double g);
  
  size_t read_cached_label(shared_data*, void* v, io_buf& cache);
  void cache_label(void* v, io_buf& cache);
  void default_label(void* v);
  void parse_label(shared_data*, void* v, v_array<substring>& words);
  void delete_label(void* v);
  float weight(void* v);
  float initial(void* v);
  const label_parser rl_label_parser = {default_label, parse_label, 
					cache_label, read_cached_label, 
					delete_label, weight, initial, 
					sizeof(reward_label)};
  
  void output_example(vw& all, example* ec);
}

#endif
