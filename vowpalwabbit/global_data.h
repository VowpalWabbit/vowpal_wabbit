/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#ifndef GLOBAL_DATA_H
#define GLOBAL_DATA_H
#include <vector>
#include <map>
#include <stdint.h>
#include <cstdio>
#include "v_array.h"
#include "parse_primitives.h"
#include "loss_functions.h"
#include "comp_io.h"
#include "example.h"
#include "config.h"
#include "learner.h"
#include "allreduce.h"

struct version_struct {
  int major;
  int minor;
  int rev;
  version_struct(int maj, int min, int rv)
  {
    major = maj;
    minor = min;
    rev = rv;
  }
  version_struct(const char* v_str)
  {
    from_string(v_str);
  }
  void operator=(version_struct v){
    major = v.major;
    minor = v.minor;
    rev = v.rev;
  }
  void operator=(const char* v_str){
    from_string(v_str);
  }
  bool operator==(version_struct v){
    return (major == v.major && minor == v.minor && rev == v.rev);
  }
  bool operator==(const char* v_str){
    version_struct v_tmp(v_str);
    return (*this == v_tmp);
  }
  bool operator!=(version_struct v){
    return !(*this == v);
  }
  bool operator!=(const char* v_str){
    version_struct v_tmp(v_str);
    return (*this != v_tmp);
  }
  bool operator>=(version_struct v){
    if(major < v.major) return false;
    if(major > v.major) return true;
    if(minor < v.minor) return false;
    if(minor > v.minor) return true;
    if(rev >= v.rev ) return true;
    return false;
  }
  bool operator>=(const char* v_str){
    version_struct v_tmp(v_str);
    return (*this >= v_tmp);
  }
  bool operator>(version_struct v){
    if(major < v.major) return false;
    if(major > v.major) return true;
    if(minor < v.minor) return false;
    if(minor > v.minor) return true;
    if(rev > v.rev ) return true;
    return false;
  }
  bool operator>(const char* v_str){
    version_struct v_tmp(v_str);
    return (*this > v_tmp);
  }
  bool operator<=(version_struct v){
    return !(*this < v);
  }
  bool operator<=(const char* v_str){
    version_struct v_tmp(v_str);
    return (*this <= v_tmp);
  }
  bool operator<(version_struct v){
    return !(*this >= v);
  }
  bool operator<(const char* v_str){
    version_struct v_tmp(v_str);
    return (*this < v_tmp);
  }
  std::string to_string() const
  {
    char v_str[128];
    std::sprintf(v_str,"%d.%d.%d",major,minor,rev);
    std::string s = v_str;
    return s;
  }
  void from_string(const char* str)
  {
    std::sscanf(str,"%d.%d.%d",&major,&minor,&rev);
  }
};

const version_struct version(PACKAGE_VERSION);

typedef float weight;

struct regressor {
  weight* weight_vector;
  size_t weight_mask; // (stride*(1 << num_bits) -1)
  uint32_t stride;
};

struct vw {
  shared_data* sd;

  parser* p;
#ifndef _WIN32
  pthread_t parse_thread;
#else
  HANDLE parse_thread;
#endif

  node_socks socks;

  learner* l;//the top level leaner
  learner* scorer;//a scoring function

  void learn(example*);

  void (*set_minmax)(shared_data* sd, float label);

  size_t current_pass;

  uint32_t num_bits; // log_2 of the number of features.
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

  bool save_resume;

  std::string options_from_file;
  char** options_from_file_argv;
  int options_from_file_argc;

  bool searn;
  void* /*Searn::searn*/ searnstr;

  uint32_t wpp; 

  int stdout_fileno;

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
  std::vector<std::string> pairs; // pairs of features to cross.
  std::vector<std::string> triples; // triples of features to cross.
  bool ignore_some;
  bool ignore[256];//a set of namespaces to ignore

  std::vector<std::string> ngram_strings; // pairs of features to cross.
  std::vector<std::string> skip_strings; // triples of features to cross.
  uint32_t ngram[256];//ngrams to generate.
  uint32_t skips[256];//skips in ngrams.
  uint32_t affix_features[256]; // affixes to generate (up to 8 per namespace)
  bool     spelling_features[256]; // generate spelling features for which namespace
  bool audit;//should I print lots of debugging information?
  bool quiet;//Should I suppress updates?
  bool training;//Should I train if lable data is available?
  bool active;
  bool active_simulation;
  bool adaptive;//Should I use adaptive individual learning rates?
  bool normalized_updates; //Should every feature be normalized
  bool invariant_updates; //Should we use importance aware/safe updates
  bool random_weights;
  bool add_constant;
  bool nonormalize;
  bool do_reset_source;
  bool holdout_set_off;
  bool early_terminate;
  uint32_t holdout_period;
  uint32_t holdout_after;
  size_t check_holdout_every_n_passes;  // default: 1, but searn might want to set it higher if you spend multiple passes learning a single policy

  float normalized_sum_norm_x;
  size_t normalized_idx; //offset idx where the norm is stored (1 or 2 depending on whether adaptive is true)
  size_t feature_mask_idx; //offset idx where mask is stored

  size_t lda;
  float lda_alpha;
  float lda_rho;
  float lda_D;

  std::string text_regressor_name;
  std::string inv_hash_regressor_name;
  
  std::string span_server;

  size_t length () { return ((size_t)1) << num_bits; };

  uint32_t rank;

  //Prediction output
  v_array<int> final_prediction_sink; // set to send global predictions to.
  int raw_prediction; // file descriptors for text output.
  size_t unique_id; //unique id for each node in the network, id == 0 means extra io.
  size_t total; //total number of nodes
  size_t node; //node id number

  void (*print)(int,float,float,v_array<char>);
  void (*print_text)(int, string, v_array<char>);
  loss_function* loss;

  char* program_name;

  bool stdin_off;

  //runtime accounting variables. 
  float initial_t;
  float eta;//learning rate control.
  float eta_decay_rate;

  std::string final_regressor_name;
  regressor reg;

  size_t max_examples; // for TLC

  bool hash_inv;
  bool print_invert;
  std::map< std::string, size_t> name_index_map;

  vw();
};

void print_result(int f, float res, float weight, v_array<char> tag);
void binary_print_result(int f, float res, float weight, v_array<char> tag);
void active_print_result(int f, float res, float weight, v_array<char> tag);
void noop_mm(shared_data*, float label);
void print_lda_result(vw& all, int f, float* res, float weight, v_array<char> tag);
void get_prediction(int sock, float& res, float& weight);
void compile_gram(vector<string> grams, uint32_t* dest, char* descriptor, bool quiet);
int print_tag(std::stringstream& ss, v_array<char> tag);

#endif
 
