// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once
// This is the interface for a learning algorithm
#include <iostream>
#include "memory.h"
#include "multiclass.h"
#include "simple_label.h"
#include "parser.h"
#include "future_compat.h"
#include "label.h"
#include <memory>

enum class prediction_type_t
{
  scalar,
  scalars,
  action_scores,
  action_probs,
  multiclass,
  multilabels,
  prob,
  multiclassprobs,
  decision_probs
};

const char* to_string(prediction_type_t prediction_type);

namespace LEARNER
{
template <class T, class E>
struct learner;

using base_learner = learner<char, char>;
using single_learner = learner<char, example>;
using multi_learner = learner<char, multi_ex>;

struct func_data
{
  using fn = void (*)(void* data);
  void* data;
  base_learner* base;
  fn func;
};

inline func_data tuple_dbf(void* data, base_learner* base, void (*func)(void*))
{
  func_data foo;
  foo.data = data;
  foo.base = base;
  foo.func = func;
  return foo;
}

struct learn_data
{
  using fn = void (*)(void* data, base_learner& base, void* ex);
  // Label is either label& or std::vector<label>&
  using fn_with_label = void (*)(void* data, base_learner& base, void* ex, void* label);
  using multi_fn = void (*)(void* data, base_learner& base, void* ex, size_t count, size_t step, polyprediction* pred,
      bool finalize_predictions);

  void* data;
  base_learner* base;
  fn learn_f;
  fn predict_f;

  fn_with_label learn_with_label_f;
  fn_with_label predict_with_label_f;

