/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#pragma once
// This is the interface for a learning algorithm
#include <iostream>
#include "memory.h"
#include "multiclass.h"
#include "simple_label.h"
#include "parser.h"

#include <memory>

namespace prediction_type
{
enum prediction_type_t
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
}  // namespace prediction_type

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
  using multi_fn = void (*)(void* data, base_learner& base, void* ex, size_t count, size_t step, polyprediction* pred,
      bool finalize_predictions);

  void* data;
  base_learner* base;
  fn learn_f;
  fn predict_f;
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
  prediction_type::prediction_type_t pred_type;
  size_t weights;  // this stores the number of "weight vectors" required by the learner.
  size_t increment;
  bool is_multiline;  // Is this a single-line or multi-line reduction?

  using end_fptr_type = void (*)(vw&, void*, void*);
  using finish_fptr_type = void (*)(void*);

  // called once for each example.  Must work under reduction.
  inline void learn(E& ec, size_t i = 0)
  {
    assert((is_multiline && std::is_same<multi_ex, E>::value) ||
        (!is_multiline && std::is_same<example, E>::value));  // sanity check under debug compile
    increment_offset(ec, increment, i);
    learn_fd.learn_f(learn_fd.data, *learn_fd.base, (void*)&ec);
    decrement_offset(ec, increment, i);
  }

  inline void predict(E& ec, size_t i = 0)
  {
    assert((is_multiline && std::is_same<multi_ex, E>::value) ||
        (!is_multiline && std::is_same<example, E>::value));  // sanity check under debug compile
    increment_offset(ec, increment, i);
    learn_fd.predict_f(learn_fd.data, *learn_fd.base, (void*)&ec);
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
        learn_fd.predict_f(learn_fd.data, *learn_fd.base, (void*)&ec);
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
      learn_fd.multipredict_f(learn_fd.data, *learn_fd.base, (void*)&ec, count, increment, pred, finalize_predictions);
      decrement_offset(ec, increment, lo);
    }
  }

  template <class L>
  inline void set_predict(void (*u)(T&, L&, E&))
  {
    learn_fd.predict_f = (learn_data::fn)u;
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
    learn_fd.update_f(learn_fd.data, *learn_fd.base, (void*)&ec);
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
    finish_example_fd.finish_example_f(all, finish_example_fd.data, (void*)&ec);
  }
  // called after learn example for each example.  Explicitly not recursive.
  void set_finish_example(void (*f)(vw& all, T&, E&))
  {
    finish_example_fd.data = learn_fd.data;
    finish_example_fd.finish_example_f = (end_fptr_type)(f);
  }

  template <class L>
  static learner<T, E>& init_learner(T* dat, L* base, void (*learn)(T&, L&, E&), void (*predict)(T&, L&, E&), size_t ws,
      prediction_type::prediction_type_t pred_type)
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
    size_t ws, prediction_type::prediction_type_t pred_type)
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
  auto ret =
      &learner<T, E>::init_learner(dat.get(), (L*)nullptr, learn, predict, params_per_weight, prediction_type::scalar);

  dat.release();
  return *ret;
}

// base predictor only
template <class T, class E, class L>
learner<T, E>& init_learner(void (*predict)(T&, L&, E&), size_t params_per_weight)
{
  return learner<T, E>::init_learner(
      nullptr, (L*)nullptr, predict, predict, params_per_weight, prediction_type::scalar);
}

template <class T, class E, class L>
learner<T, E>& init_learner(free_ptr<T>& dat, void (*learn)(T&, L&, E&), void (*predict)(T&, L&, E&),
    size_t params_per_weight, prediction_type::prediction_type_t pred_type)
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
    void (*predict)(T&, L&, E&), parser* p, size_t ws,
    prediction_type::prediction_type_t pred_type = prediction_type::multiclass)
{
  learner<T, E>& l = learner<T, E>::init_learner(dat.get(), base, learn, predict, ws, pred_type);

  dat.release();
  l.set_finish_example(MULTICLASS::finish_example<T>);
  p->lp = MULTICLASS::mc_label;
  return l;
}

template <class T, class E, class L>
learner<T, E>& init_cost_sensitive_learner(free_ptr<T>& dat, L* base, void (*learn)(T&, L&, E&),
    void (*predict)(T&, L&, E&), parser* p, size_t ws,
    prediction_type::prediction_type_t pred_type = prediction_type::multiclass)
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
}  // namespace LEARNER
