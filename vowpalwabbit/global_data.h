// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once
#include <iostream>
#include <iomanip>
#include <vector>
#include <map>
#include <cfloat>
#include <cstdint>
#include <cstdio>
#include <inttypes.h>
#include <climits>
#include <stack>
#include <unordered_map>
#include <string>
#include <array>
#include <memory>
#include "vw_string_view.h"

// Thread cannot be used in managed C++, tell the compiler that this is unmanaged even if included in a managed project.
#ifdef _M_CEE
#pragma managed(push, off)
#undef _M_CEE
#include <thread>
#define _M_CEE 001
#pragma managed(pop)
#else
#include <thread>
#endif

#include "v_array.h"
#include "array_parameters.h"
#include "parse_primitives.h"
#include "loss_functions.h"
#include "comp_io.h"
#include "example.h"
#include "config.h"
#include "learner.h"
#include <time.h>
#include "hash.h"
#include "crossplat_compat.h"
#include "error_reporting.h"
#include "constant.h"
#include "rand48.h"
#include "hashstring.h"

#include "options.h"
#include "version.h"

typedef float weight;

typedef std::unordered_map<std::string, std::unique_ptr<features>> feature_dict;

struct dictionary_info
{
  std::string name;
  uint64_t file_hash;
  std::shared_ptr<feature_dict> dict;
};

class namedlabels
{
 private:
  // NOTE: This ordering is critical. m_id2name and m_name2id contain pointers into m_label_list!
  std::string m_label_list;
  std::vector<VW::string_view> m_id2name;
  std::unordered_map<VW::string_view, uint64_t> m_name2id;
  uint32_t m_K;

 public:
  namedlabels(const std::string& label_list) : m_label_list(label_list)
  {
    tokenize(',', m_label_list, m_id2name);

    m_K = (uint32_t)m_id2name.size();
    m_name2id.max_load_factor(0.25);
    m_name2id.reserve(m_K);

    for (size_t k = 0; k < m_K; k++)
    {
      const VW::string_view& l = m_id2name[k];
      auto iter = m_name2id.find(l);
      if (iter != m_name2id.end())
        THROW("error: label dictionary initialized with multiple occurances of: " << l);
      m_name2id.emplace(l, k + 1);
    }
  }

  uint32_t getK() { return m_K; }

  uint32_t get(VW::string_view s) const
  {
    auto iter = m_name2id.find(s);
    if (iter == m_name2id.end())
    {
      std::cerr << "warning: missing named label '" << s << '\'' << std::endl;
      return 0;
    }
    return iter->second;
  }

  VW::string_view get(uint32_t v) const
  {
    static_assert(sizeof(size_t) >= sizeof(uint32_t), "size_t is smaller than 32-bits. Potential overflow issues.");
    if ((v == 0) || (v > m_K))
    {
      return VW::string_view();
    }
    else
      return m_id2name[(size_t)(v - 1)];
  }
};

struct shared_data
{
  size_t queries;

  uint64_t example_number;
  uint64_t total_features;

  double t;
  double weighted_labeled_examples;
  double old_weighted_labeled_examples;
  double weighted_unlabeled_examples;
  double weighted_labels;
  double sum_loss;
  double sum_loss_since_last_dump;
  float dump_interval;  // when should I update for the user.
  double gravity;
  double contraction;
  float min_label;  // minimum label encountered
  float max_label;  // maximum label encountered

  namedlabels* ldict;

  // for holdout
  double weighted_holdout_examples;
  double weighted_holdout_examples_since_last_dump;
  double holdout_sum_loss_since_last_dump;
  double holdout_sum_loss;
  // for best model selection
  double holdout_best_loss;
  double weighted_holdout_examples_since_last_pass;  // reserved for best predictor selection
  double holdout_sum_loss_since_last_pass;
  size_t holdout_best_pass;
  // for --probabilities
  bool report_multiclass_log_loss;
  double multiclass_log_loss;
  double holdout_multiclass_log_loss;

  bool is_more_than_two_labels_observed;
  float first_observed_label;
  float second_observed_label;

  // Column width, precision constants:
  static constexpr int col_avg_loss = 8;
  static constexpr int prec_avg_loss = 6;
  static constexpr int col_since_last = 8;
  static constexpr int prec_since_last = 6;
  static constexpr int col_example_counter = 12;
  static constexpr int col_example_weight = col_example_counter + 2;
  static constexpr int prec_example_weight = 1;
  static constexpr int col_current_label = 8;
  static constexpr int prec_current_label = 4;
  static constexpr int col_current_predict = 8;
  static constexpr int prec_current_predict = 4;
  static constexpr int col_current_features = 8;

