
/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#pragma once
#include <iostream>
#include <iomanip>
#include <vector>
#include <map>
#include <cfloat>
#include <stdint.h>
#include <cstdio>
#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include "v_array.h"
#include "array_parameters.h"
#include "parse_primitives.h"
#include "loss_functions.h"
#include "comp_io.h"
#include "example.h"
#include "config.h"
#include "learner.h"
#include "v_hashmap.h"
#include <time.h>
#include "hash.h"
#include "crossplat_compat.h"

struct version_struct
{ int major;
  int minor;
  int rev;
  version_struct(int maj = 0, int min = 0, int rv = 0)
  { major = maj;
    minor = min;
    rev = rv;
  }
  version_struct(const char* v_str)
  { from_string(v_str);
  }
  void operator=(version_struct v)
  { major = v.major;
    minor = v.minor;
    rev = v.rev;
  }
  void operator=(const char* v_str)
  { from_string(v_str);
  }
  bool operator==(version_struct v)
  { return (major == v.major && minor == v.minor && rev == v.rev);
  }
  bool operator==(const char* v_str)
  { version_struct v_tmp(v_str);
    return (*this == v_tmp);
  }
  bool operator!=(version_struct v)
  { return !(*this == v);
  }
  bool operator!=(const char* v_str)
  { version_struct v_tmp(v_str);
    return (*this != v_tmp);
  }
  bool operator>=(version_struct v)
  { if(major < v.major) return false;
    if(major > v.major) return true;
    if(minor < v.minor) return false;
    if(minor > v.minor) return true;
    if(rev >= v.rev ) return true;
    return false;
  }
  bool operator>=(const char* v_str)
  { version_struct v_tmp(v_str);
    return (*this >= v_tmp);
  }
  bool operator>(version_struct v)
  { if(major < v.major) return false;
    if(major > v.major) return true;
    if(minor < v.minor) return false;
    if(minor > v.minor) return true;
    if(rev > v.rev ) return true;
    return false;
  }
  bool operator>(const char* v_str)
  { version_struct v_tmp(v_str);
    return (*this > v_tmp);
  }
  bool operator<=(version_struct v)
  { return !(*this > v);
  }
  bool operator<=(const char* v_str)
  { version_struct v_tmp(v_str);
    return (*this <= v_tmp);
  }
  bool operator<(version_struct v)
  { return !(*this >= v);
  }
  bool operator<(const char* v_str)
  { version_struct v_tmp(v_str);
    return (*this < v_tmp);
  }
  std::string to_string() const
  { char v_str[128];
    sprintf_s(v_str,sizeof(v_str),"%d.%d.%d",major,minor,rev);
    std::string s = v_str;
    return s;
  }
  void from_string(const char* str)
  {
#ifdef _WIN32
    sscanf_s(str, "%d.%d.%d", &major, &minor, &rev);
#else
    std::sscanf(str,"%d.%d.%d",&major,&minor,&rev);
#endif
  }
};

const version_struct version(PACKAGE_VERSION);

typedef float weight;

typedef v_hashmap< substring, features* > feature_dict;

struct dictionary_info
{ char* name;
  unsigned long long file_hash;
  feature_dict* dict;
};

inline void deleter(substring ss, uint64_t label)
{ free_it(ss.begin); }

class namedlabels
{
private:

  v_array<substring> id2name;
  v_hashmap<substring,uint64_t> name2id;
  uint32_t K;

public:

  namedlabels(std::string label_list)
  { id2name = v_init<substring>();
    char* temp = calloc_or_throw<char>(1+label_list.length());
    memcpy(temp, label_list.c_str(), strlen(label_list.c_str()));
    substring ss = { temp, nullptr };
    ss.end = ss.begin + label_list.length();
    tokenize(',', ss, id2name);

    K = (uint32_t)id2name.size();
    name2id.delete_v();//delete automatically allocated vector.
    name2id.init(4 * K + 1, 0, substring_equal);
    for (size_t k=0; k<K; k++)
    { substring& l = id2name[k];
      uint64_t hash = uniform_hash((unsigned char*)l.begin, l.end-l.begin, 378401);
      uint64_t id = name2id.get(l, hash);
      if (id != 0) // TODO: memory leak: char* temp
        THROW("error: label dictionary initialized with multiple occurances of: " << l);
      size_t len = l.end - l.begin;
      substring l_copy = { calloc_or_throw<char>(len), nullptr };
      memcpy(l_copy.begin, l.begin, len * sizeof(char));
      l_copy.end = l_copy.begin + len;
      name2id.put(l_copy, hash, (uint32_t)(k+1));
    }
  }