  fn update_f;
  multi_fn multipredict_f;
};

struct sensitivity_data
{
  using fn = float (*)(void* data, base_learner& base, example& ex);
  void* data;
  fn sensitivity_f;
};

struct save_load_data
{
  using fn = void (*)(void*, io_buf&, bool read, bool text);
  void* data;
  base_learner* base;
  fn save_load_f;
};

struct finish_example_data
{
  using fn = void (*)(vw&, void* data, void* ex);
  void* data;
  base_learner* base;
  fn finish_example_f;
};

void generic_driver(vw& all);
void generic_driver(const std::vector<vw*>& alls);
void generic_driver_onethread(vw& all);

inline void noop_sl(void*, io_buf&, bool, bool) {}
inline void noop(void*) {}
inline float noop_sensitivity(void*, base_learner&, example&)
{
  std::cout << std::endl;
  return 0.;
}
float recur_sensitivity(void*, base_learner&, example&);

inline void increment_offset(example& ex, const size_t increment, const size_t i)
{
  ex.ft_offset += static_cast<uint32_t>(increment * i);
}

inline void increment_offset(multi_ex& ec_seq, const size_t increment, const size_t i)
{
  for (auto ec : ec_seq) ec->ft_offset += static_cast<uint32_t>(increment * i);
}

inline void decrement_offset(example& ex, const size_t increment, const size_t i)
{
  assert(ex.ft_offset >= increment * i);
  ex.ft_offset -= static_cast<uint32_t>(increment * i);
}

inline void decrement_offset(multi_ex& ec_seq, const size_t increment, const size_t i)
{
  for (auto ec : ec_seq)
  {
    assert(ec->ft_offset >= increment * i);
    ec->ft_offset -= static_cast<uint32_t>(increment * i);
  }
}

#define TYPED_EXAMPLE_LEARN_FN(TYPE, FIELD_NAME)                                                                       \
  inline void learn_with_label(example& ec, TYPE& label, size_t i = 0)                                                 \
  {                                                                                                                    \
    assert((is_multiline && std::is_same<multi_ex, E>::value) || (!is_multiline && std::is_same<example, E>::value));  \
    increment_offset(ec, increment, i);                                                                                \
    if (learn_fd.learn_with_label_f != nullptr)                                                                        \
    {                                                                                                                  \
      learn_fd.learn_with_label_f(learn_fd.data, *learn_fd.base, static_cast<void*>(&ec), static_cast<void*>(&label)); \
    }                                                                                                                  \
    else                                                                                                               \
    {                                                                                                                  \
      polylabel saved = std::move(ec.l);                                                                                          \
      ec.l.FIELD_NAME = std::move(label);                                                                                         \
      learn_fd.learn_f(learn_fd.data, *learn_fd.base, static_cast<void*>(&ec));                                        \
      label = std::move(ec.l.FIELD_NAME);\
      ec.l = std::move(saved);                                                                                                    \
    }                                                                                                                  \
                                                                                                                       \
    decrement_offset(ec, increment, i);                                                                                \
  }

#define TYPED_EXAMPLE_PREDICT_FN(TYPE, FIELD_NAME)                                                                    \
  inline void predict_with_label(example& ec, TYPE& label, size_t i = 0)                                              \
  {                                                                                                                   \
    assert((is_multiline && std::is_same<multi_ex, E>::value) || (!is_multiline && std::is_same<example, E>::value)); \
    increment_offset(ec, increment, i);                                                                               \
    if (learn_fd.predict_with_label_f != nullptr)                                                                     \
    {                                                                                                                 \
      learn_fd.predict_with_label_f(                                                                                  \
          learn_fd.data, *learn_fd.base, static_cast<void*>(&ec), static_cast<void*>(&label));                        \
    }                                                                                                                 \
    else                                                                                                              \
    {                                                                                                                 \
      polylabel saved = std::move(ec.l);                                                                                          \
      ec.l.FIELD_NAME = std::move(label);                                                                                         \
      learn_fd.predict_f(learn_fd.data, *learn_fd.base, static_cast<void*>(&ec));                                        \
      label = std::move(ec.l.FIELD_NAME);\
      ec.l = std::move(saved);                                                                                                    \
    }                                                                                                                 \
                                                                                                                      \
    decrement_offset(ec, increment, i);                                                                               \
  }

#define TYPED_MULTI_EXAMPLE_LEARN_FN(TYPE, FIELD_NAME)                                                                \
  inline void learn_with_label(std::vector<example*>& example_collection, std::vector<TYPE>& labels, size_t i = 0)    \
  {                                                                                                                   \
    assert((is_multiline && std::is_same<multi_ex, E>::value) || (!is_multiline && std::is_same<example, E>::value)); \
    increment_offset(example_collection, increment, i);                                                               \
    if (learn_fd.learn_with_label_f != nullptr)                                                                       \
    {                                                                                                                 \
      learn_fd.learn_with_label_f(                                                                                    \
          learn_fd.data, *learn_fd.base, static_cast<void*>(&example_collection), static_cast<void*>(&labels));       \
    }                                                                                                                 \
    else                                                                                                              \
    {                                                                                                                 \
      std::vector<polylabel> saved;                                                                                   \
      saved.reserve(example_collection.size());                                                                       \
      size_t it = 0;                                                                                                  \
      for (const auto& example : example_collection)                                                                  \
      {                                                                                                               \
        saved.push_back(std::move(example->l));                                                                       \
        example->l.FIELD_NAME = std::move(labels[it]);                                                                \
        it++;                                                                                                         \
      }                                                                                                               \
      learn_fd.learn_f(                                                                                               \
          learn_fd.data, *learn_fd.base, static_cast<void*>(&example_collection));         \
      it = 0;                                                                                                  \
      for (const auto& example : example_collection)                                                                  \
      {                                                                                                               \
        example->l = std::move(saved[it]);                                                                            \
        labels[it] = std::move(example->l.FIELD_NAME); \
        it++;                                                           \
      }                                                                                                               \
    }                                                                                                                 \
  }

#define TYPED_MULTI_EXAMPLE_PREDICT_FN(TYPE, FIELD_NAME)                                                              \
  inline void predict_with_label(std::vector<example*>& example_collection, std::vector<TYPE>& labels, size_t i = 0)   \
  {                                                                                                                   \
    assert((is_multiline && std::is_same<multi_ex, E>::value) || (!is_multiline && std::is_same<example, E>::value)); \
    increment_offset(example_collection, increment, i);                                                               \
    if (learn_fd.predict_with_label_f != nullptr)                                                                     \
    {                                                                                                                 \
      learn_fd.predict_with_label_f(                                                                                  \
          learn_fd.data, *learn_fd.base, static_cast<void*>(&example_collection), static_cast<void*>(&labels));        \
    }                                                                                                                 \
    else                                                                                                              \
    {                                                                                                                 \
      std::vector<polylabel> saved;                                                                                   \
      saved.reserve(example_collection.size());                                                                       \
      size_t it = 0;                                                                                                  \
      for (const auto& example : example_collection)                                                                  \
      {                                                                                                               \
        saved.push_back(std::move(example->l));                                                                       \
        example->l.FIELD_NAME = std::move(labels[it]);                                                                \
        it++;                                                                                                         \
      }                                                                                                               \
      learn_fd.predict_f(                                                                                             \
          learn_fd.data, *learn_fd.base, static_cast<void*>(&example_collection));         \
      it = 0;                                                                                                  \
      for (const auto& example : example_collection)                                                                  \
      {                                                                                                               \
        example->l = std::move(saved[it]);                                                                            \
        labels[it] = std::move(example->l.FIELD_NAME);\
         it++;                                                           \
      }                                                                                                               \
    }                                                                                                                 \
  }

#define DEFINE_TYPED_PREDICT_AND_LEARN_FUNCS(TYPE, FIELD_NAME) \
  TYPED_EXAMPLE_LEARN_FN(TYPE, FIELD_NAME)                     \
  TYPED_EXAMPLE_PREDICT_FN(TYPE, FIELD_NAME)                   \
  TYPED_MULTI_EXAMPLE_LEARN_FN(TYPE, FIELD_NAME)               \
  TYPED_MULTI_EXAMPLE_PREDICT_FN(TYPE, FIELD_NAME)

#define TYPED_EXAMPLE_LEARN_CASE(TYPE, FIELD_NAME)                                                                   \
  case label_type_t::FIELD_NAME:                                                                                     \
  {                                                                                                                  \
    TYPE saved = std::move(ec.l.FIELD_NAME);                                                                         \
    learn_fd.learn_with_label_f(learn_fd.data, *learn_fd.base, static_cast<void*>(&ec), static_cast<void*>(&saved)); \
    ec.l.FIELD_NAME = std::move(saved);                                                                              \
  }                                                                                                                  \
  break;

#define TYPED_MULTI_EXAMPLE_LEARN_CASE(TYPE, FIELD_NAME)                                                             \
  case label_type_t::FIELD_NAME:                                                                                     \
  {                                                                                                                  \
    std::vector<TYPE> saved;                                                                                         \
    saved.reserve(ec.size());                                                                                        \
    for (const auto& example : ec)                                                                                   \
    {                                                                                                                \
      saved.push_back(std::move(example->l.FIELD_NAME));                                                             \
    }                                                                                                                \
    learn_fd.learn_with_label_f(learn_fd.data, *learn_fd.base, static_cast<void*>(&ec), static_cast<void*>(&saved)); \
    size_t it = 0;                                                                                                   \
    for (const auto& example : ec)                                                                                   \
    {                                                                                                                \
      example->l.FIELD_NAME = std::move(saved[it]);                                                                  \
      it++;                                                                                                          \
    }                                                                                                                \
  }                                                                                                                  \
  break;

#define TYPED_EXAMPLE_PREDICT_CASE(TYPE, FIELD_NAME)                                                                   \
  case label_type_t::FIELD_NAME:                                                                                       \
  {                                                                                                                    \
    TYPE saved = std::move(ec.l.FIELD_NAME);                                                                           \
    learn_fd.predict_with_label_f(learn_fd.data, *learn_fd.base, static_cast<void*>(&ec), static_cast<void*>(&saved)); \
    ec.l.FIELD_NAME = std::move(saved);                                                                                \
  }                                                                                                                    \
  break;

#define TYPED_MULTI_EXAMPLE_PREDICT_CASE(TYPE, FIELD_NAME)                                                             \
  case label_type_t::FIELD_NAME:                                                                                       \
  {                                                                                                                    \
    std::vector<TYPE> saved;                                                                                           \
    saved.reserve(ec.size());                                                                                          \
    for (const auto& example : ec)                                                                                     \
    {                                                                                                                  \
      saved.push_back(std::move(example->l.FIELD_NAME));                                                               \
    }                                                                                                                  \
    learn_fd.predict_with_label_f(learn_fd.data, *learn_fd.base, static_cast<void*>(&ec), static_cast<void*>(&saved)); \
    size_t it = 0;                                                                                                     \
    for (const auto& example : ec)                                                                                     \
    {                                                                                                                  \
      example->l.FIELD_NAME = std::move(saved[it]);                                                                    \
      it++;                                                                                                            \
    }                                                                                                                  \
  }                                                                                                                    \
  break;

template <class T, class E>
struct learner
{
 private:
  func_data init_fd;
  learn_data learn_fd;
  sensitivity_data sensitivity_fd;
  finish_example_data finish_example_fd;
  save_load_data save_load_fd;
  func_data end_pass_fd;
  func_data end_examples_fd;
  func_data finisher_fd;

