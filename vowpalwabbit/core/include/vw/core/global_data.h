// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "vw/allreduce/allreduce_type.h"
#include "vw/common/future_compat.h"
#include "vw/common/string_view.h"
#include "vw/core/array_parameters.h"
#include "vw/core/constant.h"
#include "vw/core/error_reporting.h"
#include "vw/core/input_parser.h"
#include "vw/core/interaction_generation_state.h"
#include "vw/core/metrics_collector.h"
#include "vw/core/multi_ex.h"
#include "vw/core/setup_base.h"
#include "vw/core/version.h"
#include "vw/core/vw_fwd.h"
#include "vw/io/logger.h"

#include <array>
#include <cfloat>
#include <cinttypes>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

// Thread cannot be used in managed C++, tell the compiler that this is unmanaged even if included in a managed project.
#ifdef _M_CEE
#  pragma managed(push, off)
#  undef _M_CEE
#  include <thread>
#  define _M_CEE 001
#  pragma managed(pop)
#else
#  include <thread>
#endif

using vw VW_DEPRECATED("Use VW::workspace instead of ::vw. ::vw will be removed in VW 10.") = VW::workspace;

namespace VW
{
namespace details
{
using feature_dict = std::unordered_map<std::string, std::unique_ptr<VW::features>>;
class dictionary_info
{
public:
  std::string name;
  uint64_t file_hash;
  std::shared_ptr<details::feature_dict> dict;
};
}  // namespace details

using options_deleter_type = void (*)(VW::config::options_i*);
class workspace;

class all_reduce_base;
enum class all_reduce_type;

class default_reduction_stack_setup;
namespace parsers
{
namespace flatbuffer
{
class parser;
}

#ifdef VW_FEAT_CSV_ENABLED
namespace csv
{
class csv_parser;
class csv_parser_options;
}  // namespace csv
#endif
}  // namespace parsers

namespace details
{

class trace_message_wrapper
{
public:
  void* inner_context;
  VW::trace_message_t trace_message;

  trace_message_wrapper(void* context, VW::trace_message_t trace_message)
      : inner_context(context), trace_message(trace_message)
  {
  }
  ~trace_message_wrapper() = default;
};

class invert_hash_info
{
public:
  std::vector<VW::audit_strings> weight_components;
  uint64_t offset;
  uint64_t stride_shift;
};

class feature_tweaks_config
{
public:
  bool add_constant;
  float initial_constant;
  bool permutations;  // if true - permutations of features generated instead of simple combinations. false by default
  // Referenced by examples as their set of interactions. Can be overriden by learners.
  std::vector<std::vector<namespace_index>> interactions;
  std::vector<std::vector<extent_term>> extent_interactions;
  bool ignore_some;
  std::array<bool, NUM_NAMESPACES> ignore;  // a set of namespaces to ignore
  bool ignore_some_linear;
  std::array<bool, NUM_NAMESPACES> ignore_linear;  // a set of namespaces to ignore for linear
  std::unordered_map<std::string, std::set<std::string>>
      ignore_features_dsjson;  // a map from hash(namespace) to a vector of hash(feature). This flag is only available
                               // for dsjson.

  bool redefine_some;                                  // --redefine param was used
  std::array<unsigned char, NUM_NAMESPACES> redefine;  // keeps new chars for namespaces
  std::unique_ptr<VW::kskip_ngram_transformer> skip_gram_transformer;
  std::vector<std::string> limit_strings;      // descriptor of feature limits
  std::array<uint32_t, NUM_NAMESPACES> limit;  // count to limit features by
  std::array<uint64_t, NUM_NAMESPACES>
      affix_features;  // affixes to generate (up to 16 per namespace - 4 bits per affix)
  std::array<bool, NUM_NAMESPACES> spelling_features;  // generate spelling features for which namespace
  std::vector<std::string> dictionary_path;            // where to look for dictionaries