  ~namedlabels()
  { if (id2name.size()>0)
      free(id2name[0].begin);
    name2id.iter(deleter);
    name2id.delete_v();
    id2name.delete_v();
  }

  uint32_t getK() { return K; }

  uint64_t get(substring& s)
  { uint64_t hash = uniform_hash((unsigned char*)s.begin, s.end-s.begin, 378401);
    uint64_t v  =  name2id.get(s, hash);
    if (v == 0)
    { std::cerr << "warning: missing named label '";
      for (char*c = s.begin; c != s.end; c++) std::cerr << *c;
      std::cerr << '\'' << std::endl;
    }
    return v;
  }

  substring get(uint32_t v)
  { if ((v == 0) || (v > K))
    { substring ss = {nullptr,nullptr};
      return ss;
    }
    else
      return id2name[v-1];
  }
};

struct shared_data
{ size_t queries;

  uint64_t example_number;
  uint64_t total_features;

  double t;
  double weighted_labeled_examples;
  double old_weighted_labeled_examples;
  double weighted_unlabeled_examples;
  double weighted_labels;
  double sum_loss;
  double sum_loss_since_last_dump;
  float dump_interval;// when should I update for the user.
  double gravity;
  double contraction;
  float min_label;//minimum label encountered
  float max_label;//maximum label encountered

  namedlabels* ldict;

  //for holdout
  double weighted_holdout_examples;
  double weighted_holdout_examples_since_last_dump;
  double holdout_sum_loss_since_last_dump;
  double holdout_sum_loss;
  //for best model selection
  double holdout_best_loss;
  double weighted_holdout_examples_since_last_pass;//reserved for best predictor selection
  double holdout_sum_loss_since_last_pass;
  size_t holdout_best_pass;
  // for --probabilities
  bool report_multiclass_log_loss;
  double multiclass_log_loss;
  double holdout_multiclass_log_loss;
  
  bool  is_more_than_two_labels_observed;
  float first_observed_label;
  float second_observed_label;

  // Column width, precision constants:
  static const int col_avg_loss = 8;
  static const int prec_avg_loss = 6;
  static const int col_since_last = 8;
  static const int prec_since_last = 6;
  static const int col_example_counter = 12;
  static const int col_example_weight = col_example_counter + 2;
  static const int prec_example_weight = 1;
  static const int col_current_label = 8;
  static const int prec_current_label = 4;
  static const int col_current_predict = 8;
  static const int prec_current_predict = 4;
  static const int col_current_features = 8;

  double weighted_examples()
  {return weighted_labeled_examples + weighted_unlabeled_examples;}

  void update(bool test_example, bool labeled_example, float loss, float weight, size_t num_features)
  { t += weight;
    if(test_example)
    { weighted_holdout_examples += weight;//test weight seen
      weighted_holdout_examples_since_last_dump += weight;
      weighted_holdout_examples_since_last_pass += weight;
      holdout_sum_loss += loss;
      holdout_sum_loss_since_last_dump += loss;
      holdout_sum_loss_since_last_pass += loss;//since last pass
    }
    else
      {
	if (labeled_example)
	  weighted_labeled_examples += weight;
	else
	  weighted_unlabeled_examples += weight;
	sum_loss += loss;
	sum_loss_since_last_dump += loss;
	total_features += num_features;
	example_number++;
      }
  }

  inline void update_dump_interval(bool progress_add, float progress_arg)
  { sum_loss_since_last_dump = 0.0;
    old_weighted_labeled_examples = weighted_labeled_examples;
    if (progress_add)
      dump_interval = (float)weighted_examples() + progress_arg;
    else
      dump_interval = (float)weighted_examples() * progress_arg;
  }

