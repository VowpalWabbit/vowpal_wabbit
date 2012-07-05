#ifndef CSOAA_H
#define CSOAA_H

#include "io.h"
#include "parse_primitives.h"
#include "global_data.h"
#include "example.h"
#include "oaa.h"
#include "parser.h"
#include "parse_args.h"

namespace CSOAA {
  struct wclass {
    float x;
    size_t weight_index;
    float partial_prediction;  // a partial prediction: new!
    float wap_value;  // used for wap to store values derived from costs
    bool operator==(wclass j){return weight_index == j.weight_index;}
  };

  
  struct label {
    v_array<wclass> costs;
  };
  
  void parse_flags(vw& all, std::vector<std::string>&, po::variables_map& vm, size_t s);

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

  int example_is_test(example* ec);
}

namespace CSOAA_AND_WAP_LDF {
  typedef CSOAA::label label;

  void parse_flags(vw& all, std::string ldf_arg, std::vector<std::string>&, po::variables_map& vm, size_t s);
  void global_print_newline(vw& all);
  void output_example(vw& all, example* ec, bool&hit_loss);

  const label_parser cs_label_parser = CSOAA::cs_label_parser;
}

namespace LabelDict {
  bool ec_is_label_definition(example*ec);
  bool ec_is_example_header(example*ec);
  bool ec_seq_is_label_definition(v_array<example*>ec_seq);
  void add_example_namespaces_from_example(example*target, example*source);
  void del_example_namespaces_from_example(example*target, example*source);
  void add_example_namespace_from_memory(example*ec, size_t lab);
  void del_example_namespace_from_memory(example* ec, size_t lab);
  void set_label_features(size_t lab, v_array<feature>features);
  void free_label_features();
}

#endif