  double weighted_examples() { return weighted_labeled_examples + weighted_unlabeled_examples; }

  void update(bool test_example, bool labeled_example, float loss, float weight, size_t num_features)
  {
    t += weight;
    if (test_example && labeled_example)
    {
      weighted_holdout_examples += weight;  // test weight seen
      weighted_holdout_examples_since_last_dump += weight;
      weighted_holdout_examples_since_last_pass += weight;
      holdout_sum_loss += loss;
      holdout_sum_loss_since_last_dump += loss;
      holdout_sum_loss_since_last_pass += loss;  // since last pass
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
  {
    sum_loss_since_last_dump = 0.0;
    old_weighted_labeled_examples = weighted_labeled_examples;
    if (progress_add)
      dump_interval = (float)weighted_examples() + progress_arg;
    else
      dump_interval = (float)weighted_examples() * progress_arg;
  }

  void print_update(bool holdout_set_off, size_t current_pass, float label, float prediction, size_t num_features,
      bool progress_add, float progress_arg)
  {
    std::ostringstream label_buf, pred_buf;

    label_buf << std::setw(col_current_label) << std::setfill(' ');
    if (label < FLT_MAX)
      label_buf << std::setprecision(prec_current_label) << std::fixed << std::right << label;
    else
      label_buf << std::left << " unknown";

    pred_buf << std::setw(col_current_predict) << std::setprecision(prec_current_predict) << std::fixed << std::right
             << std::setfill(' ') << prediction;

    print_update(
        holdout_set_off, current_pass, label_buf.str(), pred_buf.str(), num_features, progress_add, progress_arg);
  }

  void print_update(bool holdout_set_off, size_t current_pass, uint32_t label, uint32_t prediction, size_t num_features,
      bool progress_add, float progress_arg)
  {
    std::ostringstream label_buf, pred_buf;

    label_buf << std::setw(col_current_label) << std::setfill(' ');
    if (label < INT_MAX)
      label_buf << std::right << label;
    else
      label_buf << std::left << " unknown";

    pred_buf << std::setw(col_current_predict) << std::right << std::setfill(' ') << prediction;

    print_update(
        holdout_set_off, current_pass, label_buf.str(), pred_buf.str(), num_features, progress_add, progress_arg);
  }

  void print_update(bool holdout_set_off, size_t current_pass, const std::string& label, uint32_t prediction,
      size_t num_features, bool progress_add, float progress_arg)
  {
    std::ostringstream pred_buf;

    pred_buf << std::setw(col_current_predict) << std::right << std::setfill(' ') << prediction;

    print_update(holdout_set_off, current_pass, label, pred_buf.str(), num_features, progress_add, progress_arg);
  }

  void print_update(bool holdout_set_off, size_t current_pass, const std::string& label, const std::string& prediction,
      size_t num_features, bool progress_add, float progress_arg)
  {
    std::streamsize saved_w = std::cerr.width();
    std::streamsize saved_prec = std::cerr.precision();
    std::ostream::fmtflags saved_f = std::cerr.flags();
    bool holding_out = false;

    if (!holdout_set_off && current_pass >= 1)
    {
      if (holdout_sum_loss == 0. && weighted_holdout_examples == 0.)
        std::cerr << std::setw(col_avg_loss) << std::left << " unknown";
      else
        std::cerr << std::setw(col_avg_loss) << std::setprecision(prec_avg_loss) << std::fixed << std::right
                  << (holdout_sum_loss / weighted_holdout_examples);

      std::cerr << " ";

      if (holdout_sum_loss_since_last_dump == 0. && weighted_holdout_examples_since_last_dump == 0.)
        std::cerr << std::setw(col_since_last) << std::left << " unknown";
      else
        std::cerr << std::setw(col_since_last) << std::setprecision(prec_since_last) << std::fixed << std::right
                  << (holdout_sum_loss_since_last_dump / weighted_holdout_examples_since_last_dump);

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
      std::cerr << " " << std::setw(col_since_last) << std::setprecision(prec_avg_loss) << std::right << std::fixed;
      if (weighted_labeled_examples == old_weighted_labeled_examples)
        std::cerr << "n.a.";
      else
        std::cerr << (sum_loss_since_last_dump / (weighted_labeled_examples - old_weighted_labeled_examples));
    }
    std::cerr << " " << std::setw(col_example_counter) << std::right << example_number << " "
              << std::setw(col_example_weight) << std::setprecision(prec_example_weight) << std::right
              << weighted_examples() << " " << std::setw(col_current_label) << std::right << label << " "
              << std::setw(col_current_predict) << std::right << prediction << " " << std::setw(col_current_features)
              << std::right << num_features;

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
{
  Socket,
  Thread
};

class AllReduce;


enum class label_type_t
{
  simple,
  cb,       // contextual-bandit
  cb_eval,  // contextual-bandit evaluation
  cs,       // cost-sensitive
  multi,
  mc,
  ccb  // conditional contextual-bandit
};

struct rand_state
{
 private:
  uint64_t random_state;

 public:
  constexpr rand_state() : random_state(0) {}
  rand_state(uint64_t initial) : random_state(initial) {}
  constexpr uint64_t get_current_state() const noexcept { return random_state; }
  float get_and_update_random() { return merand48(random_state); }
  float get_random() const { return merand48_noadvance(random_state); }
  void set_random_state(uint64_t initial) noexcept { random_state = initial; }
};

struct vw
{
 private:
  std::shared_ptr<rand_state> _random_state_sp = std::make_shared<rand_state>();  // per instance random_state

 public:
  shared_data* sd;

  parser* p;
  std::thread parse_thread;

  AllReduceType all_reduce_type;
  AllReduce* all_reduce;

  LEARNER::base_learner* l;               // the top level learner
  LEARNER::single_learner* scorer;        // a scoring function
  LEARNER::base_learner* cost_sensitive;  // a cost sensitive learning algorithm.  can be single or multi line learner

  void learn(example&);
  void learn(multi_ex&);
  void predict(example&);
  void predict(multi_ex&);
  void finish_example(example&);
  void finish_example(multi_ex&);

  void (*set_minmax)(shared_data* sd, float label);

  uint64_t current_pass;

  uint32_t num_bits;  // log_2 of the number of features.
  bool default_bits;

  uint32_t hash_seed;

  std::string data_filename;  // was vm["data"]

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

  VW::version_struct model_file_ver;
  double normalized_sum_norm_x;
  bool vw_is_main = false;  // true if vw is executable; false in library mode

  // error reporting
  vw_ostream trace_message;

  // Flag used when VW internally manages lifetime of options object.
  bool should_delete_options = false;
  VW::config::options_i* options;

  void* /*Search::search*/ searchstr;

  uint32_t wpp;

  int stdout_fileno;

  std::vector<std::string> initial_regressors;

  std::string feature_mask;

  std::string per_feature_regularizer_input;
  std::string per_feature_regularizer_output;
  std::string per_feature_regularizer_text;

  float l1_lambda;  // the level of l_1 regularization to impose.
  float l2_lambda;  // the level of l_2 regularization to impose.
  bool no_bias;     // no bias in regularization
  float power_t;    // the power on learning rate decay.
  int reg_mode;

  size_t pass_length;
  size_t numpasses;
  size_t passes_complete;
  uint64_t parse_mask;  // 1 << num_bits -1
  bool permutations;    // if true - permutations of features generated instead of simple combinations. false by default

  // Referenced by examples as their set of interactions. Can be overriden by reductions.
  std::vector<std::string> interactions;
  // TODO #1863 deprecate in favor of only interactions field.
  std::vector<std::string> pairs;  // pairs of features to cross.
  // TODO #1863 deprecate in favor of only interactions field.
  std::vector<std::string> triples;  // triples of features to cross.
  bool ignore_some;
  std::array<bool, NUM_NAMESPACES> ignore;  // a set of namespaces to ignore
  bool ignore_some_linear;
  std::array<bool, NUM_NAMESPACES> ignore_linear;  // a set of namespaces to ignore for linear

  bool redefine_some;                                  // --redefine param was used
  std::array<unsigned char, NUM_NAMESPACES> redefine;  // keeps new chars for namespaces
  std::vector<std::string> ngram_strings;
  std::vector<std::string> skip_strings;
  std::array<uint32_t, NUM_NAMESPACES> ngram;  // ngrams to generate.
  std::array<uint32_t, NUM_NAMESPACES> skips;  // skips in ngrams.
  std::vector<std::string> limit_strings;      // descriptor of feature limits
  std::array<uint32_t, NUM_NAMESPACES> limit;  // count to limit features by
  std::array<uint64_t, NUM_NAMESPACES>
      affix_features;  // affixes to generate (up to 16 per namespace - 4 bits per affix)
  std::array<bool, NUM_NAMESPACES> spelling_features;  // generate spelling features for which namespace
  std::vector<std::string> dictionary_path;            // where to look for dictionaries

  // feature_dict can be created in either loaded_dictionaries or namespace_dictionaries.
  // use shared pointers to avoid the question of ownership
  std::vector<dictionary_info> loaded_dictionaries;  // which dictionaries have we loaded from a file to memory?
  // This array is required to be value initialized so that the std::vectors are constructed.
  std::array<std::vector<std::shared_ptr<feature_dict>>, NUM_NAMESPACES>
      namespace_dictionaries{};  // each namespace has a list of dictionaries attached to it

  void (*delete_prediction)(void*);
  bool audit;     // should I print lots of debugging information?
  bool quiet;     // Should I suppress progress-printing of updates?
  bool training;  // Should I train if lable data is available?
  bool active;
  bool invariant_updates;  // Should we use importance aware/safe updates
  uint64_t random_seed;
  bool random_weights;
  bool random_positive_weights;  // for initialize_regressor w/ new_mf
  bool normal_weights;
  bool tnormal_weights;
  bool add_constant;
  bool nonormalize;
  bool do_reset_source;
  bool holdout_set_off;
  bool early_terminate;
  uint32_t holdout_period;
  uint32_t holdout_after;
  size_t check_holdout_every_n_passes;  // default: 1, but search might want to set it higher if you spend multiple
                                        // passes learning a single policy

  size_t normalized_idx;  // offset idx where the norm is stored (1 or 2 depending on whether adaptive is true)

  uint32_t lda;

  std::string text_regressor_name;
  std::string inv_hash_regressor_name;

  size_t length() { return ((size_t)1) << num_bits; };

  std::stack<LEARNER::base_learner* (*)(VW::config::options_i&, vw&)> reduction_stack;

  // Prediction output
  v_array<int> final_prediction_sink;  // set to send global predictions to.
  int raw_prediction;                  // file descriptors for text output.

  VW_DEPRECATED("print has been deprecated, use print_by_ref")
  void (*print)(int, float, float, v_array<char>);
  void (*print_by_ref)(int, float, float, const v_array<char>&);
  VW_DEPRECATED("print_text has been deprecated, use print_text_by_ref")
  void (*print_text)(int, std::string, v_array<char>);
  void (*print_text_by_ref)(int, const std::string&, const v_array<char>&);
  loss_function* loss;

  char* program_name;

  bool stdin_off;

  // runtime accounting variables.
  float initial_t;
  float eta;  // learning rate control.
  float eta_decay_rate;
  time_t init_time;

  std::string final_regressor_name;

  parameters weights;

  size_t max_examples;  // for TLC

  bool hash_inv;
  bool print_invert;

  // Set by --progress <arg>
  bool progress_add;   // additive (rather than multiplicative) progress dumps
  float progress_arg;  // next update progress dump multiplier

  std::map<std::string, size_t> name_index_map;

  label_type_t label_type;

  vw();
  std::shared_ptr<rand_state> get_random_state() { return _random_state_sp; }

  vw(const vw&) = delete;
  vw& operator=(const vw&) = delete;

  // vw object cannot be moved as many objects hold a pointer to it.
  // That pointer would be invalidated if it were to be moved.
  vw(const vw&&) = delete;
  vw& operator=(const vw&&) = delete;
};

VW_DEPRECATED("Use print_result_by_ref instead")
void print_result(int f, float res, float weight, v_array<char> tag);
void print_result_by_ref(int f, float res, float weight, const v_array<char>& tag);

VW_DEPRECATED("Use binary_print_result_by_ref instead")
void binary_print_result(int f, float res, float weight, v_array<char> tag);
void binary_print_result_by_ref(int f, float res, float weight, const v_array<char>& tag);

void noop_mm(shared_data*, float label);
void get_prediction(int sock, float& res, float& weight);
void compile_gram(
    std::vector<std::string> grams, std::array<uint32_t, NUM_NAMESPACES>& dest, char* descriptor, bool quiet);
void compile_limits(std::vector<std::string> limits, std::array<uint32_t, NUM_NAMESPACES>& dest, bool quiet);

VW_DEPRECATED("Use print_tag_by_ref instead")
int print_tag(std::stringstream& ss, v_array<char> tag);
int print_tag_by_ref(std::stringstream& ss, const v_array<char>& tag);