  void print_update(bool holdout_set_off, size_t current_pass, float label, float prediction,
                    size_t num_features, bool progress_add, float progress_arg)
  { std::ostringstream label_buf, pred_buf;

    label_buf << std::setw(col_current_label)
              << std::setfill(' ');
    if (label < FLT_MAX)
      label_buf << std::setprecision(prec_current_label) << std::fixed << std::right << label;
    else
      label_buf << std::left << " unknown";

    pred_buf << std::setw(col_current_predict) << std::setprecision(prec_current_predict)
             << std::fixed << std::right
             << std::setfill(' ')
             << prediction;

    print_update(holdout_set_off, current_pass, label_buf.str(), pred_buf.str(), num_features,
                 progress_add, progress_arg);
  }

  void print_update(bool holdout_set_off, size_t current_pass, uint32_t label, uint32_t prediction,
                    size_t num_features, bool progress_add, float progress_arg)
  { std::ostringstream label_buf, pred_buf;

    label_buf << std::setw(col_current_label)
              << std::setfill(' ');
    if (label < INT_MAX)
      label_buf << std::right << label;
    else
      label_buf << std::left << " unknown";

    pred_buf << std::setw(col_current_predict) << std::right
             << std::setfill(' ')
             << prediction;

    print_update(holdout_set_off, current_pass, label_buf.str(), pred_buf.str(), num_features,
                 progress_add, progress_arg);
  }

  void print_update(bool holdout_set_off, size_t current_pass, const std::string &label, uint32_t prediction,
                    size_t num_features, bool progress_add, float progress_arg)
  { std::ostringstream pred_buf;

    pred_buf << std::setw(col_current_predict) << std::right << std::setfill(' ')
             << prediction;

    print_update(holdout_set_off, current_pass, label, pred_buf.str(), num_features,
                 progress_add, progress_arg);
  }

  void print_update(bool holdout_set_off, size_t current_pass, const std::string &label, const std::string &prediction,
                    size_t num_features, bool progress_add, float progress_arg)
  { std::streamsize saved_w = std::cerr.width();
    std::streamsize saved_prec = std::cerr.precision();
    std::ostream::fmtflags saved_f = std::cerr.flags();
    bool holding_out = false;

    if(!holdout_set_off && current_pass >= 1)
    { if(holdout_sum_loss == 0. && weighted_holdout_examples == 0.)
        std::cerr << std::setw(col_avg_loss) << std::left << " unknown";
      else
        std::cerr << std::setw(col_avg_loss) << std::setprecision(prec_avg_loss) << std::fixed << std::right
                  << (holdout_sum_loss / weighted_holdout_examples);

      std::cerr << " ";

      if(holdout_sum_loss_since_last_dump == 0. && weighted_holdout_examples_since_last_dump == 0.)
        std::cerr << std::setw(col_since_last) << std::left << " unknown";
      else
        std::cerr << std::setw(col_since_last) << std::setprecision(prec_since_last) << std::fixed << std::right
                  << (holdout_sum_loss_since_last_dump/weighted_holdout_examples_since_last_dump);

      weighted_holdout_examples_since_last_dump = 0;
      holdout_sum_loss_since_last_dump = 0.0;

      holding_out = true;
    }
    else
      {
	std::cerr << std::setw(col_avg_loss) << std::setprecision(prec_avg_loss) << std::right << std::fixed;
	if (weighted_labeled_examples > 0.)
	  std::cerr << (sum_loss / weighted_labeled_examples);
	else
	  std::cerr << "n.a.";
	std::cerr << " "
		  << std::setw(col_since_last) << std::setprecision(prec_avg_loss) << std::right << std::fixed;
	if (weighted_labeled_examples == old_weighted_labeled_examples)
	  std::cerr << "n.a.";
	else
	  std::cerr << (sum_loss_since_last_dump / (weighted_labeled_examples - old_weighted_labeled_examples));
    }
    std::cerr << " "
              << std::setw(col_example_counter) << std::right << example_number
              << " "
              << std::setw(col_example_weight) << std::setprecision(prec_example_weight) << std::right << weighted_examples()
              << " "
              << std::setw(col_current_label) << std::right << label
              << " "
              << std::setw(col_current_predict) << std::right << prediction
              << " "
              << std::setw(col_current_features) << std::right << num_features;

    if (holding_out)
      std::cerr << " h";

    std::cerr << std::endl;
    std::cerr.flush();

    std::cerr.width(saved_w);
    std::cerr.precision(saved_prec);
    std::cerr.setf(saved_f);

    update_dump_interval(progress_add, progress_arg);
  }
};