  std::shared_ptr<void> learner_data;
  learner(){};  // Should only be able to construct a learner through init_learner function
 public:
  prediction_type_t pred_type;
  label_type_t label_type;
  size_t weights;  // this stores the number of "weight vectors" required by the learner.
  size_t increment;
  bool is_multiline;  // Is this a single-line or multi-line reduction?

  using end_fptr_type = void (*)(vw&, void*, void*);
  using finish_fptr_type = void (*)(void*);

  DEFINE_TYPED_PREDICT_AND_LEARN_FUNCS(no_label::no_label, empty);
  DEFINE_TYPED_PREDICT_AND_LEARN_FUNCS(label_data, simple);
  DEFINE_TYPED_PREDICT_AND_LEARN_FUNCS(MULTICLASS::label_t, multi);
  DEFINE_TYPED_PREDICT_AND_LEARN_FUNCS(COST_SENSITIVE::label, cs);
  DEFINE_TYPED_PREDICT_AND_LEARN_FUNCS(CB::label, cb);
  DEFINE_TYPED_PREDICT_AND_LEARN_FUNCS(CCB::label, conditional_contextual_bandit);
  DEFINE_TYPED_PREDICT_AND_LEARN_FUNCS(CB_EVAL::label, cb_eval);
  DEFINE_TYPED_PREDICT_AND_LEARN_FUNCS(MULTILABEL::labels, multilabels);