  // feature_dict can be created in either loaded_dictionaries or namespace_dictionaries.
  // use shared pointers to avoid the question of ownership
  std::vector<details::dictionary_info>
      loaded_dictionaries;  // which dictionaries have we loaded from a file to memory?
  // This array is required to be value initialized so that the std::vectors are constructed.
  std::array<std::vector<std::shared_ptr<details::feature_dict>>, NUM_NAMESPACES>
      namespace_dictionaries{};  // each namespace has a list of dictionaries attached to it
};

class output_model_config
{
public:
  std::string final_regressor_name;
  std::string text_regressor_name;
  std::string inv_hash_regressor_name;
  std::string json_weights_file_name;
  bool dump_json_weights_include_feature_names = false;
  bool dump_json_weights_include_extra_online_state = false;
  bool save_resume;
  bool preserve_performance_counters;
  bool save_per_pass;
  std::string per_feature_regularizer_output;
  std::string per_feature_regularizer_text;
};

class passes_config
{
public:
  uint64_t current_pass;
  bool holdout_set_off;
  bool early_terminate;
  uint32_t holdout_period;
  uint32_t holdout_after;
  size_t check_holdout_every_n_passes;  // default: 1, but search might want to set it higher if you spend multiple
                                        // passes learning a single policy
};

class initial_weights_config
{
public:
  uint32_t num_bits;      // log_2 of the number of features.
  size_t normalized_idx;  // offset idx where the norm is stored (1 or 2 depending on whether adaptive is true)
  std::vector<std::string> initial_regressors;
  float initial_weight;
  bool random_weights;
  bool random_positive_weights;  // for initialize_regressor w/ new_mf
  bool normal_weights;
  bool tnormal_weights;
  std::string per_feature_regularizer_input;
};

class update_rule_config
{
public:
  // runtime accounting variables.
  float initial_t;
  float power_t;  // the power on learning rate decay.
  float eta;      // learning rate control.
  float eta_decay_rate;
};

class loss_config
{
public:
  std::unique_ptr<loss_function> loss;
  float l1_lambda;  // the level of l_1 regularization to impose.
  float l2_lambda;  // the level of l_2 regularization to impose.
  bool no_bias;     // no bias in regularization
  int reg_mode;
};

class reduction_state
{
public:
  bool active;
  bool bfgs;
  uint32_t lda;
  // hack to support cb model loading into ccb learner
  bool is_ccb_input_model = false;
  void* /*Search::search*/ searchstr;
  bool invariant_updates;  // Should we use importance aware/safe updates, gd only
  uint32_t total_feature_width;
};

class runtime_config
{
public:
#ifdef VW_FEAT_NETWORKING_ENABLED
  bool daemon;
#endif
  bool vw_is_main = false;  // true if vw is executable; false in library mode
  bool training;            // Should I train if lable data is available?
  size_t pass_length;
  size_t numpasses;
  bool default_bits;
  all_reduce_type selected_all_reduce_type;
  uint32_t hash_seed;
};

class runtime_state
{
public:
  VW::version_struct model_file_ver;
  size_t passes_complete;
  // Default value of 2 follows behavior of 1-indexing and can change to 0-indexing if detected
  uint32_t indexing = 2;  // for 0 or 1 indexing
  // bool nonormalize; not used?
  bool do_reset_source;
  std::unique_ptr<all_reduce_base> all_reduce;
  VW::details::generate_interactions_object_cache generate_interactions_object_cache_state;
  uint64_t parse_mask;  // 1 << num_bits -1
};

class parser_runtime
{
public:
  std::string data_filename;
  std::unique_ptr<parser> example_parser;
  // Experimental field.
  // Generic parser interface to make it possible to use any external parser.
  std::unique_ptr<VW::details::input_parser> custom_parser;
  std::thread parse_thread;
  size_t max_examples;  // for TLC
  bool chain_hash_json = false;
  bool api_status = false;
#ifdef VW_FEAT_FLATBUFFERS_ENABLED
  std::unique_ptr<VW::parsers::flatbuffer::parser> flat_converter;
#endif
};

class output_config
{
public:
  bool quiet;
  bool audit;  // should I print lots of debugging information?
  bool hash_inv;
  bool print_invert;
  bool hexfloat_weights;
};

class output_runtime
{
public:
  // error reporting
  std::shared_ptr<details::trace_message_wrapper> trace_message_wrapper_context;
  std::shared_ptr<std::ostream> trace_message;

