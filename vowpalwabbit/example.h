/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */

#pragma once

#include <stdint.h>
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
#include <vector>

const unsigned char wap_ldf_namespace = 126;
const unsigned char history_namespace = 127;
const unsigned char constant_namespace = 128;
const unsigned char nn_output_namespace = 129;
const unsigned char autolink_namespace = 130;
const unsigned char neighbor_namespace =
    131;  // this is \x83 -- to do quadratic, say "-q a`printf "\x83"` on the command line
const unsigned char affix_namespace = 132;         // this is \x84
const unsigned char spelling_namespace = 133;      // this is \x85
const unsigned char conditioning_namespace = 134;  // this is \x86
const unsigned char dictionary_namespace = 135;    // this is \x87
const unsigned char node_id_namespace = 136;       // this is \x88
const unsigned char message_namespace = 137;       // this is \x89

typedef union {
  no_label::no_label empty;
  label_data simple;
  MULTICLASS::label_t multi;
  COST_SENSITIVE::label cs;
  CB::label cb;
  CB_EVAL::label cb_eval;
  MULTILABEL::labels multilabels;
} polylabel;

inline void delete_scalars(void* v)
{
  v_array<float>* preds = (v_array<float>*)v;
  preds->delete_v();
}

typedef union {
  float scalar;
  v_array<float> scalars;           // a sequence of scalar predictions
  ACTION_SCORE::action_scores a_s;  // a sequence of classes with scores.  Also used for probabilities.
  uint32_t multiclass;
  MULTILABEL::labels multilabels;
  float prob;  // for --probabilities --csoaa_ldf=mc
} polyprediction;

struct example : public example_predict  // core example datatype.
{
  // input fields
  polylabel l;

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
};

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

inline int example_is_newline(example& ec)
{  // if only index is constant namespace or no index
  if (ec.tag.size() > 0)
    return false;
  return ((ec.indices.size() == 0) || ((ec.indices.size() == 1) && (ec.indices.last() == constant_namespace)));
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
void clear_seq_and_finish_examples(vw& all, multi_ex& ec_seq);

void return_multiple_example(vw& all, v_array<example*>& examples);
}  // namespace VW