  inline void learn(example& ec, size_t i = 0)
  {
    assert(!is_multiline);
    increment_offset(ec, increment, i);
    if (learn_fd.learn_with_label_f != nullptr)
    {
      switch (learn_fd.base->label_type)
      {
        TYPED_EXAMPLE_LEARN_CASE(no_label::no_label, empty);
        TYPED_EXAMPLE_LEARN_CASE(label_data, simple);
        TYPED_EXAMPLE_LEARN_CASE(MULTICLASS::label_t, multi);
        TYPED_EXAMPLE_LEARN_CASE(COST_SENSITIVE::label, cs);
        TYPED_EXAMPLE_LEARN_CASE(CB::label, cb);
        TYPED_EXAMPLE_LEARN_CASE(CCB::label, conditional_contextual_bandit);
        TYPED_EXAMPLE_LEARN_CASE(CB_EVAL::label, cb_eval);
        TYPED_EXAMPLE_LEARN_CASE(MULTILABEL::labels, multilabels);
        default:
          THROW("Unknown label type");
          break;
      }
    }
    else
    {
      learn_fd.learn_f(learn_fd.data, *learn_fd.base, static_cast<void*>(&ec));
    }
    decrement_offset(ec, increment, i);
  }

  // called once for each example.  Must work under reduction.
  inline void learn(multi_ex& ec, size_t i = 0)
  {
    assert(is_multiline);
    increment_offset(ec, increment, i);
    if (learn_fd.learn_with_label_f != nullptr)
    {
      switch (learn_fd.base->label_type)
      {
        TYPED_MULTI_EXAMPLE_LEARN_CASE(no_label::no_label, empty);
        TYPED_MULTI_EXAMPLE_LEARN_CASE(label_data, simple);
        TYPED_MULTI_EXAMPLE_LEARN_CASE(MULTICLASS::label_t, multi);
        TYPED_MULTI_EXAMPLE_LEARN_CASE(COST_SENSITIVE::label, cs);
        TYPED_MULTI_EXAMPLE_LEARN_CASE(CB::label, cb);
        TYPED_MULTI_EXAMPLE_LEARN_CASE(CCB::label, conditional_contextual_bandit);
        TYPED_MULTI_EXAMPLE_LEARN_CASE(CB_EVAL::label, cb_eval);
        TYPED_MULTI_EXAMPLE_LEARN_CASE(MULTILABEL::labels, multilabels);
        default:
          THROW("Unknown label type");
          break;
      }
    }
    else
    {
      learn_fd.learn_f(learn_fd.data, *learn_fd.base, static_cast<void*>(&ec));
    }
    decrement_offset(ec, increment, i);
  }

