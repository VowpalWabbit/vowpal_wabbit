/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#ifndef CONTEXTUAL_BANDIT_H
#define CONTEXTUAL_BANDIT_H

namespace CB {

  struct cb_class {
    float cost;  // the cost of this class
    uint32_t action;  // the index of this class
    float probability; //new for bandit setting, specifies the probability the data collection policy chose this class for importance weighting
    float partial_prediction;//essentially a return value
    bool operator==(cb_class j){return action == j.action;}
  };

  struct label {
    v_array<cb_class> costs;
  };

  size_t read_cached_label(shared_data* sd, void* v, io_buf& cache);
  void cache_label(void* v, io_buf& cache);
  void default_label(void* v);
  void parse_label(parser* p, shared_data* sd, void* v, v_array<substring>& words);
  void delete_label(void* v);
  float weight(void* v);
  void copy_label(void*&dst,void*src);
  const label_parser cb_label_parser = {default_label, parse_label, 
					cache_label, read_cached_label, 
					delete_label, weight, 
                                        copy_label,
					sizeof(label)};
}

#endif
