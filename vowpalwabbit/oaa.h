#ifndef OAA_H
#define OAA_H

#include "io.h"
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
  
  typedef uint32_t prediction_t;
  
  void parse_flags(vw& all, std::vector<std::string>&, po::variables_map& vm, size_t s);
  
  size_t read_cached_label(shared_data*, void* v, io_buf& cache);
  void cache_label(void* v, io_buf& cache);
  void default_label(void* v);
  void parse_label(shared_data*, void* v, v_array<substring>& words);
  void delete_label(void* v);
  float weight(void* v);
  float initial(void* v);
  const label_parser mc_label_parser = {default_label, parse_label, 
					cache_label, read_cached_label, 
					delete_label, weight, initial, 
					sizeof(mc_label)};
  
  void output_example(vw& all, example* ec);

  inline int example_is_newline(example* ec)
  {
    // if only index is constant namespace or no index
    return ((ec->indices.index() == 0) || 
            ((ec->indices.index() == 1) &&
             (ec->indices.last() == constant_namespace)));
  }

  inline int example_is_test(example* ec)
  {
    return (((OAA::mc_label*)ec->ld)->label == (uint32_t)-1);
  }


}

#endif