  inline void predict(example& ec, size_t i = 0)
  {
    assert(!is_multiline);
    increment_offset(ec, increment, i);
    if (learn_fd.predict_with_label_f != nullptr)
    {
      switch (learn_fd.base->label_type)
      {
        TYPED_EXAMPLE_PREDICT_CASE(no_label::no_label, empty);
        TYPED_EXAMPLE_PREDICT_CASE(label_data, simple);
        TYPED_EXAMPLE_PREDICT_CASE(MULTICLASS::label_t, multi);
        TYPED_EXAMPLE_PREDICT_CASE(COST_SENSITIVE::label, cs);
        TYPED_EXAMPLE_PREDICT_CASE(CB::label, cb);
        TYPED_EXAMPLE_PREDICT_CASE(CCB::label, conditional_contextual_bandit);
        TYPED_EXAMPLE_PREDICT_CASE(CB_EVAL::label, cb_eval);
        TYPED_EXAMPLE_PREDICT_CASE(MULTILABEL::labels, multilabels);
        default:
          THROW("Unknown label type");
          break;
      }
    }
    else
    {
      learn_fd.predict_f(learn_fd.data, *learn_fd.base, static_cast<void*>(&ec));
    }
    decrement_offset(ec, increment, i);
  }

  inline void predict(multi_ex& ec, size_t i = 0)
  {
    assert(is_multiline);
    increment_offset(ec, increment, i);
    if (learn_fd.predict_with_label_f != nullptr)
    {
      switch (learn_fd.base->label_type)
      {
        TYPED_MULTI_EXAMPLE_PREDICT_CASE(no_label::no_label, empty);
        TYPED_MULTI_EXAMPLE_PREDICT_CASE(label_data, simple);
        TYPED_MULTI_EXAMPLE_PREDICT_CASE(MULTICLASS::label_t, multi);
        TYPED_MULTI_EXAMPLE_PREDICT_CASE(COST_SENSITIVE::label, cs);
        TYPED_MULTI_EXAMPLE_PREDICT_CASE(CB::label, cb);
        TYPED_MULTI_EXAMPLE_PREDICT_CASE(CCB::label, conditional_contextual_bandit);
        TYPED_MULTI_EXAMPLE_PREDICT_CASE(CB_EVAL::label, cb_eval);
        TYPED_MULTI_EXAMPLE_PREDICT_CASE(MULTILABEL::labels, multilabels);
        default:
          THROW("Unknown label type");
          break;
      }
    }
    else
    {
      learn_fd.predict_f(learn_fd.data, *learn_fd.base, static_cast<void*>(&ec));
    }
    decrement_offset(ec, increment, i);
  }

  inline void multipredict(E& ec, size_t lo, size_t count, polyprediction* pred, bool finalize_predictions)
  {
    assert((is_multiline && std::is_same<multi_ex, E>::value) ||
        (!is_multiline && std::is_same<example, E>::value));  // sanity check under debug compile
    if (learn_fd.multipredict_f == NULL)
    {
      increment_offset(ec, increment, lo);
      for (size_t c = 0; c < count; c++)
      {
        learn_fd.predict_f(learn_fd.data, *learn_fd.base, static_cast<void*>(&ec));
        if (finalize_predictions)
          pred[c] = ec.pred;  // TODO: this breaks for complex labels because = doesn't do deep copy!
        else
          pred[c].scalar = ec.partial_prediction;
        // pred[c].scalar = finalize_prediction ec.partial_prediction; // TODO: this breaks for complex labels because =
        // doesn't do deep copy! // note works if ec.partial_prediction, but only if finalize_prediction is run????
        increment_offset(ec, increment, 1);
      }
      decrement_offset(ec, increment, lo + count);
    }
    else
    {
      increment_offset(ec, increment, lo);
      learn_fd.multipredict_f(
          learn_fd.data, *learn_fd.base, static_cast<void*>(&ec), count, increment, pred, finalize_predictions);
      decrement_offset(ec, increment, lo);
    }
  }

