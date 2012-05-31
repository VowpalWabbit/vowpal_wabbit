/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */

#ifndef GLOBAL_DATA_H
#define GLOBAL_DATA_H
#include <vector>
#include <stdint.h>
#include "v_array.h"
#include "parse_primitives.h"
#include "loss_functions.h"
#include "comp_io.h"
#include "example.h"
#include "config.h"

const std::string version = PACKAGE_VERSION;

typedef float weight;

struct regressor {
  weight* weight_vectors;
  weight* regularizers;
};

struct vw {
  shared_data* sd;

  label_parser* lp;
  parser* p;

  void (*driver)(void *);
  void (*set_minmax)(shared_data* sd, double label);

  size_t num_bits; // log_2 of the number of features.
  bool default_bits;

  string data_filename; // was vm["data"]

  bool daemon; 
  size_t num_children;

  bool save_per_pass;
  float active_c0;
  float initial_weight;

  bool bfgs;
  bool hessian_on;
  int m;

  bool sequence;
  bool searn;

  size_t stride;

  std::string per_feature_regularizer_input;
  std::string per_feature_regularizer_output;
  std::string per_feature_regularizer_text;
  
  float l1_lambda; //the level of l_1 regularization to impose.
  float l2_lambda; //the level of l_2 regularization to impose.
  float power_t;//the power on learning rate decay.
  int reg_mode;

  size_t minibatch;

  float rel_threshold; // termination threshold

  size_t pass_length;
  size_t numpasses;
  size_t passes_complete;
  size_t parse_mask; // 1 << num_bits -1
  size_t weight_mask; // (stride*(1 << num_bits) -1)
  std::vector<std::string> pairs; // pairs of features to cross.
  bool ignore_some;
  bool ignore[256];//a set of namespaces to ignore
  size_t ngram;//ngrams to generate.
  size_t skips;//skips in ngrams.
  bool audit;//should I print lots of debugging information?
  bool quiet;//Should I suppress updates?
  bool training;//Should I train if label data is available?
  bool active;
  bool active_simulation;
  bool adaptive;//Should I use adaptive individual learning rates?
  bool exact_adaptive_norm;//Should I use the exact norm when computing the update?
  bool random_weights;
  bool add_constant;
  bool nonormalize;

  size_t lda;
  float lda_alpha;
  float lda_rho;
  float lda_D;

  std::string text_regressor_name;
  
  std::string span_server;

  size_t length () { return 1 << num_bits; };

  size_t rank;

  //Prediction output
  v_array<size_t> final_prediction_sink; // set to send global predictions to.
  int raw_prediction; // file descriptors for text output.
  size_t unique_id; //unique id for each node in the network, id == 0 means extra io.
  size_t total; //total number of nodes
  size_t node; //node id number

  void (*print)(int,float,float,v_array<char>);
  loss_function* loss;

  char* program_name;

  //runtime accounting variables. 
  double initial_t;
  float eta;//learning rate control.
  float eta_decay_rate;

  std::string final_regressor_name;
  regressor reg;

  vw();
};

void print_result(int f, float res, float weight, v_array<char> tag);
void binary_print_result(int f, float res, float weight, v_array<char> tag);
void noop_mm(shared_data*, double label);
void print_lda_result(vw& all, int f, float* res, float weight, v_array<char> tag);
void get_prediction(int sock, float& res, float& weight);

#endif
