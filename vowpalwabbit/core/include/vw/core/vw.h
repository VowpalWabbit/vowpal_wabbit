// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

/*! \mainpage
 *
 * For the primary interface see:
 * - \link VW VW namespace documentation \endlink
 *
 * For other docs see:
 * - [Project website](https://vowpalwabbit.org)
 * - [Wiki](https://github.com/VowpalWabbit/vowpal_wabbit/wiki)
 * - C++ build instructions:
 *     - [Install dependencies](https://github.com/VowpalWabbit/vowpal_wabbit/wiki/Dependencies)
 *     - [Build](https://github.com/VowpalWabbit/vowpal_wabbit/wiki/Building)
 *     - [Install](https://github.com/VowpalWabbit/vowpal_wabbit/wiki/Installing)
 * - [Install other languages](https://vowpalwabbit.org/start.html)
 * - [Tutorials](https://github.com/VowpalWabbit/vowpal_wabbit/wiki/Tutorial)
 */

#ifdef _WIN32
#  ifdef LEAKCHECK
// Visual Leak Detector for memory leak detection on Windows
#    include <vld.h>
#  endif
#endif

#include "vw/common/future_compat.h"
#include "vw/common/hash.h"
#include "vw/core/error_reporting.h"
#include "vw/core/global_data.h"
#include "vw/core/hashstring.h"
#include "vw/core/parser.h"
#include "vw/core/setup_base.h"
#include "vw/core/vw_fwd.h"

#include <memory>