enum AllReduceType
{ Socket,
  Thread
};

class AllReduce;

// avoid name clash
namespace label_type
{ enum label_type_t
{ simple,
  cb, // contextual-bandit
  cb_eval, // contextual-bandit evaluation
  cs, // cost-sensitive
  multi,
  mc
};
}

typedef void(*trace_message_t)(void *context, const std::string&);

// TODO: change to virtual class

// invoke trace_listener when << endl is encountered.
class vw_ostream : public std::ostream
{
	class vw_streambuf : public std::stringbuf
	{
		vw_ostream& parent;
	public:
		vw_streambuf(vw_ostream& str);

		virtual int sync();
	};
	vw_streambuf buf;

public:
	vw_ostream();

	void* trace_context;
	trace_message_t trace_listener;
};

struct vw
{ shared_data* sd;

  parser* p;
#ifndef _WIN32
  pthread_t parse_thread;
#else
  HANDLE parse_thread;
#endif
  AllReduceType all_reduce_type;
  AllReduce* all_reduce;

  LEARNER::base_learner* l;//the top level learner
  LEARNER::base_learner* scorer;//a scoring function
  LEARNER::base_learner* cost_sensitive;//a cost sensitive learning algorithm.

  void learn(example*);

  void (*set_minmax)(shared_data* sd, float label);

  uint64_t current_pass;

  uint32_t num_bits; // log_2 of the number of features.
  bool default_bits;

  std::string data_filename; // was vm["data"]

  bool daemon;
  size_t num_children;

  bool save_per_pass;
  float initial_weight;
  float initial_constant;

  bool bfgs;
  bool hessian_on;

  bool save_resume;
  bool preserve_performance_counters;
  std::string id;

  version_struct model_file_ver;
  double normalized_sum_norm_x;
  bool vw_is_main;  // true if vw is executable; false in library mode

  po::options_description opts;
  po::options_description* new_opts;
  po::variables_map vm;
  std::stringstream* file_options;
  std::vector<std::string> args;

  void* /*Search::search*/ searchstr;

  uint32_t wpp;

  int stdout_fileno;

  std::string per_feature_regularizer_input;
  std::string per_feature_regularizer_output;
  std::string per_feature_regularizer_text;

  float l1_lambda; //the level of l_1 regularization to impose.
  float l2_lambda; //the level of l_2 regularization to impose.
  bool no_bias;    //no bias in regularization
  float power_t;//the power on learning rate decay.
  int reg_mode;

  size_t pass_length;
  size_t numpasses;
  size_t passes_complete;
  uint64_t parse_mask; // 1 << num_bits -1
  bool permutations; // if true - permutations of features generated instead of simple combinations. false by default
  v_array<v_string> interactions; // interactions of namespaces to cross.
  std::vector<std::string> pairs; // pairs of features to cross.
  std::vector<std::string> triples; // triples of features to cross.
  bool ignore_some;
  bool ignore[256];//a set of namespaces to ignore
  bool ignore_some_linear;
  bool ignore_linear[256];//a set of namespaces to ignore for linear

  bool redefine_some;          // --redefine param was used
  unsigned char redefine[256]; // keeps new chars for amespaces

