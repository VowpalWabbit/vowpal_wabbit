#ifndef CSOAA_H
#define CSOAA_H

#include "io.h"
#include "parse_primitives.h"
#include "global_data.h"
#include "example.h"

namespace CSOAA {

  //  typedef label_cost feature;

  struct label {
    v_array<float> costs;
  };
  
  void parse_flag(size_t s);

  size_t read_cached_label(void* v, io_buf& cache);
  void cache_label(void* v, io_buf& cache);
  void default_label(void* v);
  void parse_label(void* v, v_array<substring>& words);
  void delete_label(void* v);
  float weight(void* v);
  float initial(void* v);
  const label_parser csoaa_label = {default_label, parse_label, 
                                    cache_label, read_cached_label, 
                                    delete_label, weight, initial, 
                                    sizeof(label)};

}
#endif
