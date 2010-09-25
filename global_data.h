/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */

#ifndef GLOBAL_DATA_H
#define GLOBAL_DATA_H
#include <vector>
#include "v_array.h"
#include "parse_regressor.h"

using namespace std;

extern string version;

struct int_pair {
  int fd;
  int id;
};

struct global_data {
  size_t thread_bits; // log_2 of the number of threads.
  size_t partition_bits; // log_2 of the number of partitions of features.
  size_t num_bits; // log_2 of the number of features.
  bool default_bits;

  bool backprop;
  bool corrective;
  bool delayed_global;
  float global_multiplier;

  size_t numpasses;
  size_t thread_mask; // 1 << num_bits >> thread_bits - 1.
  size_t mask; // 1 << num_bits -1
  vector<string> pairs; // pairs of features to cross.
  size_t ngram;//ngrams to generate.
  size_t skips;//skips in ngrams.
  bool audit;//should I print lots of debugging information?
  bool quiet;//Should I suppress updates?
  bool training;//Should I train if label data is available?
  bool adaptive;//Should I use adaptive individual learning rates?
  
  double min_label;//minimum label encountered
  double max_label;//maximum label encountered

  size_t num_threads () { return 1 << thread_bits; };
  size_t num_partitions () { return 1 << partition_bits; };
  size_t length () { return 1 << num_bits; };

  //Prediction output
  v_array<int_pair> final_prediction_sink; // set to send global predictions to.
  int raw_prediction; // file descriptors for text output.
  int local_prediction;  //file descriptor to send local prediction to.
  size_t unique_id; //unique id for each node in the network, id == 0 means extra io.

  void (*print)(int,float,float,v_array<char>);
  loss_function* loss;

  char* program_name;

  //runtime accounting variables. 
  long long int example_number;
  double weighted_examples;
  double old_weighted_examples;
  double weighted_labels;
  size_t total_features;
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

const size_t ring_size = 1 << 11;

#endif
