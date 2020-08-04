// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <cstdint>
#include <vector>

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
#include "ccb_label.h"
#include "slates_label.h"
#include "decision_scores.h"
#include <vector>
#include <iostream>
#include "cb_continuous_label.h"
#include "prob_dist_cont.h"

typedef union
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
} polylabel;

inline void delete_scalars(void* v)
{
  v_array<float>* preds = (v_array<float>*)v;
  preds->delete_v();
}

typedef union
{
  float scalar;
  v_array<float> scalars;           // a sequence of scalar predictions
  ACTION_SCORE::action_scores a_s;  // a sequence of classes with scores.  Also used for probabilities.
  VW::actions_pdf::pdf prob_dist;
  VW::decision_scores_t decision_scores;
  uint32_t multiclass;
  MULTILABEL::labels multilabels;
  float prob;  // for --probabilities --csoaa_ldf=mc
  VW::actions_pdf::action_pdf_value a_pdf;
} polyprediction;

VW_WARNING_STATE_PUSH
VW_WARNING_DISABLE_DEPRECATED_USAGE
struct example : public example_predict  // core example datatype.
{
  example();
  ~example();

  example(const example&) = delete;
  example& operator=(const example&) = delete;
  example(example&& other) noexcept;
  example& operator=(example&& other) noexcept;

  /// Example contains unions for label and prediction. These do not get cleaned
  /// up by the constructor because the type is not known at that time. To
  /// ensure correct cleanup delete_unions must be explicitly called.
  void delete_unions(void (*delete_label)(void*), void (*delete_prediction)(void*));

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

  VW_DEPRECATED(
      "in_use has been removed, examples taken from the pool are assumed to be in use if there is a reference to them. "
      "Standalone examples are by definition always in use.")
  bool in_use = true;
};
VW_WARNING_STATE_POP

struct vw;

struct flat_example
{
  polylabel l;

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

inline int example_is_newline(example const& ec)
{  // if only index is constant namespace or no index
  if (!ec.tag.empty())
    return false;
  return ((ec.indices.empty()) || ((ec.indices.size() == 1) && (ec.indices.last() == constant_namespace)));
}

inline bool valid_ns(char c) { return !(c == '|' || c == ':'); }

inline void add_passthrough_feature_magic(example& ec, uint64_t magic, uint64_t i, float x)
{
  if (ec.passthrough)
    ec.passthrough->push_back(x, (FNV_prime * magic) ^ i);
}

#define add_passthrough_feature(ec, i, x) \
  add_passthrough_feature_magic(ec, __FILE__[0] * 483901 + __FILE__[1] * 3417 + __FILE__[2] * 8490177, i, x);

typedef std::vector<example*> multi_ex;

namespace VW
{
void return_multiple_example(vw& all, v_array<example*>& examples);

struct restore_prediction
{
  restore_prediction(example& ec);
  ~restore_prediction();

 private:
  const polyprediction _prediction;
  example& _ec;
};

struct swap_restore_action_scores_prediction
{
  swap_restore_action_scores_prediction(example& ec, ACTION_SCORE::action_scores& base_prediction);
  ~swap_restore_action_scores_prediction();

 private:
  const polyprediction _prediction;
  example& _ec;
  ACTION_SCORE::action_scores& _base_prediction;
};

struct swap_restore_pdf_prediction
{
  swap_restore_pdf_prediction(example& ec, actions_pdf::pdf& base_prediction);
  ~swap_restore_pdf_prediction();

 private:
  const polyprediction _prediction;
  example& _ec;
  actions_pdf::pdf& _base_prediction;
};

struct swap_restore_cb_label
{
  swap_restore_cb_label(example& ec, CB::label& base_label);
  ~swap_restore_cb_label();

  private:
    const polylabel _label;
    example& _ec;
    CB::label& _base_label;
};

}  // namespace VW
std::string features_to_string(const example& ec);
std::string simple_label_to_string(const example& ec);
std::string scalar_pred_to_string(const example& ec);
std::string a_s_pred_to_string(const example& ec);
std::string prob_dist_pred_to_string(const example& ec);
std::string multiclass_pred_to_string(const example& ec);
std::string depth_indent_string(const example& ec);
std::string depth_indent_string(int32_t stack_depth);
std::string cb_label_to_string(const example& ec);
