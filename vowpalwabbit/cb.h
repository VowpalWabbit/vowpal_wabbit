#ifndef CB_H
#define CB_H

#define CB_TYPE_DR 0
#define CB_TYPE_DM 1
#define CB_TYPE_IPS 2

#include "global_data.h"
#include "parser.h"

//Contextual Bandit module to deal with incomplete cost-sensitive data
//Currently implemented as a reduction to cost-sensitive learning, using the methods discussed in the paper 'Doubly Robust Policy Evaluation and Learning'.

//CB is currently made to work with CSOAA or WAP as base cs learner
//TODO: extend to handle CSOAA_LDF and WAP_LDF

namespace CB {

  struct cb_class {  // names are for compatibility with 'features'
    float x;  // the cost of this class
    uint32_t weight_index;  // the index of this class
    float partial_prediction;  // a partial prediction: new!
    float prob_action; //new for bandit setting, specifies the probability our policy chose this class for importance weighting
    bool operator==(cb_class j){return weight_index == j.weight_index;}
  };

  struct label {
    v_array<cb_class> costs;
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
  const label_parser cb_label_parser = {default_label, parse_label, 
					cache_label, read_cached_label, 
					delete_label, weight, initial, 
					sizeof(label)};

}

#endif
