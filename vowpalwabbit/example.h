// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "v_array.h"
#include "no_label.h"
#include "simple_label.h"
#include "multiclass.h"
#include "multilabel.h"
#include "cost_sensitive.h"
#include "cb.h"
#include "constant.h"
#include "feature_group.h"
#include "action_score.h"
#include "example_predict.h"
#include "conditional_contextual_bandit.h"
#include "continuous_actions_reduction_features.h"
#include "ccb_label.h"
#include "slates_label.h"
#include "decision_scores.h"
#include "cb_continuous_label.h"
#include "prob_dist_cont.h"
#include "active_multiclass_prediction.h"

#include <atomic>
#include <cstdint>
#include <vector>
#include <iostream>
#include <condition_variable>


struct polylabel
{
  no_label::no_label empty;
  label_data simple;
  MULTICLASS::label_t multi;
  COST_SENSITIVE::label cs;
  CB::label cb;
  VW::cb_continuous::continuous_label cb_cont;
  CCB::label conditional_contextual_bandit;
  VW::slates::label slates;
  CB_EVAL::label cb_eval;
  MULTILABEL::labels multilabels;
};

struct polyprediction
{
  polyprediction() = default;
  ~polyprediction() = default;

  polyprediction(polyprediction&&) = default;
  polyprediction& operator=(polyprediction&&) = default;

  polyprediction(const polyprediction&) = delete;
  polyprediction& operator=(const polyprediction&) = delete;

  float scalar = 0.f;
  v_array<float> scalars;           // a sequence of scalar predictions
  ACTION_SCORE::action_scores a_s;  // a sequence of classes with scores.  Also used for probabilities.
  VW::decision_scores_t decision_scores;
  uint32_t multiclass;
  MULTILABEL::labels multilabels;
  float prob = 0.f;                                          // for --probabilities --csoaa_ldf=mc
  VW::continuous_actions::probability_density_function pdf;  // probability density defined over an action range
  VW::continuous_actions::probability_density_function_value pdf_value;  // probability density value for a given action
  VW::active_multiclass_prediction active_multiclass;
};

struct example_lock
{
  // NT set to false!!!
  //std::atomic<bool> done_parsing; // flag used in multithreaded parsing to indicate that the example is done being parsed
  std::atomic<bool>* done_parsing;
  //for get_example
  std::condition_variable* example_parsed;
  //for cv notify and wait
  std::mutex* example_cv_mutex;

  example_lock()
  : done_parsing(new std::atomic<bool>(false))
  , example_parsed(new std::condition_variable)
  , example_cv_mutex(new std::mutex)
  
  {}
  ~example_lock()
  {
    delete done_parsing;
    delete example_parsed;
    delete example_cv_mutex;
  }
  
  example_lock(const example_lock&) = delete;
  example_lock& operator=(const example_lock&) = delete;
  example_lock(example_lock&& other)
  {
    done_parsing = other.done_parsing;
    example_parsed = other.example_parsed;
    example_cv_mutex = other.example_cv_mutex;
    other.done_parsing = nullptr;
    other.example_parsed = nullptr;
    other.example_cv_mutex = nullptr;
  }
  example_lock& operator=(example_lock&& other)
  {
    done_parsing = other.done_parsing;
    example_parsed = other.example_parsed;
    example_cv_mutex = other.example_cv_mutex;
    other.done_parsing = nullptr;
    other.example_parsed = nullptr;
    other.example_cv_mutex = nullptr;
    return *this;
  }
};

VW_WARNING_STATE_PUSH
VW_WARNING_DISABLE_DEPRECATED_USAGE
struct example : public example_predict  // core example datatype.
{
  example() = default;
  ~example();

  example(const example&) = delete;
  example& operator=(const example&) = delete;
  example(example&& other) = default;
  example& operator=(example&& other) = default;

  // input fields
  polylabel l;

  // output prediction
  polyprediction pred;

  float weight = 1.f;  // a relative importance weight for the example, default = 1
  v_array<char> tag;   // An identifier for the example.
  size_t example_counter = 0;

  // helpers
  size_t num_features = 0;         // precomputed, cause it's fast&easy.
  float partial_prediction = 0.f;  // shared data for prediction.
  float updated_prediction = 0.f;  // estimated post-update prediction.
  float loss = 0.f;
  float total_sum_feat_sq = 0.f;  // precomputed, cause it's kind of fast & easy.
  float confidence = 0.f;
  features* passthrough =
      nullptr;  // if a higher-up reduction wants access to internal state of lower-down reductions, they go here

  bool test_only = false;
  bool end_pass = false;  // special example indicating end of pass.
  bool sorted = false;    // Are the features sorted or not?
  bool is_newline = false;

  // Deprecating a field can make deprecated warnings hard to track down through implicit usage in the constructor.
  // This is deprecated, but we won't mark it so we don't have those issues.
  // VW_DEPRECATED(
  //     "in_use has been removed, examples taken from the pool are assumed to be in use if there is a reference to
  //     them. " "Standalone examples are by definition always in use.")
  bool in_use = true;

  // // This object is only used because atomic signaling is not available in C++11
  // example_lock ex_lock;
};
VW_WARNING_STATE_POP

struct example_vector  // core example datatype.
{
  example_vector() = default;
  ~example_vector();

  example_vector(const example_vector&) = delete;
  example_vector& operator=(const example_vector&) = delete;
  example_vector(example_vector&& other) = default;
  example_vector& operator=(example_vector&& other) = default;

  v_array<example*> ev;

  void push_back(example* ex){
    ev.push_back(ex);
  }

  size_t size(){
    return ev.size();
  }

  // This object is only used because atomic signaling is not available in C++11
  example_lock ev_lock;
};

struct vw;

struct flat_example
{
  polylabel l;
  reduction_features _reduction_features;

  size_t tag_len;
  char* tag;  // An identifier for the example.

  size_t example_counter;
  uint64_t ft_offset;
  float global_weight;

  size_t num_features;      // precomputed, cause it's fast&easy.
  float total_sum_feat_sq;  // precomputed, cause it's kind of fast & easy.
  features fs;              // all the features
};

flat_example* flatten_example(vw& all, example* ec);
flat_example* flatten_sort_example(vw& all, example* ec);
void free_flatten_example(flat_example* fec);

inline int example_is_newline(example const& ec) { return ec.is_newline; }

inline bool valid_ns(char c) { return !(c == '|' || c == ':'); }

inline void add_passthrough_feature_magic(example& ec, uint64_t magic, uint64_t i, float x)
{
  if (ec.passthrough) ec.passthrough->push_back(x, (FNV_prime * magic) ^ i);
}

#define add_passthrough_feature(ec, i, x) \
  add_passthrough_feature_magic(ec, __FILE__[0] * 483901 + __FILE__[1] * 3417 + __FILE__[2] * 8490177, i, x);

typedef std::vector<example*> multi_ex;

namespace VW
{
void return_multiple_example(vw& all, v_array<example*>& examples);

typedef example& (*example_factory_t)(void*);

}  // namespace VW

std::string simple_label_to_string(const example& ec);
std::string cb_label_to_string(const example& ec);
std::string scalar_pred_to_string(const example& ec);
std::string a_s_pred_to_string(const example& ec);
std::string prob_dist_pred_to_string(const example& ec);
std::string multiclass_pred_to_string(const example& ec);
std::string debug_depth_indent_string(const multi_ex& ec);
std::string debug_depth_indent_string(const example& ec);
std::string debug_depth_indent_string(int32_t stack_depth);
std::string cb_label_to_string(const example& ec);