namespace VW
{
using driver_output_func_t = VW::trace_message_t;

/*    Caveats:
    (1) Some commandline parameters do not make sense as a library.
    (2) The code is not yet reentrant.
   */

// TODO: uncomment when all uses are migrated
VW_DEPRECATED("Replaced with new unique_ptr based overload.")
VW::workspace* initialize(std::unique_ptr<config::options_i, options_deleter_type> options, io_buf* model = nullptr,
    bool skip_model_load = false, trace_message_t trace_listener = nullptr, void* trace_context = nullptr);

// TODO: uncomment when all uses are migrated
VW_DEPRECATED("Replaced with new unique_ptr based overload.")
VW::workspace* initialize(config::options_i& options, io_buf* model = nullptr, bool skip_model_load = false,
    trace_message_t trace_listener = nullptr, void* trace_context = nullptr);

// TODO: uncomment when all uses are migrated
VW_DEPRECATED("Replaced with new unique_ptr based overload.")
VW::workspace* initialize(const std::string& s, io_buf* model = nullptr, bool skip_model_load = false,
    trace_message_t trace_listener = nullptr, void* trace_context = nullptr);

// TODO: uncomment when all uses are migrated
VW_DEPRECATED("Replaced with new unique_ptr based overload.")
VW::workspace* initialize(int argc, char* argv[], io_buf* model = nullptr, bool skip_model_load = false,
    trace_message_t trace_listener = nullptr, void* trace_context = nullptr);

// TODO: uncomment when all uses are migrated
VW_DEPRECATED("Replaced with new unique_ptr based overload.")
VW::workspace* seed_vw_model(VW::workspace* vw_model, const std::string& extra_args,
    trace_message_t trace_listener = nullptr, void* trace_context = nullptr);
// Allows the input command line string to have spaces escaped by '\'

// TODO: uncomment when all uses are migrated
VW_DEPRECATED("Replaced with new unique_ptr based overload.")
VW::workspace* initialize_escaped(std::string const& s, io_buf* model = nullptr, bool skip_model_load = false,
    trace_message_t trace_listener = nullptr, void* trace_context = nullptr);

// Experimental (VW::setup_base_i):
VW_DEPRECATED("For scenarios requiring the builder, initialize_experimental should be used.")
VW::workspace* initialize_with_builder(const std::string& s, io_buf* model = nullptr, bool skip_model_load = false,
    trace_message_t trace_listener = nullptr, void* trace_context = nullptr,
    std::unique_ptr<VW::setup_base_i> = nullptr);

/**
 * @brief Initialize a workspace.
 *
 * ## Examples
 *
 * To intialize a workspace with specific arguments.
 * \code
 * auto vw = VW::initialize(VW::make_unique<VW::config::options_cli>(
 *    std::vector<std::string>{"--cb_explore_adf", "--epsilon=0.1", "--quadratic=::"}));
 * \endcode
 *
 * To initialize a workspace with a string that needs to be split.
 * VW::split_command_line() can be used to split the string similar to how a
 * shell would
 * \code
 * auto all = VW::initialize(VW::make_unique<VW::config::options_cli>(
 *   VW::split_command_line("--cb_explore_adf --epsilon=0.1 --quadratic=::")));
 * \endcode
 *
 * **Note:** You used to need to call VW::finish() to free the workspace. This is no
 * longer needed and the destructor will free the workspace. However,
 * VW::finish() would also do driver finalization steps, such as writing the output
 * model. This is not often needed in library mode but can be run using
 * VW::workspace::finish().
 *
 * @param options The options to initialize the workspace with. Usually an
 * instance of VW::config::options_cli.
 * @param model_override_reader optional reading source to read the model from.
 * Will override any model specified on the command line.
 * @param driver_output_func optional function to forward driver ouput to
 * @param driver_output_func_context context for driver_output_func
 * @param custom_logger optional custom logger object to override with
 * @param setup_base optional advanced override of reduction stack
 * @return std::unique_ptr<VW::workspace> initialized workspace
 */
std::unique_ptr<VW::workspace> initialize(std::unique_ptr<config::options_i> options,
    std::unique_ptr<VW::io::reader> model_override_reader = nullptr, driver_output_func_t driver_output_func = nullptr,
    void* driver_output_func_context = nullptr, VW::io::logger* custom_logger = nullptr);

/// Creates a workspace based off of another workspace. What this means is that
/// the model weights and the shared_data object are shared. This function needs
/// to be used with caution. Reduction data is not shared, therefore this
/// function is unsafe to use for situations where reduction state is required
/// for proper operation such as marginal and cb_adf. Learn on a seeded instance
/// is unsafe, and prediction is also potentially unsafe.
std::unique_ptr<VW::workspace> seed_vw_model(VW::workspace& vw_model, const std::vector<std::string>& extra_args,
    driver_output_func_t driver_output_func = nullptr, void* driver_output_func_context = nullptr,
    VW::io::logger* custom_logger = nullptr);

VW_WARNING_STATE_PUSH
VW_WARNING_DISABLE_BADLY_FORMED_XML
/**
 * @brief Initialize a workspace. This interface is currently experimental, but
 * will replace the existing array of initialize functions.
 *
 * @param options The options to initialize the workspace with. Usually an
 * instance of VW::config::options_cli.
 * @param model_override_reader optional reading source to read the model from.
 * Will override any model specified on the command line.
 * @param driver_output_func optional function to forward driver ouput to
 * @param driver_output_func_context context for driver_output_func
 * @param custom_logger optional custom logger object to override with
 * @param setup_base optional advanced override of reduction stack
 * @return std::unique_ptr<VW::workspace> initialized workspace
 */
std::unique_ptr<VW::workspace> initialize_experimental(std::unique_ptr<config::options_i> options,
    std::unique_ptr<VW::io::reader> model_override_reader = nullptr, driver_output_func_t driver_output_func = nullptr,
    void* driver_output_func_context = nullptr, VW::io::logger* custom_logger = nullptr,
    std::unique_ptr<VW::setup_base_i> setup_base = nullptr);
VW_WARNING_STATE_POP

VW_DEPRECATED(
    "VW no longer supports manipulating a command line with cmd_string_replace_value. This function will be removed in "
    "VW 10.")
void cmd_string_replace_value(std::stringstream*& ss, std::string flag_to_replace, const std::string& new_value);

// The argv array from both of these functions must be freed.
VW_DEPRECATED(
    "This functionality is now implemented by VW::split_command_line which supports escaping, etc. This function will "
    "be removed in VW 10.")
char** to_argv(std::string const& s, int& argc);
VW_DEPRECATED(
    "This functionality is now implemented by VW::split_command_line which supports escaping, etc. This function will "
    "be removed in VW 10.")
char** to_argv_escaped(std::string const& s, int& argc);
VW_DEPRECATED("This function will be removed in VW 10.")
void free_args(int argc, char* argv[]);

const char* are_features_compatible(const VW::workspace& vw1, const VW::workspace& vw2);

VW_WARNING_STATE_PUSH
VW_WARNING_DISABLE_BADLY_FORMED_XML
/**
 * @brief Call finish() after you are done with the vw instance. This cleans up memory usage if delete_all is true.
 * Finish will cause final stat printouts and model serialization to occur. IMPORTANT: If lifetime is managed by a
 * unique_ptr from initialize_experimental, then you must call this with delete_all = false
 *
 * @param all workspace to be finished
 * @param delete_all whethere to also also call delete on this instance.
 */
VW_DEPRECATED(
    "If needing to cleanup memory, rely on the workspace destructor. Driver finalization is now handled by "
    "VW::workspace::finish().")
void finish(VW::workspace& all, bool delete_all = true);

VW_WARNING_STATE_POP
void sync_stats(VW::workspace& all);

void start_parser(VW::workspace& all);
void end_parser(VW::workspace& all);

VW_DEPRECATED(
    "It is no longer supported to query whether an example is a ring example. This function will be removed in VW 10")
bool is_ring_example(const VW::workspace& all, const example* ae);

class primitive_feature_space  // just a helper definition.
{
public:
  unsigned char name;
  feature* fs;
  size_t len;
};

// The next commands deal with creating examples.
/* The simplest of two ways to create an example.  An example_line is the literal line in a VW-format datafile.
 */
example* read_example(VW::workspace& all, const char* example_line);
example* read_example(VW::workspace& all, const std::string& example_line);

// The more complex way to create an example.

// after you create and fill feature_spaces, get an example with everything filled in.
example* import_example(VW::workspace& all, const std::string& label, primitive_feature_space* features, size_t len);

// callers must free memory using dealloc_examples
// this interface must be used with care as finish_example is a no-op for these examples.
// thus any delay introduced when freeing examples must be at least as long as the one
// introduced by all.l->finish_example implementations.
// e.g. multiline examples as used by cb_adf must not be released before the finishing newline example.
VW_DEPRECATED(
    "This function is no longer needed and will be removed. Use new/make_unique/make_shared or stack based as "
    "appropriate.")
example* alloc_examples(size_t count);
VW_DEPRECATED("This function is no longer needed and will be removed.")
void dealloc_examples(example* example_ptr, size_t count);

void parse_example_label(VW::workspace& all, example& ec, const std::string& label);
void setup_examples(VW::workspace& all, VW::multi_ex& examples);
void setup_example(VW::workspace& all, example* ae);
example* new_unused_example(VW::workspace& all);
example* get_example(parser* pf);
float get_topic_prediction(example* ec, size_t i);  // i=0 to max topic -1
float get_label(example* ec);
float get_importance(example* ec);
float get_initial(example* ec);
float get_prediction(example* ec);
float get_cost_sensitive_prediction(example* ec);
v_array<float>& get_cost_sensitive_prediction_confidence_scores(example* ec);
uint32_t* get_multilabel_predictions(example* ec, size_t& len);
float get_action_score(example* ec, size_t i);
size_t get_action_score_length(example* ec);
size_t get_tag_length(example* ec);
const char* get_tag(example* ec);
size_t get_feature_number(example* ec);
float get_confidence(example* ec);
feature* get_features(VW::workspace& all, example* ec, size_t& feature_number);
void return_features(feature* f);

void add_constant_feature(const VW::workspace& all, example* ec);
void add_label(example* ec, float label, float weight = 1, float base = 0);

// notify VW that you are done with the example.
void finish_example(VW::workspace& all, example& ec);
void finish_example(VW::workspace& all, multi_ex& ec);
void empty_example(VW::workspace& all, example& ec);

void move_feature_namespace(example* dst, example* src, namespace_index c);

void copy_example_metadata(example*, const example*);
void copy_example_data(example*, const example*);  // metadata + features, don't copy the label
void copy_example_data_with_label(example* dst, const example* src);

// after export_example, must call releaseFeatureSpace to free native memory
primitive_feature_space* export_example(VW::workspace& all, example* e, size_t& len);
void release_feature_space(primitive_feature_space* features, size_t len);
VW_DEPRECATED("VW::releaseFeatureSpace renamed to VW::release_feature_space")
inline void releaseFeatureSpace(primitive_feature_space* features, size_t len)  // NOLINT
{
  release_feature_space(features, len);
}

void save_predictor(VW::workspace& all, const std::string& reg_name);
void save_predictor(VW::workspace& all, io_buf& buf);

// inlines

// First create the hash of a namespace.
inline uint64_t hash_space(VW::workspace& all, const std::string& s)
{
  return all.example_parser->hasher(s.data(), s.length(), all.hash_seed);
}
inline uint64_t hash_space_static(const std::string& s, const std::string& hash)
{
  return get_hasher(hash)(s.data(), s.length(), 0);
}
inline uint64_t hash_space_cstr(VW::workspace& all, const char* fstr)
{
  return all.example_parser->hasher(fstr, strlen(fstr), all.hash_seed);
}
// Then use it as the seed for hashing features.
inline uint64_t hash_feature(VW::workspace& all, const std::string& s, uint64_t u)
{
  return all.example_parser->hasher(s.data(), s.length(), u) & all.parse_mask;
}
inline uint64_t hash_feature_static(const std::string& s, uint64_t u, const std::string& h, uint32_t num_bits)
{
  size_t parse_mark = (1 << num_bits) - 1;
  return get_hasher(h)(s.data(), s.length(), u) & parse_mark;
}

inline uint64_t hash_feature_cstr(VW::workspace& all, const char* fstr, uint64_t u)
{
  return all.example_parser->hasher(fstr, strlen(fstr), u) & all.parse_mask;
}

inline uint64_t chain_hash(VW::workspace& all, const std::string& name, const std::string& value, uint64_t u)
{
  // chain hash is hash(feature_value, hash(feature_name, namespace_hash)) & parse_mask
  return all.example_parser->hasher(
             value.data(), value.length(), all.example_parser->hasher(name.data(), name.length(), u)) &
      all.parse_mask;
}

inline uint64_t chain_hash_static(
    const std::string& name, const std::string& value, uint64_t u, hash_func_t hash_func, uint64_t parse_mask)
{
  // chain hash is hash(feature_value, hash(feature_name, namespace_hash)) & parse_mask
  return hash_func(value.data(), value.length(), hash_func(name.data(), name.length(), u)) & parse_mask;
}

inline float get_weight(VW::workspace& all, uint32_t index, uint32_t offset)
{
  return (&all.weights[static_cast<uint64_t>(index) << all.weights.stride_shift()])[offset];
}

inline void set_weight(VW::workspace& all, uint32_t index, uint32_t offset, float value)
{
  (&all.weights[static_cast<uint64_t>(index) << all.weights.stride_shift()])[offset] = value;
}

inline uint32_t num_weights(VW::workspace& all) { return static_cast<uint32_t>(all.length()); }

inline uint32_t get_stride(VW::workspace& all) { return all.weights.stride(); }

inline void init_features(primitive_feature_space& fs, size_t features_count)
{
  fs.fs = new feature[features_count];
  fs.len = features_count;
}

inline void set_feature(primitive_feature_space& fs, size_t index, uint64_t feature_hash, float value)
{
  fs.fs[index].weight_index = feature_hash;
  fs.fs[index].x = value;
}
}  // namespace VW
