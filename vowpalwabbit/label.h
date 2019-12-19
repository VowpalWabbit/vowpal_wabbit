#pragma once

#include "no_label.h"
#include "simple_label.h"
#include "multiclass.h"
#include "multilabel.h"
#include "cost_sensitive.h"
#include "cb.h"
#include "example_predict.h"
#include "ccb_label.h"

union polylabel {
  no_label::no_label empty;
  label_data simple;
  MULTICLASS::label_t multi;
  COST_SENSITIVE::label cs;
  CB::label cb;
  CCB::label conditional_contextual_bandit;
  CB_EVAL::label cb_eval;
  MULTILABEL::labels multilabels;

  polylabel() { memset(this, 0, sizeof(polylabel)); }
  ~polylabel() { }
};

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
 private:
  polylabel internal_union;
  label_type_t tag = label_type_t::unset;

  inline void ensure_is_type(label_type_t type) const
  {
    if (tag != type)
    {
      THROW("Expected type: " << to_string(type) << ", but found: " << to_string(tag));
    }
  }

  template <typename T>
  void destruct(T& item)
  {
    item.~T();
  }

 public:
  new_polylabel() {}

  label_type_t get_type() const { return tag; }

  void reset()
  {
    switch (tag)
    {
      case (label_type_t::unset):
        // Nothing to do! Whatever was in here has already been destroyed.
        break;
      case (label_type_t::empty):
        destruct(internal_union.empty);
        break;
      case (label_type_t::simple):
        destruct(internal_union.simple);
        break;
      case (label_type_t::multi):
        destruct(internal_union.multi);
        break;
      case (label_type_t::cs):
        destruct(internal_union.cs);
        break;
      case (label_type_t::cb):
        destruct(internal_union.cb);
        break;
      case (label_type_t::conditional_contextual_bandit):
        destruct(internal_union.conditional_contextual_bandit);
        break;
      case (label_type_t::cb_eval):
        destruct(internal_union.cb_eval);
        break;
      case (label_type_t::multilabels):
        destruct(internal_union.multilabels);
        break;
      default:;
    }

    tag = label_type_t::unset;
  }

  template <typename... Args>
  no_label::no_label& init_as_empty(Args&&... args)
  {
    ensure_is_type(label_type_t::unset);
    new (&internal_union.empty) no_label::no_label(std::forward<Args>(args)...);
    tag = label_type_t::empty;
    return internal_union.empty;
  }

  const no_label::no_label& empty() const
  {
    ensure_is_type(label_type_t::empty);
    return internal_union.empty;
  }

  no_label::no_label& empty()
  {
    ensure_is_type(label_type_t::empty);
    return internal_union.empty;
  }

  template <typename... Args>
  label_data& init_as_simple(Args&&... args)
  {
    ensure_is_type(label_type_t::unset);
    new (&internal_union.simple) label_data(std::forward<Args>(args)...);
    tag = label_type_t::simple;
    return internal_union.simple;
  }

  const label_data& simple() const
  {
    ensure_is_type(label_type_t::simple);
    return internal_union.simple;
  }

  label_data& simple()
  {
    ensure_is_type(label_type_t::simple);
    return internal_union.simple;
  }

  template <typename... Args>
  MULTICLASS::label_t& init_as_multi(Args&&... args)
  {
    ensure_is_type(label_type_t::unset);
    new (&internal_union.multi) MULTICLASS::label_t(std::forward<Args>(args)...);
    tag = label_type_t::multi;
    return internal_union.multi;
  }

  const MULTICLASS::label_t& multi() const
  {
    ensure_is_type(label_type_t::multi);
    return internal_union.multi;
  }

  MULTICLASS::label_t& multi()
  {
    ensure_is_type(label_type_t::multi);
    return internal_union.multi;
  }

  template <typename... Args>
  COST_SENSITIVE::label& init_as_cs(Args&&... args)
  {
    ensure_is_type(label_type_t::unset);
    new (&internal_union.cs) COST_SENSITIVE::label(std::forward<Args>(args)...);
    tag = label_type_t::cs;
    return internal_union.cs;
  }

  const COST_SENSITIVE::label& cs() const
  {
    ensure_is_type(label_type_t::cs);
    return internal_union.cs;
  }

  COST_SENSITIVE::label& cs()
  {
    ensure_is_type(label_type_t::cs);
    return internal_union.cs;
  }

  template <typename... Args>
  CB::label& init_as_cb(Args&&... args)
  {
    ensure_is_type(label_type_t::unset);
    new (&internal_union.cb) CB::label(std::forward<Args>(args)...);
    tag = label_type_t::cb;
    return internal_union.cb;
  }
  const CB::label& cb() const
  {
    ensure_is_type(label_type_t::cb);
    return internal_union.cb;
  }

  CB::label& cb()
  {
    ensure_is_type(label_type_t::cb);
    return internal_union.cb;
  }

  template <typename... Args>
  CCB::label& init_as_conditional_contextual_bandit(Args&&... args)
  {
    ensure_is_type(label_type_t::unset);
    new (&internal_union.conditional_contextual_bandit) CCB::label(std::forward<Args>(args)...);
    tag = label_type_t::conditional_contextual_bandit;
    return internal_union.conditional_contextual_bandit;
  }

  const CCB::label& conditional_contextual_bandit() const
  {
    ensure_is_type(label_type_t::conditional_contextual_bandit);
    return internal_union.conditional_contextual_bandit;
  }

  CCB::label& conditional_contextual_bandit()
  {
    ensure_is_type(label_type_t::conditional_contextual_bandit);
    return internal_union.conditional_contextual_bandit;
  }

  template <typename... Args>
  CB_EVAL::label& init_as_cb_eval(Args&&... args)
  {
    ensure_is_type(label_type_t::unset);
    new (&internal_union.cb_eval) CB_EVAL::label(std::forward<Args>(args)...);
    tag = label_type_t::cb_eval;
    return internal_union.cb_eval;
  }

  const CB_EVAL::label& cb_eval() const
  {
    ensure_is_type(label_type_t::cb_eval);
    return internal_union.cb_eval;
  }

  CB_EVAL::label& cb_eval()
  {
    ensure_is_type(label_type_t::cb_eval);
    return internal_union.cb_eval;
  }

  template <typename... Args>
  MULTILABEL::labels& init_as_multilabels(Args&&... args)
  {
    ensure_is_type(label_type_t::unset);
    new (&internal_union.multilabels) MULTILABEL::labels(std::forward<Args>(args)...);
    tag = label_type_t::multilabels;
    return internal_union.multilabels;
  }

  const MULTILABEL::labels& multilabels() const
  {
    ensure_is_type(label_type_t::multilabels);
    return internal_union.multilabels;
  }

  MULTILABEL::labels& multilabels()
  {
    ensure_is_type(label_type_t::multilabels);
    return internal_union.multilabels;
  }
};