  std::vector<std::string> ngram_strings;
  std::vector<std::string> skip_strings;
  uint32_t ngram[256];//ngrams to generate.
  uint32_t skips[256];//skips in ngrams.
  std::vector<std::string> limit_strings; // descriptor of feature limits
  uint32_t limit[256];//count to limit features by
  uint64_t affix_features[256]; // affixes to generate (up to 16 per namespace - 4 bits per affix)
  bool     spelling_features[256]; // generate spelling features for which namespace
  std::vector<std::string> dictionary_path;  // where to look for dictionaries
  std::vector<feature_dict*> namespace_dictionaries[256]; // each namespace has a list of dictionaries attached to it
  std::vector<dictionary_info> loaded_dictionaries; // which dictionaries have we loaded from a file to memory?

  void(*delete_prediction)(void*); bool audit; //should I print lots of debugging information?
  bool quiet;//Should I suppress progress-printing of updates?
  bool training;//Should I train if lable data is available?
  bool active;
  bool adaptive;//Should I use adaptive individual learning rates?
  bool normalized_updates; //Should every feature be normalized
  bool invariant_updates; //Should we use importance aware/safe updates
  uint64_t random_seed;
  uint64_t random_state; // per instance random_state
  bool random_weights;
  bool random_positive_weights; // for initialize_regressor w/ new_mf
  bool add_constant;
  bool nonormalize;
  bool do_reset_source;
  bool holdout_set_off;
  bool early_terminate;
  uint32_t holdout_period;
  uint32_t holdout_after;
  size_t check_holdout_every_n_passes;  // default: 1, but search might want to set it higher if you spend multiple passes learning a single policy

  size_t normalized_idx; //offset idx where the norm is stored (1 or 2 depending on whether adaptive is true)

  uint32_t lda;

  std::string text_regressor_name;
  std::string inv_hash_regressor_name;

  size_t length () { return ((size_t)1) << num_bits; };

  v_array<LEARNER::base_learner* (*)(vw&)> reduction_stack;

  //Prediction output
  v_array<int> final_prediction_sink; // set to send global predictions to.
  int raw_prediction; // file descriptors for text output.

  void (*print)(int,float,float,v_array<char>);
  void (*print_text)(int, std::string, v_array<char>);
  loss_function* loss;

  char* program_name;

  bool stdin_off;

  //runtime accounting variables.
  float initial_t;
  float eta;//learning rate control.
  float eta_decay_rate;
  time_t init_time;

  std::string final_regressor_name;

  parameters weights;
  
  size_t max_examples; // for TLC

  bool hash_inv;
  bool print_invert;

  // Set by --progress <arg>
  bool  progress_add;   // additive (rather than multiplicative) progress dumps
  float progress_arg;   // next update progress dump multiplier

  std::map< std::string, size_t> name_index_map;

  label_type::label_type_t label_type;

  vw_ostream trace_message;

  vw();

  // ostream doesn't have copy constructor and the python library used some boost code which code potentially invoke this
  vw(const vw &);
};

void print_result(int f, float res, float weight, v_array<char> tag);
void binary_print_result(int f, float res, float weight, v_array<char> tag);
void noop_mm(shared_data*, float label);
void get_prediction(int sock, float& res, float& weight);
void compile_gram(std::vector<std::string> grams, uint32_t* dest, char* descriptor, bool quiet);
void compile_limits(std::vector<std::string> limits, uint32_t* dest, bool quiet);
int print_tag(std::stringstream& ss, v_array<char> tag);
void add_options(vw& all, po::options_description& opts);
inline po::options_description_easy_init new_options(vw& all, std::string name = "\0")
{ all.new_opts = new po::options_description(name);
  return all.new_opts->add_options();
}
bool no_new_options(vw& all);
bool missing_option(vw& all, bool keep, const char* name, const char* description);
template <class T> bool missing_option(vw& all, const char* name, const char* description)
{ new_options(all)(name, po::value<T>(), description);
  return no_new_options(all);
}
template <class T, bool keep> bool missing_option(vw& all, const char* name,
    const char* description)
{ if (missing_option<T>(all, name, description))
    return true;
  if (keep)
    *all.file_options << " --" << name << " " << all.vm[name].as<T>();
  return false;
}
void add_options(vw& all);
