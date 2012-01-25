#ifndef OAA_H
#define OAA_H

#include "io.h"
#include "parse_primitives.h"
#include "global_data.h"
#include "example.h"

namespace OAA
{

  struct mc_label {
    uint32_t label;
    float weight;
  };
  
  typedef uint32_t prediction_t;
  
  void parse_flags(size_t s, void (*base_l)(example*), void (*base_f)());
  void learn(example* ec);
  void finish();
  
  size_t read_cached_label(void* v, io_buf& cache);
  void cache_label(void* v, io_buf& cache);
  void default_label(void* v);
  void parse_label(void* v, v_array<substring>& words);
  void delete_label(void* v);
  float weight(void* v);
  float initial(void* v);
  const label_parser mc_label_parser = {default_label, parse_label, 
					cache_label, read_cached_label, 
					delete_label, weight, initial, 
					sizeof(mc_label)};
  
  void update_indicies(example* ec, size_t amount);

}

#endif
