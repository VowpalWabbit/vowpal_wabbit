// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#ifdef _WIN32
#ifdef LEAKCHECK
// Visual Leak Detector for memory leak detection on Windows
#include <vld.h>
#endif
#endif

#include "global_data.h"
#include "example.h"
#include "hash.h"
#include "simple_label.h"
#include "parser.h"
#include "parse_example.h"

#include "options.h"

namespace VW
{
/*    Caveats:
    (1) Some commandline parameters do not make sense as a library.
    (2) The code is not yet reentrant.
   */
vw* initialize(config::options_i& options, io_buf* model = nullptr, bool skipModelLoad = false,
    trace_message_t trace_listener = nullptr, void* trace_context = nullptr);
vw* initialize(std::string s, io_buf* model = nullptr, bool skipModelLoad = false,
    trace_message_t trace_listener = nullptr, void* trace_context = nullptr);
vw* initialize(int argc, char* argv[], io_buf* model = nullptr, bool skipModelLoad = false,
    trace_message_t trace_listener = nullptr, void* trace_context = nullptr);
vw* seed_vw_model(
    vw* vw_model, std::string extra_args, trace_message_t trace_listener = nullptr, void* trace_context = nullptr);
// Allows the input command line string to have spaces escaped by '\'
vw* initialize_escaped(std::string const& s, io_buf* model = nullptr, bool skipModelLoad = false,
    trace_message_t trace_listener = nullptr, void* trace_context = nullptr);

void cmd_string_replace_value(std::stringstream*& ss, std::string flag_to_replace, std::string new_value);

VW_DEPRECATED("By value version is deprecated, pass std::string by const ref instead using `to_argv`")
char** get_argv_from_string(std::string s, int& argc);

// The argv array from both of these functions must be freed.
char** to_argv(std::string const& s, int& argc);
char** to_argv_escaped(std::string const& s, int& argc);
void free_args(int argc, char* argv[]);

const char* are_features_compatible(vw& vw1, vw& vw2);

/*
  Call finish() after you are done with the vw instance.  This cleans up memory usage.
 */
void finish(vw& all, bool delete_all = true);
void sync_stats(vw& all);

void start_parser(vw& all);
void end_parser(vw& all);
bool is_ring_example(vw& all, example* ae);

struct primitive_feature_space  // just a helper definition.
{
  unsigned char name;
  feature* fs;
  size_t len;
};

// The next commands deal with creating examples.  Caution: VW does not all allow creation of many examples at once by
// default.  You can adjust the exact number by tweaking ring_size.

/* The simplest of two ways to create an example.  An example_line is the literal line in a VW-format datafile.
 */
example* read_example(vw& all, char* example_line);
example* read_example(vw& all, std::string example_line);

// The more complex way to create an example.

// after you create and fill feature_spaces, get an example with everything filled in.
example* import_example(vw& all, const std::string& label, primitive_feature_space* features, size_t len);

// callers must free memory using release_example
// this interface must be used with care as finish_example is a no-op for these examples.
// thus any delay introduced when freeing examples must be at least as long as the one
// introduced by all.l->finish_example implementations.
// e.g. multiline examples as used by cb_adf must not be released before the finishing newline example.
example* alloc_examples(size_t, size_t);
void dealloc_example(void (*delete_label)(void*), example& ec, void (*delete_prediction)(void*) = nullptr);

void parse_example_label(vw& all, example& ec, std::string label);
void setup_examples(vw& all, v_array<example*>& examples);
void setup_example(vw& all, example* ae);
example* new_unused_example(vw& all);
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
feature* get_features(vw& all, example* ec, size_t& feature_number);
void return_features(feature* f);

void add_constant_feature(vw& all, example* ec);
void add_label(example* ec, float label, float weight = 1, float base = 0);

// notify VW that you are done with the example.
void finish_example(vw& all, example& ec);
void finish_example(vw& all, multi_ex& ec);
void empty_example(vw& all, example& ec);

void copy_example_data(bool audit, example*, example*, size_t, void (*copy_label)(void*, void*));
void copy_example_metadata(bool audit, example*, example*);
void copy_example_data(bool audit, example*, example*);  // metadata + features, don't copy the label
void clear_example_data(example&);                       // don't clear the label
void move_feature_namespace(example* dst, example* src, namespace_index c);

// after export_example, must call releaseFeatureSpace to free native memory
primitive_feature_space* export_example(vw& all, example* e, size_t& len);
void releaseFeatureSpace(primitive_feature_space* features, size_t len);

void save_predictor(vw& all, std::string reg_name);
void save_predictor(vw& all, io_buf& buf);

// inlines

// First create the hash of a namespace.
inline uint64_t hash_space(vw& all, const std::string& s)
{
  return all.p->hasher(s.data(), s.length(), all.hash_seed);
}
inline uint64_t hash_space_static(const std::string& s, const std::string& hash)
{
  return getHasher(hash)(s.data(), s.length(), 0);
}
inline uint64_t hash_space_cstr(vw& all, const char* fstr)
{
  return all.p->hasher(fstr, strlen(fstr), all.hash_seed);
}
// Then use it as the seed for hashing features.
inline uint64_t hash_feature(vw& all, const std::string& s, uint64_t u)
{
  return all.p->hasher(s.data(), s.length(), u) & all.parse_mask;
}
inline uint64_t hash_feature_static(const std::string& s, uint64_t u, const std::string& h, uint32_t num_bits)
{
  size_t parse_mark = (1 << num_bits) - 1;
  return getHasher(h)(s.data(), s.length(), u) & parse_mark;
}

inline uint64_t hash_feature_cstr(vw& all, char* fstr, uint64_t u)
{
  return all.p->hasher(fstr, strlen(fstr), u) & all.parse_mask;
}

inline float get_weight(vw& all, uint32_t index, uint32_t offset)
{
  return (&all.weights[((uint64_t)index) << all.weights.stride_shift()])[offset];
}

inline void set_weight(vw& all, uint32_t index, uint32_t offset, float value)
{
  (&all.weights[((uint64_t)index) << all.weights.stride_shift()])[offset] = value;
}

inline uint32_t num_weights(vw& all) { return (uint32_t)all.length(); }

inline uint32_t get_stride(vw& all) { return all.weights.stride(); }

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
