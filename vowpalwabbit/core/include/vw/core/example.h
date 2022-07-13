// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "active_multiclass_prediction.h"
#include "ccb_label.h"
#include "decision_scores.h"
#include "simple_label.h"
#include "vw/core/action_score.h"
#include "vw/core/cache.h"
#include "vw/core/cb.h"
#include "vw/core/cb_continuous_label.h"
#include "vw/core/constant.h"
#include "vw/core/cost_sensitive.h"
#include "vw/core/example_predict.h"
#include "vw/core/feature_group.h"
#include "vw/core/multiclass.h"
#include "vw/core/multilabel.h"
#include "vw/core/no_label.h"
#include "vw/core/prob_dist_cont.h"
#include "vw/core/slates_label.h"
#include "vw/core/v_array.h"

#include <cstdint>
#include <vector>

namespace VW
{
struct workspace;
}
namespace VW
{
void copy_example_data(example* dst, const example* src);
void setup_example(VW::workspace& all, example* ae);

struct polylabel
{
  no_label::no_label empty = static_cast<char>(0);
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
  VW::v_array<float> scalars;       // a sequence of scalar predictions
  ACTION_SCORE::action_scores a_s;  // a sequence of classes with scores.  Also used for probabilities.
  VW::decision_scores_t decision_scores;
  uint32_t multiclass = 0;
  MULTILABEL::labels multilabels;
  float prob = 0.f;                                          // for --probabilities --csoaa_ldf=mc
  VW::continuous_actions::probability_density_function pdf;  // probability density defined over an action range
  VW::continuous_actions::probability_density_function_value pdf_value;  // probability density value for a given action
  VW::active_multiclass_prediction active_multiclass;
  char nopred = static_cast<char>(0);
};

std::string to_string(const v_array<float>& scalars, int decimal_precision = DEFAULT_FLOAT_PRECISION);

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

  float weight = 1.f;     // a relative importance weight for the example, default = 1
  VW::v_array<char> tag;  // An identifier for the example.
  size_t example_counter = 0;

  // helpers
  size_t num_features = 0;  // precomputed, cause it's fast&easy.
  size_t num_features_from_interactions = 0;
  float partial_prediction = 0.f;  // shared data for prediction.
  float updated_prediction = 0.f;  // estimated post-update prediction.
  float loss = 0.f;

  // This value is only used for gd's sensitivity call, but it is costly to
  // calculate. Therefore it is calculated only when needed. Anything that
  // modifies the feature_groups in this example should invalidate this value
  // with reset_total_sum_feat_sq() to ensure it gets recalculated if needed.
  float total_sum_feat_sq = 0.f;
  float confidence = 0.f;
  features* passthrough =
      nullptr;  // if a higher-up reduction wants access to internal state of lower-down reductions, they go here

  bool test_only = false;
  bool end_pass = false;  // special example indicating end of pass.
  bool sorted = false;    // Are the features sorted or not?
  bool is_newline = false;

  size_t get_num_features() const noexcept { return num_features + num_features_from_interactions; }

  float get_total_sum_feat_sq();

  void reset_total_sum_feat_sq()
  {
    total_sum_feat_sq = 0.f;
    total_sum_feat_sq_calculated = false;
  }

  friend void VW::copy_example_data(example* dst, const example* src);
  friend void VW::setup_example(VW::workspace& all, example* ae);

private:
  bool total_sum_feat_sq_calculated = false;
  bool use_permutations = false;
};

struct workspace;

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

flat_example* flatten_example(VW::workspace& all, example* ec);
flat_example* flatten_sort_example(VW::workspace& all, example* ec);
void free_flatten_example(flat_example* fec);

inline bool example_is_newline(const example& ec) { return ec.is_newline; }

inline bool valid_ns(char c) { return !(c == '|' || c == ':'); }

inline void add_passthrough_feature_magic(example& ec, uint64_t magic, uint64_t i, float x)
{
  if (ec.passthrough) { ec.passthrough->push_back(x, (FNV_prime * magic) ^ i); }
}

#define add_passthrough_feature(ec, i, x) \
  VW::add_passthrough_feature_magic(ec, __FILE__[0] * 483901 + __FILE__[1] * 3417 + __FILE__[2] * 8490177, i, x);

using multi_ex = std::vector<example*>;

void return_multiple_example(VW::workspace& all, VW::multi_ex& examples);

using example_factory_t = example& (*)(void*);

namespace model_utils
{
size_t read_model_field(io_buf& io, flat_example& fe, VW::label_parser& lbl_parser);
size_t write_model_field(io_buf& io, const flat_example& fe, const std::string& upstream_name, bool text,
    VW::label_parser& lbl_parser, uint64_t parse_mask);
}  // namespace model_utils
}  // namespace VW

// Deprecated compat definitions

using polylabel VW_DEPRECATED("polylabel moved into VW namespace") = VW::polylabel;
using polyprediction VW_DEPRECATED("polyprediction moved into VW namespace") = VW::polyprediction;
using example VW_DEPRECATED("example moved into VW namespace") = VW::example;
using multi_ex VW_DEPRECATED("multi_ex moved into VW namespace") = VW::multi_ex;
using flat_example VW_DEPRECATED("flat_example moved into VW namespace") = VW::flat_example;

VW_DEPRECATED("flatten_example moved into VW namespace")
inline VW::flat_example* flatten_example(VW::workspace& all, VW::example* ec) { return VW::flatten_example(all, ec); }

VW_DEPRECATED("flatten_sort_example moved into VW namespace")
inline VW::flat_example* flatten_sort_example(VW::workspace& all, VW::example* ec)
{
  return VW::flatten_sort_example(all, ec);
}
VW_DEPRECATED("free_flatten_example moved into VW namespace")
inline void free_flatten_example(VW::flat_example* fec) { return VW::free_flatten_example(fec); }

VW_DEPRECATED("example_is_newline moved into VW namespace")
inline bool example_is_newline(const VW::example& ec) { return VW::example_is_newline(ec); }

VW_DEPRECATED("valid_ns moved into VW namespace")
inline bool valid_ns(char c) { return VW::valid_ns(c); }

VW_DEPRECATED("add_passthrough_feature_magic moved into VW namespace")
inline void add_passthrough_feature_magic(VW::example& ec, uint64_t magic, uint64_t i, float x)
{
  return VW::add_passthrough_feature_magic(ec, magic, i, x);
}