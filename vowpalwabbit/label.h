#pragma once

#include "no_label.h"
#include "simple_label.h"
#include "multiclass.h"
#include "multilabel.h"
#include "cost_sensitive.h"
#include "cb.h"
#include "example_predict.h"
#include "ccb_label.h"

typedef union {
  no_label::no_label empty;
  label_data simple;
  MULTICLASS::label_t multi;
  COST_SENSITIVE::label cs;
  CB::label cb;
  CCB::label conditional_contextual_bandit;
  CB_EVAL::label cb_eval;
  MULTILABEL::labels multilabels;
} polylabel;

#define TO_STRING_CASE(enum_type) \
  case enum_type:                 \
    return #enum_type;

enum class label_type_t
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

inline const char* to_string(label_type_t label_type)
{
  switch (label_type)
  {
    TO_STRING_CASE(label_type_t::unset)
    TO_STRING_CASE(label_type_t::empty)
    TO_STRING_CASE(label_type_t::simple)
    TO_STRING_CASE(label_type_t::multi)
    TO_STRING_CASE(label_type_t::cs)
    TO_STRING_CASE(label_type_t::cb)
    TO_STRING_CASE(label_type_t::conditional_contextual_bandit)
    TO_STRING_CASE(label_type_t::cb_eval)
    TO_STRING_CASE(label_type_t::multilabels)
    default:
      return "<unsupported>";
  }
}

struct new_polylabel
{
  mutable polylabel internal_union;
  mutable label_type_t tag = label_type_t::unset;

  new_polylabel() {}

  label_type_t get_type() const { return tag; }

  void reset() const
  {
    switch (tag)
    {
      case (label_type_t::unset):
        break;
      case (label_type_t::empty):
        break;
      case (label_type_t::simple):
        break;
      case (label_type_t::multi):
        break;
      case (label_type_t::cs):
        break;
      case (label_type_t::cb):
        break;
      case (label_type_t::conditional_contextual_bandit):
        break;
      case (label_type_t::cb_eval):
        break;
      case (label_type_t::multilabels):
        break;
      default:;
    }

    tag = label_type_t::unset;
  }

  no_label::no_label& empty() const
  {
    if (tag != label_type_t::empty)
    {
      if (tag != label_type_t::unset)
      {
        std::cout << "prev: " << to_string(tag) << ", to: " << to_string(label_type_t::empty) << std::endl;
      }

      reset();
      new (&internal_union.empty) no_label::no_label();
      tag = label_type_t::empty;
    }

    return internal_union.empty;
  }

  label_data& simple() const
  {
    if (tag != label_type_t::simple)
    {
      if (tag != label_type_t::unset)
      {
        std::cout << "prev: " << to_string(tag) << ", to: " << to_string(label_type_t::simple) << std::endl;
      }

      reset();
      new (&internal_union.simple) label_data();
      tag = label_type_t::simple;
    }

    return internal_union.simple;
  }

  MULTICLASS::label_t& multi() const
  {
    if (tag != label_type_t::multi)
    {
      if (tag != label_type_t::unset)
      {
        std::cout << "prev: " << to_string(tag) << ", to: " << to_string(label_type_t::multi) << std::endl;
      }

      memset(&internal_union, 0, sizeof(polylabel));
      tag = label_type_t::multi;
    }

    return internal_union.multi;
  }

  COST_SENSITIVE::label& cs() const
  {
    if (tag != label_type_t::cs)
    {
      if (tag != label_type_t::unset)
      {
        std::cout << "prev: " << to_string(tag) << ", to: " << to_string(label_type_t::cs) << std::endl;
      }

      memset(&internal_union, 0, sizeof(polylabel));
      tag = label_type_t::cs;
    }
    return internal_union.cs;
  }

  CB::label& cb() const
  {
    if (tag != label_type_t::cb)
    {
      if (tag != label_type_t::unset)
      {
        std::cout << "prev: " << to_string(tag) << ", to: " << to_string(label_type_t::cb) << std::endl;
      }

      memset(&internal_union, 0, sizeof(polylabel));
      tag = label_type_t::cb;
    }

    return internal_union.cb;
  }
  CCB::label& conditional_contextual_bandit() const
  {
    if (tag != label_type_t::conditional_contextual_bandit)
    {
      if (tag != label_type_t::unset)
      {
        std::cout << "prev: " << to_string(tag) << ", to: " << to_string(label_type_t::conditional_contextual_bandit)
                  << std::endl;
      }

      memset(&internal_union, 0, sizeof(polylabel));
      tag = label_type_t::conditional_contextual_bandit;
    }

    return internal_union.conditional_contextual_bandit;
  }

  CB_EVAL::label& cb_eval() const
  {
    if (tag != label_type_t::cb_eval)
    {
      if (tag != label_type_t::unset)
      {
        std::cout << "prev: " << to_string(tag) << ", to: " << to_string(label_type_t::cb_eval) << std::endl;
      }

      memset(&internal_union, 0, sizeof(polylabel));
      tag = label_type_t::cb_eval;
    }

    return internal_union.cb_eval;
  }

  MULTILABEL::labels& multilabels() const
  {
    if (tag != label_type_t::multilabels)
    {
      if (tag != label_type_t::unset)
      {
        std::cout << "prev: " << to_string(tag) << ", to: " << to_string(label_type_t::multilabels) << std::endl;
      }

      memset(&internal_union, 0, sizeof(polylabel));
      tag = label_type_t::multilabels;
    }

    return internal_union.multilabels;
  }
};
