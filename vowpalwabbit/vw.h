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

#include "compat.h"
#include "global_data.h"
#include "hashstring.h"
#include "parser.h"
#include "setup_base.h"
#include "vw/common/hash.h"
#include "vw_fwd.h"

#include <memory>

namespace VW
{
/*    Caveats:
    (1) Some commandline parameters do not make sense as a library.
    (2) The code is not yet reentrant.
   */
VW::workspace* initialize(std::unique_ptr<config::options_i, options_deleter_type> options, io_buf* model = nullptr,
    bool skip_model_load = false, trace_message_t trace_listener = nullptr, void* trace_context = nullptr);
VW::workspace* initialize(config::options_i& options, io_buf* model = nullptr, bool skip_model_load = false,
    trace_message_t trace_listener = nullptr, void* trace_context = nullptr);
VW::workspace* initialize(const std::string& s, io_buf* model = nullptr, bool skip_model_load = false,
    trace_message_t trace_listener = nullptr, void* trace_context = nullptr);
VW::workspace* initialize(int argc, char* argv[], io_buf* model = nullptr, bool skip_model_load = false,
    trace_message_t trace_listener = nullptr, void* trace_context = nullptr);

VW::workspace* seed_vw_model(VW::workspace* vw_model, const std::string& extra_args,
    trace_message_t trace_listener = nullptr, void* trace_context = nullptr);
// Allows the input command line string to have spaces escaped by '\'
VW::workspace* initialize_escaped(std::string const& s, io_buf* model = nullptr, bool skip_model_load = false,
    trace_message_t trace_listener = nullptr, void* trace_context = nullptr);
// Experimental (VW::setup_base_i):
VW::workspace* initialize_with_builder(const std::string& s, io_buf* model = nullptr, bool skipModelLoad = false,
    trace_message_t trace_listener = nullptr, void* trace_context = nullptr,
    std::unique_ptr<VW::setup_base_i> = nullptr);

using driver_output_func_t = void (*)(void*, const std::string&);
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
 * @param logger_output_func optional function to forward logger ouput to
 * @param logger_output_func_context context for logger_output_func
 * @param setup_base optional advanced override of reduction stack
 * @return std::unique_ptr<VW::workspace> initialized workspace
 */
std::unique_ptr<VW::workspace> initialize_experimental(std::unique_ptr<config::options_i> options,
    std::unique_ptr<VW::io::reader> model_override_reader = nullptr, driver_output_func_t driver_output_func = nullptr,
    void* driver_output_func_context = nullptr, VW::io::logger_output_func_t logger_output_func = nullptr,
    void* logger_output_func_context = nullptr, std::unique_ptr<VW::setup_base_i> setup_base = nullptr);
VW_WARNING_STATE_POP

void cmd_string_replace_value(std::stringstream*& ss, std::string flag_to_replace, const std::string& new_value);

// The argv array from both of these functions must be freed.
char** to_argv(std::string const& s, int& argc);
char** to_argv_escaped(std::string const& s, int& argc);
void free_args(int argc, char* argv[]);

const char* are_features_compatible(VW::workspace& vw1, VW::workspace& vw2);

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
void finish(VW::workspace& all, bool delete_all = true);
VW_WARNING_STATE_POP
void sync_stats(VW::workspace& all);

void start_parser(VW::workspace& all);
void end_parser(VW::workspace& all);
bool is_ring_example(const VW::workspace& all, const example* ae);

struct primitive_feature_space  // just a helper definition.
{
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
example* alloc_examples(size_t count);
void dealloc_examples(example* example_ptr, size_t count);

void parse_example_label(VW::workspace& all, example& ec, const std::string& label);
void setup_examples(VW::workspace& all, v_array<example*>& examples);
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

void add_constant_feature(VW::workspace& all, example* ec);
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
void releaseFeatureSpace(primitive_feature_space* features, size_t len);

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
  return getHasher(hash)(s.data(), s.length(), 0);
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
  return getHasher(h)(s.data(), s.length(), u) & parse_mark;
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
