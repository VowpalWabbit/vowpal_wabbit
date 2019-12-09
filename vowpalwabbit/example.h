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

enum class label_type_tag
{
  unset,
  empty,
  simple,
  multi,
  cs,
  cb,
  conditional_contextual_bandit,
  cb_eval,
  multilabels
};

struct new_polylabel
{
  mutable polylabel internal_union;
  mutable label_type_tag tag = label_type_tag::unset;

  new_polylabel() {
    memset(&internal_union, 0, sizeof(polylabel));
  }
 
  no_label::no_label& empty() const
  {
    if (tag != label_type_tag::empty)
    {
      memset(&internal_union, 0, sizeof(polylabel));
      tag = label_type_tag::empty;
    }
   
    return internal_union.empty;
  }


  label_data& simple() const
  {
    if (tag != label_type_tag::simple)
    {
      memset(&internal_union, 0, sizeof(polylabel));
      tag = label_type_tag::simple;
    }else
    {
      THROW("Polylabel already set");
    }

    return internal_union.simple;
  }


  MULTICLASS::label_t& multi() const
  {
    if (tag != label_type_tag::multi)
    {
      memset(&internal_union, 0, sizeof(polylabel));
      tag = label_type_tag::multi;
    }else
    {
      THROW("Polylabel already set");
    }

    return internal_union.multi;
  }


  COST_SENSITIVE::label& cs() const
  {
    if (tag != label_type_tag::cs)
    {
      memset(&internal_union, 0, sizeof(polylabel));
      tag = label_type_tag::cs;
    }else
    {
      THROW("Polylabel already set");
    }

    return internal_union.cs;
  }

  CB::label& cb() const
  {
    if (tag != label_type_tag::cb)
    {
      memset(&internal_union, 0, sizeof(polylabel));
      tag = label_type_tag::cb;
    }else
    {
      THROW("Polylabel already set");
    }

    return internal_union.cb;
  }
  CCB::label& conditional_contextual_bandit() const
  {
    if (tag != label_type_tag::conditional_contextual_bandit)
    {
      memset(&internal_union, 0, sizeof(polylabel));
      tag = label_type_tag::conditional_contextual_bandit;
    }else
    {
      THROW("Polylabel already set");
    }

    return internal_union.conditional_contextual_bandit;
  }


  CB_EVAL::label& cb_eval() const
  {
    if (tag != label_type_tag::cb_eval)
    {
      memset(&internal_union, 0, sizeof(polylabel));
      tag = label_type_tag::cb_eval;
    }else
    {
      THROW("Polylabel already set");
    }

    return internal_union.cb_eval;
  }

  MULTILABEL::labels& multilabels()
  {
    if (tag != label_type_tag::multilabels)
    {
      memset(&internal_union, 0, sizeof(polylabel));
      tag = label_type_tag::multilabels;
    }else
    {
      THROW("Polylabel already set");
    }

    return internal_union.multilabels;
  }
};

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

struct example : public example_predict  // core example datatype.
{
  // input fields
  new_polylabel l;

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
  new_polylabel l;

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