  std::unique_ptr<VW::io::writer> stdout_adapter;

  std::map<uint64_t, VW::details::invert_hash_info> index_name_map;
  std::shared_ptr<std::vector<char>> audit_buffer;
  std::unique_ptr<VW::io::writer> audit_writer;
  VW::metrics_collector global_metrics;

  // Prediction output
  std::vector<std::unique_ptr<VW::io::writer>> final_prediction_sink;  // set to send global predictions to.
  std::unique_ptr<VW::io::writer> raw_prediction;                      // file descriptors for text output.
};
}  // namespace details

class workspace
{
public:
  parameters weights;
  std::shared_ptr<VW::LEARNER::learner> l;  // the top level learner
  std::unique_ptr<VW::config::options_i, options_deleter_type> options;
  std::shared_ptr<VW::shared_data> sd;

  void learn(example&);
  void learn(multi_ex&);
  void predict(example&);
  void predict(multi_ex&);
  void finish_example(example&);
  void finish_example(multi_ex&);

  /// This is used to perform finalization steps the driver/cli would normally do.
  /// If using VW in library mode, this call is optional.
  /// Some things this function does are: print summary, finalize regressor, output metrics, etc
  void finish();

  /**
   * @brief Generate a JSON string with the current model state and invert hash
   * lookup table. Bottom learner in use must be gd and workspace.hash_inv must
   * be true. This function is experimental and subject to change.
   *
   * @return std::string JSON formatted string
   */
  std::string dump_weights_to_json_experimental();

  details::feature_tweaks_config feature_tweaks_config;  // feature related configs
  details::initial_weights_config initial_weights_config;
  details::update_rule_config update_rule_config;
  details::loss_config loss_config;
  details::passes_config passes_config;
  details::output_model_config output_model_config;

  details::parser_runtime parser_runtime;
  details::runtime_config runtime_config;
  details::runtime_state runtime_state;
  details::reduction_state reduction_state;

  details::output_config output_config;
  VW::io::logger logger;
  details::output_runtime output_runtime;

  // Function to set min_label and max_label in shared_data
  // Should be bound to a VW::shared_data pointer upon creating the function
  // May be nullptr, so you must check before calling it
  std::function<void(float)> set_minmax;

  std::string id;
  std::string feature_mask;

  size_t length() { return (static_cast<size_t>(1)) << initial_weights_config.num_bits; };

  void (*print_by_ref)(VW::io::writer*, float, float, const v_array<char>&, VW::io::logger&);
  void (*print_text_by_ref)(VW::io::writer*, const std::string&, const v_array<char>&, VW::io::logger&);

  std::shared_ptr<VW::rand_state> get_random_state() { return _random_state_sp; }
  explicit workspace(VW::io::logger logger);

  ~workspace();

  workspace(const VW::workspace&) = delete;
  VW::workspace& operator=(const VW::workspace&) = delete;

  // vw object cannot be moved as many objects hold a pointer to it.
  // That pointer would be invalidated if it were to be moved.
  workspace(const VW::workspace&&) = delete;
  VW::workspace& operator=(const VW::workspace&&) = delete;

private:
  std::shared_ptr<VW::rand_state> _random_state_sp;  // per instance random_state
};

namespace details
{
void print_result_by_ref(
    VW::io::writer* f, float res, float weight, const VW::v_array<char>& tag, VW::io::logger& logger);

void compile_limits(std::vector<std::string> limits, std::array<uint32_t, VW::NUM_NAMESPACES>& dest, bool quiet,
    VW::io::logger& logger);
}  // namespace details
}  // namespace VW

using reduction_setup_fn VW_DEPRECATED("") = VW::reduction_setup_fn;
using options_deleter_type VW_DEPRECATED("") = VW::options_deleter_type;
