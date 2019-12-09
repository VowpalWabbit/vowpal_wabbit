/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */

#pragma once

#include <cstdint>
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
#include <vector>

typedef union
{
  no_label::no_label empty;
  label_data simple;
  MULTICLASS::label_t multi;
  COST_SENSITIVE::label cs;
  CB::label cb;
  CCB::label conditional_contextual_bandit;
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
  CCB::decision_scores_t decision_scores;
  uint32_t multiclass;
  MULTILABEL::labels multilabels;
  float prob;  // for --probabilities --csoaa_ldf=mc
} polyprediction;

class gd_prediction_cache
{
  std::vector<polyprediction> _cache;

 public:
  bool inline get_value(uint64_t ft_offset, polyprediction& pred)
  {
    if (_cache.size() > ft_offset)
    {
      pred = _cache[ft_offset];
      return true;
    }
    return false;
  }

  void inline set_value(uint64_t ft_offset, const polyprediction& pred)
  {
    if (_cache.size() <= ft_offset)
    {
      _cache.resize(ft_offset + 1);
    }
    _cache[ft_offset] = pred;
  }

  void inline clear() { _cache.clear(); }
};

class gd_prediction_cache_noop
{
 public:
  bool inline get_value(uint64_t, polyprediction&) { return false; }

  void inline set_value(uint64_t, const polyprediction&) {}

  void inline clear() {}
};

struct example : public example_predict  // core example datatype.
{
  // input fields
  polylabel l;

  // Notes: TLDR; needed to make predict() indpendent of label (as it should theoretically should be)
  // 1) initial used to be in label_data (simple label)
  // 2) gd.predict() used to use this to load intial value
  // 3) It also used it as an accumulator and modified it.
  // 4) This cause two breaches of label independence abstraction during predict()
  //      a) All reductions depending on gd had to initalize example.l to sane values before base.predict()
  //      b) All reductions had to save label state before calling base.predict()
  // Making it impossible to remove dependence of predict on label
  float initial;

  // output prediction
  polyprediction pred;

  float weight;       // a relative importance weight for the example, default = 1
  v_array<char> tag;  // An identifier for the example.
  size_t example_counter;

  // helpers
  size_t num_features;       // precomputed, cause it's fast&easy.
  float partial_prediction;  // shared data for prediction.
  float updated_prediction;  // estimated post-update prediction.
  float loss;
  float total_sum_feat_sq;  // precomputed, cause it's kind of fast & easy.
  float confidence;
  features*
      passthrough;  // if a higher-up reduction wants access to internal state of lower-down reductions, they go here

  bool test_only;
  bool end_pass;  // special example indicating end of pass.
  bool sorted;    // Are the features sorted or not?
  bool in_use;    // in use or not (for the parser)
  bool predict_called_before_learn; // If the driver calls predict before learn, this flag will be set
};

struct vw;

struct flat_example
{
  polylabel l;
  float weight;  // a relative importance weight for the example, default = 1

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
}  // namespace VW
std::string features_to_string(const example& ec);
std::string simple_label_to_string(const example& ec);
std::string scalar_pred_to_string(const example& ec);
std::string a_s_pred_to_string(const example& ec);
std::string multiclass_pred_to_string(const example& ec);
std::string depth_indent_string(const multi_ex& ec);
std::string depth_indent_string(const example& ec);
std::string depth_indent_string(uint32_t depth);
std::string cb_label_to_string(const example& ec);