  template <class L, typename LabelT>
  inline void set_predict(void (*u)(T&, L&, E&, LabelT&))
  {
    learn_fd.predict_with_label_f = (learn_data::fn_with_label)u;
  }
  template <class L>
  inline void set_predict(void (*u)(T&, L&, E&))
  {
    learn_fd.predict_f = (learn_data::fn)u;
  }
  template <class L, typename LabelT>
  inline void set_learn(void (*u)(T&, L&, E&, LabelT&))
  {
    learn_fd.learn_with_label_f = (learn_data::fn_with_label)u;
  }
  template <class L>
  inline void set_learn(void (*u)(T&, L&, E&))
  {
    learn_fd.learn_f = (learn_data::fn)u;
  }
  template <class L>
  inline void set_multipredict(void (*u)(T&, L&, E&, size_t, size_t, polyprediction*, bool))
  {
    learn_fd.multipredict_f = (learn_data::multi_fn)u;
  }

  inline void update(E& ec, size_t i = 0)
  {
    assert((is_multiline && std::is_same<multi_ex, E>::value) ||
        (!is_multiline && std::is_same<example, E>::value));  // sanity check under debug compile
    increment_offset(ec, increment, i);
    learn_fd.update_f(learn_fd.data, *learn_fd.base, static_cast<void*>(&ec));
    decrement_offset(ec, increment, i);
  }
  template <class L>
  inline void set_update(void (*u)(T& data, L& base, E&))
  {
    learn_fd.update_f = (learn_data::fn)u;
  }

  // used for active learning and confidence to determine how easily predictions are changed
  inline void set_sensitivity(float (*u)(T& data, base_learner& base, example&))
  {
    sensitivity_fd.data = learn_fd.data;
    sensitivity_fd.sensitivity_f = (sensitivity_data::fn)u;
  }
  inline float sensitivity(example& ec, size_t i = 0)
  {
    increment_offset(ec, increment, i);
    const float ret = sensitivity_fd.sensitivity_f(sensitivity_fd.data, *learn_fd.base, ec);
    decrement_offset(ec, increment, i);
    return ret;
  }

  // called anytime saving or loading needs to happen. Autorecursive.
  inline void save_load(io_buf& io, const bool read, const bool text)
  {
    save_load_fd.save_load_f(save_load_fd.data, io, read, text);
    if (save_load_fd.base)
      save_load_fd.base->save_load(io, read, text);
  }
  inline void set_save_load(void (*sl)(T&, io_buf&, bool, bool))
  {
    save_load_fd.save_load_f = (save_load_data::fn)sl;
    save_load_fd.data = learn_fd.data;
    save_load_fd.base = learn_fd.base;
  }

  // called to clean up state.  Autorecursive.
  void set_finish(void (*f)(T&)) { finisher_fd = tuple_dbf(learn_fd.data, learn_fd.base, (finish_fptr_type)(f)); }
  inline void finish()
  {
    if (finisher_fd.data)
    {
      finisher_fd.func(finisher_fd.data);
    }
    learner_data.~shared_ptr<void>();
    if (finisher_fd.base)
    {
      finisher_fd.base->finish();
      free(finisher_fd.base);
    }
  }

  void end_pass()
  {
    end_pass_fd.func(end_pass_fd.data);
    if (end_pass_fd.base)
      end_pass_fd.base->end_pass();
  }  // autorecursive
  void set_end_pass(void (*f)(T&)) { end_pass_fd = tuple_dbf(learn_fd.data, learn_fd.base, (func_data::fn)f); }

  // called after parsing of examples is complete.  Autorecursive.
  void end_examples()
  {
    end_examples_fd.func(end_examples_fd.data);
    if (end_examples_fd.base)
      end_examples_fd.base->end_examples();
  }
  void set_end_examples(void (*f)(T&)) { end_examples_fd = tuple_dbf(learn_fd.data, learn_fd.base, (func_data::fn)f); }

  // Called at the beginning by the driver.  Explicitly not recursive.
  void init_driver() { init_fd.func(init_fd.data); }
  void set_init_driver(void (*f)(T&)) { init_fd = tuple_dbf(learn_fd.data, learn_fd.base, (func_data::fn)f); }

