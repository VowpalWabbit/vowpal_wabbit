// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

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
#include "vw_exception.h"
#include "label.h"
#include "prediction.h"

VW_DEPRECATED("no longer used")
inline void delete_scalars(void* v)
{
  // v_array<float>* preds = (v_array<float>*)v;
  // preds->delete_v();
}

struct example : public example_predict  // core example datatype.
{
  // input fields
  new_polylabel l;

  // output prediction
  new_polyprediction pred;

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

  ~example()
  {
    if (passthrough)
    {
      delete passthrough;
    }
  }
};

struct vw;

struct flat_example
{
  new_polylabel l;

  size_t tag_len;
  char* tag = nullptr;  // An identifier for the example.

  size_t example_counter;
  uint64_t ft_offset;
  float global_weight;

  size_t num_features;      // precomputed, cause it's fast&easy.
  float total_sum_feat_sq;  // precomputed, cause it's kind of fast & easy.
  features fs;              // all the features

  ~flat_example()
  {
    if (tag_len > 0)
      free(tag);
  }

 /* flat_example(const flat_example& other) {
    l = other.l;
    if (tag_len > 0)
      free(tag);
    memcpy(tag, other.tag, other.tag_len);
    tag_len = other.tag_len;
    example_counter = other.example_counter;
    ft_offset = other.ft_offset;
    global_weight = other.global_weight;
    num_features = other.num_features;
    total_sum_feat_sq = other.total_sum_feat_sq;
    fs = other.fs;
  }

  flat_example& operator=(const flat_example& other) {}

  flat_example(flat_example&& other) {}

  flat_example& operator=(flat_example&& other) {}*/
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
