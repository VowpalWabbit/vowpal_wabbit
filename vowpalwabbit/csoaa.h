#ifndef CSOAA_H
#define CSOAA_H

#include "io.h"
#include "parse_primitives.h"
#include "global_data.h"
#include "example.h"
#include "oaa.h"
#include "parser.h"

namespace CSOAA {

  struct wclass {  // names are for compatibility with 'feature'
    float x;  // the cost of this class
    uint32_t weight_index;  // the index of this class
    float partial_prediction;  // a partial prediction: new!
    bool operator==(wclass j){return weight_index == j.weight_index;}
  };

  
  struct label {
    v_array<wclass> costs;
  };
  
  void parse_flags(vw& all, std::vector<std::string>&, size_t s, void (*base_l)(vw&, example*), void (*base_f)(vw&));
  void learn(vw& all, example* ec);
  void finish(vw&);

  void output_example(vw& all, example* ec);
  size_t read_cached_label(shared_data* sd, void* v, io_buf& cache);
  void cache_label(void* v, io_buf& cache);
  void default_label(void* v);
  void parse_label(shared_data* sd, void* v, v_array<substring>& words);
  void delete_label(void* v);
  float weight(void* v);
  float initial(void* v);
  const label_parser cs_label_parser = {default_label, parse_label, 
					cache_label, read_cached_label, 
					delete_label, weight, initial, 
					sizeof(label)};
}

namespace CSOAA_LDF {
  typedef OAA::mc_label label;

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

  void learn(vw& all, example* ec);
  void finish(vw&);
  void parse_flags(vw& all, std::vector<std::string>&, size_t s, void (*base_l)(vw&, example*), void (*base_f)(vw&));
  void global_print_newline(vw& all);
  void output_example(vw& all, example* ec);

  const label_parser cs_label_parser = {OAA::default_label, OAA::parse_label, 
					OAA::cache_label, OAA::read_cached_label, 
					OAA::delete_label, OAA::weight, OAA::initial, 
					sizeof(label)};
}

#endif