  // called after learn example for each example.  Explicitly not recursive.
  inline void finish_example(vw& all, E& ec)
  {
    finish_example_fd.finish_example_f(all, finish_example_fd.data, static_cast<void*>(&ec));
  }
  // called after learn example for each example.  Explicitly not recursive.
  void set_finish_example(void (*f)(vw& all, T&, E&))
  {
    finish_example_fd.data = learn_fd.data;
    finish_example_fd.finish_example_f = (end_fptr_type)(f);
  }

  template <class L>
  static learner<T, E>& init_learner(
      T* dat, L* base, void (*learn)(T&, L&, E&), void (*predict)(T&, L&, E&), size_t ws, prediction_type_t pred_type)
  {
    learner<T, E>& ret = calloc_or_throw<learner<T, E> >();

    if (base != nullptr)
    {  // a reduction

      // This is a copy assignment into the current object. The purpose is to copy all of the
      // function data objects so that if this reduction does not define a function such as
      // save_load then calling save_load on this object will essentially result in forwarding the
      // call the next reduction that actually implements it.
      ret = *(learner<T, E>*)(base);

      ret.learn_fd.base = make_base(*base);
      ret.sensitivity_fd.sensitivity_f = (sensitivity_data::fn)recur_sensitivity;
      ret.finisher_fd.data = dat;
      ret.finisher_fd.base = make_base(*base);
      ret.finisher_fd.func = (func_data::fn)noop;
      ret.weights = ws;
      ret.increment = base->increment * ret.weights;
    }
    else  // a base learner
    {
      ret.weights = 1;
      ret.increment = ws;
      ret.end_pass_fd.func = (func_data::fn)noop;
      ret.end_examples_fd.func = (func_data::fn)noop;
      ret.init_fd.func = (func_data::fn)noop;
      ret.save_load_fd.save_load_f = (save_load_data::fn)noop_sl;
      ret.finisher_fd.data = dat;
      ret.finisher_fd.func = (func_data::fn)noop;
      ret.sensitivity_fd.sensitivity_f = (sensitivity_data::fn)noop_sensitivity;
      ret.finish_example_fd.data = dat;
      ret.finish_example_fd.finish_example_f = (finish_example_data::fn)return_simple_example;
    }

    ret.learner_data = std::shared_ptr<T>(dat, [](T* ptr) {
      ptr->~T();
      free(ptr);
    });

    ret.learn_fd.data = dat;
    ret.learn_fd.learn_f = (learn_data::fn)learn;
    ret.learn_fd.update_f = (learn_data::fn)learn;
    ret.learn_fd.predict_f = (learn_data::fn)predict;
    ret.learn_fd.multipredict_f = nullptr;
    ret.pred_type = pred_type;
    ret.is_multiline = std::is_same<multi_ex, E>::value;

    return ret;
  }
};

template <class T, class E, class L>
learner<T, E>& init_learner(free_ptr<T>& dat, L* base, void (*learn)(T&, L&, E&), void (*predict)(T&, L&, E&),
    size_t ws, prediction_type_t pred_type)
{
  auto ret = &learner<T, E>::init_learner(dat.get(), base, learn, predict, ws, pred_type);

  dat.release();
  return *ret;
}

// base learner/predictor
template <class T, class E, class L>
learner<T, E>& init_learner(
    free_ptr<T>& dat, void (*learn)(T&, L&, E&), void (*predict)(T&, L&, E&), size_t params_per_weight)
{
  auto ret = &learner<T, E>::init_learner(
      dat.get(), (L*)nullptr, learn, predict, params_per_weight, prediction_type_t::scalar);

  dat.release();
  return *ret;
}

// base predictor only
template <class T, class E, class L>
learner<T, E>& init_learner(void (*predict)(T&, L&, E&), size_t params_per_weight)
{
  return learner<T, E>::init_learner(
      nullptr, (L*)nullptr, predict, predict, params_per_weight, prediction_type_t::scalar);
}

template <class T, class E, class L>
learner<T, E>& init_learner(free_ptr<T>& dat, void (*learn)(T&, L&, E&), void (*predict)(T&, L&, E&),
    size_t params_per_weight, prediction_type_t pred_type)
{
  auto ret = &learner<T, E>::init_learner(dat.get(), (L*)nullptr, learn, predict, params_per_weight, pred_type);
  dat.release();
  return *ret;
}

// reduction with default prediction type
template <class T, class E, class L>
learner<T, E>& init_learner(
    free_ptr<T>& dat, L* base, void (*learn)(T&, L&, E&), void (*predict)(T&, L&, E&), size_t ws)
{
  auto ret = &learner<T, E>::init_learner(dat.get(), base, learn, predict, ws, base->pred_type);

  dat.release();
  return *ret;
}

// reduction with default num_params
template <class T, class E, class L>
learner<T, E>& init_learner(free_ptr<T>& dat, L* base, void (*learn)(T&, L&, E&), void (*predict)(T&, L&, E&))
{
  auto ret = &learner<T, E>::init_learner(dat.get(), base, learn, predict, 1, base->pred_type);

  dat.release();
  return *ret;
}

// Reduction with no data.
template <class T, class E, class L>
learner<T, E>& init_learner(L* base, void (*learn)(T&, L&, E&), void (*predict)(T&, L&, E&))
{
  return learner<T, E>::init_learner(nullptr, base, learn, predict, 1, base->pred_type);
}

// multiclass reduction
template <class T, class E, class L>
learner<T, E>& init_multiclass_learner(free_ptr<T>& dat, L* base, void (*learn)(T&, L&, E&),
    void (*predict)(T&, L&, E&), parser* p, size_t ws, prediction_type_t pred_type = prediction_type_t::multiclass)
{
  learner<T, E>& l = learner<T, E>::init_learner(dat.get(), base, learn, predict, ws, pred_type);

  dat.release();
  l.set_finish_example(MULTICLASS::finish_example<T>);
  p->lp = MULTICLASS::mc_label;
  return l;
}

template <class T, class E, class L>
learner<T, E>& init_cost_sensitive_learner(free_ptr<T>& dat, L* base, void (*learn)(T&, L&, E&),
    void (*predict)(T&, L&, E&), parser* p, size_t ws, prediction_type_t pred_type = prediction_type_t::multiclass)
{
  learner<T, E>& l = learner<T, E>::init_learner(dat.get(), base, learn, predict, ws, pred_type);
  dat.release();
  l.set_finish_example(COST_SENSITIVE::finish_example);
  p->lp = COST_SENSITIVE::cs_label;
  return l;
}

template <class T, class E>
base_learner* make_base(learner<T, E>& base)
{
  return (base_learner*)(&base);
}

template <class T, class E>
multi_learner* as_multiline(learner<T, E>* l)
{
  if (l->is_multiline)  // Tried to use a singleline reduction as a multiline reduction
    return (multi_learner*)(l);
  THROW("Tried to use a singleline reduction as a multiline reduction");
}

template <class T, class E>
single_learner* as_singleline(learner<T, E>* l)
{
  if (!l->is_multiline)  // Tried to use a multiline reduction as a singleline reduction
    return (single_learner*)(l);
  THROW("Tried to use a multiline reduction as a singleline reduction");
}

template <bool is_learn>
void multiline_learn_or_predict(multi_learner& base, multi_ex& examples, const uint64_t offset, const uint32_t id = 0)
{
  std::vector<uint64_t> saved_offsets;
  saved_offsets.reserve(examples.size());
  for (auto ec : examples)
  {
    saved_offsets.push_back(ec->ft_offset);
    ec->ft_offset = offset;
  }

  if (is_learn)
    base.learn(examples, id);
  else
    base.predict(examples, id);

  for (size_t i = 0; i < examples.size(); i++) examples[i]->ft_offset = saved_offsets[i];
}

template <bool is_learn, typename LabelT>
void multiline_learn_or_predict_with_labels(multi_learner& base, multi_ex& examples, LabelT& labels, const uint64_t offset, const uint32_t id = 0)
{
  std::vector<uint64_t> saved_offsets;
  saved_offsets.reserve(examples.size());
  for (auto ec : examples)
  {
    saved_offsets.push_back(ec->ft_offset);
    ec->ft_offset = offset;
  }

  if (is_learn)
    base.learn_with_label(examples, labels, id);
  else
    base.predict_with_label(examples, labels, id);

  for (size_t i = 0; i < examples.size(); i++) examples[i]->ft_offset = saved_offsets[i];
}
}  // namespace LEARNER
