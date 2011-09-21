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
#include "parse_regressor.h"

extern std::string version;

struct int_pair {
  int fd;
  int id;
};

struct global_data {
  size_t thread_bits; // log_2 of the number of threads.
  size_t partition_bits; // log_2 of the number of partitions of features.
  size_t num_bits; // log_2 of the number of features.
  bool default_bits;

  bool daemon; 
  size_t num_children;

  bool save_per_pass;
  bool backprop;
  bool corrective;
  bool delayed_global;
  float global_multiplier;
  float active_c0;
  float initial_weight;

  float regularization;

  bool bfgs;
  bool hessian_on;
  int m;

  size_t stride;


  std::string per_feature_regularizer_input;
  std::string per_feature_regularizer_output;
  std::string per_feature_regularizer_text;
  
  float l_1_regularization;//the level of l_1 regularization to impose.
  float update_sum;

  size_t minibatch;
  size_t ring_size;

  size_t pass_length;
  size_t numpasses;
  size_t passes_complete;
  size_t thread_mask; // 1 << num_bits >> thread_bits - 1.
  size_t mask; // 1 << num_bits -1
  std::vector<std::string> pairs; // pairs of features to cross.
  bool ignore_some;
  bool ignore[256];//a set of namespaces to ignore
  size_t ngram;//ngrams to generate.
  size_t skips;//skips in ngrams.
  size_t queries;
  bool audit;//should I print lots of debugging information?
  bool quiet;//Should I suppress updates?
  bool training;//Should I train if label data is available?
  bool active;
  bool active_simulation;
  bool adaptive;//Should I use adaptive individual learning rates?
  bool exact_adaptive_norm;//Should I use the exact norm when computing the update?
  bool random_weights;
  bool add_constant;
  
  double min_label;//minimum label encountered
  double max_label;//maximum label encountered

  size_t lda;
  float lda_alpha;
  float lda_rho;
  float lda_D;

  std::string text_regressor_name;
  
  std::string span_server;

  size_t num_threads () { return 1 << thread_bits; };
  size_t num_partitions () { return 1 << partition_bits; };
  size_t length () { return 1 << num_bits; };

  size_t rank;

  //Prediction output
  v_array<size_t> final_prediction_sink; // set to send global predictions to.
  int raw_prediction; // file descriptors for text output.
  int local_prediction;  //file descriptor to send local prediction to.
  size_t unique_id; //unique id for each node in the network, id == 0 means extra io.
  size_t total; //total number of nodes
  size_t node; //node id number

  void (*print)(int,float,float,v_array<char>);
  loss_function* loss;

  char* program_name;

  //runtime accounting variables. 
  uint64_t example_number;
  double initial_t;
  double weighted_examples;
  double weighted_unlabeled_examples;
  double old_weighted_examples;
  double weighted_labels;
  uint64_t total_features;
  double sum_loss;
  double sum_loss_since_last_dump;
  float dump_interval;// when should I update for the user.
  float eta;//learning rate control.
  float eta_decay_rate;

  regressor* reg;
};
extern pthread_mutex_t io;
extern global_data global;
extern void (*set_minmax)(double label);
void print_result(int f, float res, float weight, v_array<char> tag);
void binary_print_result(int f, float res, float weight, v_array<char> tag);
void noop_mm(double label);
void print_lda_result(int f, float* res, float weight, v_array<char> tag);

extern pthread_mutex_t output_lock;
extern pthread_cond_t output_done;

#endif
