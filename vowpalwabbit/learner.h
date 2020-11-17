// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once
// This is the interface for a learning algorithm

#include <iostream>
#include <memory>

#include "memory.h"
#include "multiclass.h"
#include "simple_label.h"
#include "parser.h"
#include "debug_log.h"

#undef VW_DEBUG_LOG
#define VW_DEBUG_LOG vw_dbg::learner

#include "future_compat.h"
#include "example.h"
#include <memory>
#include "scope_exit.h"

enum class prediction_type_t
{
  scalar,
  scalars,
  action_scores,
  pdf,
  action_probs,
  multiclass,
  multilabels,
  prob,
  multiclassprobs,
  decision_probs,
  action_pdf_value
};

const char* to_string(prediction_type_t prediction_type);

namespace VW
{
/// \brief Contains the VW::LEARNER::learner object and utilities for
/// interacting with it.
namespace LEARNER
{
template <class T, class E>
struct learner;

/// \brief Used to type erase the object and pass around common type.
using base_learner = learner<char, char>;

/// \brief Used for reductions that process single ::example objects at at time.
/// It type erases the specific reduction object type.
using single_learner = learner<char, example>;

/// \brief Used for multiline examples where there are several ::example objects
/// required to describe the overall example. It type erases the specific
/// reduction object type.
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

inline void increment_depth(example& ex)
{
  if (vw_dbg::track_stack) ++ex.stack_depth;
}

inline void increment_depth(multi_ex& ec_seq)
{
  if (vw_dbg::track_stack)
  {
    for (auto& ec : ec_seq) { ++ec->stack_depth; }
  }
}

inline void decrement_depth(example& ex)
{
  if (vw_dbg::track_stack) --ex.stack_depth;
}

inline void decrement_depth(multi_ex& ec_seq)
{
  if (vw_dbg::track_stack)
  {
    for (auto& ec : ec_seq) { --ec->stack_depth; }
  }
}

inline void increment_offset(example& ex, const size_t increment, const size_t i)
{
  ++ex._current_reduction_depth;
  ex.ft_offset += static_cast<uint32_t>(increment * i);
  increment_depth(ex);
}

inline void increment_offset(multi_ex& ec_seq, const size_t increment, const size_t i)
{
  if (ec_seq.size() > 0) increment_depth(ec_seq);
  for (auto& ec : ec_seq) { ec->ft_offset += static_cast<uint32_t>(increment * i); }
}

inline void decrement_offset(example& ex, const size_t increment, const size_t i)
{
  --ex._current_reduction_depth;
  assert(ex.ft_offset >= increment * i);
  ex.ft_offset -= static_cast<uint32_t>(increment * i);
  decrement_depth(ex);
}

inline void decrement_offset(multi_ex& ec_seq, const size_t increment, const size_t i)
{
  if (ec_seq.size() > 0) --ec_seq[0]->_current_reduction_depth;
  for (auto ec : ec_seq)
  {
    assert(ec->ft_offset >= increment * i);
    ec->ft_offset -= static_cast<uint32_t>(increment * i);
    decrement_depth(*ec);
  }
}

/// \brief Defines the interface for a learning algorithm.
///
/// Learner is implemented as a struct of pointers, and associated methods. It
/// implements a sort of virtual inheritance through the use of bundling
/// function pointers with the associated objects to call them with. A reduction
/// will recursively call the base given to it, whereas a base learner will not
/// recurse and will simply return the result. Learner is not intended to be
/// inherited from. Instead it is used through composition, and created through
/// the various VW::LEARNER::init_learner overloaded functions that chain to the
/// central factor function VW::LEARNER::learner::init_learner The state of this
/// learner, or reduction, is stored in the learner_data field. A
/// <code>std::shared_pointer<void></code> is used as this class uses type
/// erasure to allow for an arbitrary reduction to be implemented. It is
/// extremely important that the function pointers given to the class match the
/// expected types of the object. If the learner is constructed using
/// VW::LEARNER::learner::init_learner and assembled before it is transformed
/// into a VW::LEARNER::base_learner with VW::LEARNER::make_base then the usage
/// of the templated functions should ensure types are correct.
///
/// \tparam T Type of the reduction data object stored. This allows this
/// specific reduction to have it's own state.
/// \tparam E Example type this reduction supports. Must be one of ::example or
/// ::multi_ex
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
  size_t weights;  // this stores the number of "weight vectors" required by the learner.
  size_t increment;
  bool is_multiline;  // Is this a single-line or multi-line reduction?
  std::string name;   // Name of the reduction.  Used in VW_DBG to trace nested learn() and predict() calls

  // learn will return a prediction.  The framework should
  // not call predict before learn
  bool learn_returns_prediction;

  using end_fptr_type = void (*)(vw&, void*, void*);
  using finish_fptr_type = void (*)(void*);

  void log_entry(example& ec, const std::string& msg)
  {
    VW_DBG(ec) << "[" << name << "." << msg << "]" << std::endl;
    increment_depth(ec);
  }
  void log_entry(multi_ex& ec, const std::string& msg)
  {
    VW_DBG(*ec[0]) << "[" << name << "." << msg << "]" << std::endl;
    increment_depth(ec);
  }
  void log_exit(example& ec)
  {
    decrement_depth(ec);
    VW_DBG(ec) << std::endl;
  }
  void log_exit(multi_ex& ec)
  {
    decrement_depth(ec);
    VW_DBG(*ec[0]) << std::endl;
  }

  /// \brief Will update the model according to the labels and examples supplied.
  /// \param ec The ::example object or ::multi_ex to be operated on. This
  /// object **must** have a valid label set for every ::example in the field
  /// example::l that corresponds to the type this reduction expects.
  /// \param i This is the offset used for the weights in this call. If using
  /// multiple regressors/learners you can increment this value for each call.
  /// \returns While some reductions may fill the example::pred, this is not
  /// guaranteed and is undefined behavior if accessed.
  inline void learn(E& ec, size_t i = 0)
  {
    assert((is_multiline && std::is_same<multi_ex, E>::value) ||
        (!is_multiline && std::is_same<example, E>::value));  // sanity check under debug compile
    increment_offset(ec, increment, i);
    log_entry(ec, "learn");
    learn_fd.learn_f(learn_fd.data, *learn_fd.base, (void*)&ec);
    log_exit(ec);
    decrement_offset(ec, increment, i);
  }

  /// \brief Make a prediction for the given example.
  /// \param ec The ::example object or ::multi_ex to be operated on. This
  /// object **must** have a valid prediction allocated in the field
  /// example::pred that corresponds to this reduction type.
  /// \param i This is the offset used for the weights in this call. If using
  /// multiple regressors/learners you can increment this value for each call.
  /// \returns The prediction calculated by this reduction be set on
  /// example::pred. If <code>E</code> is ::multi_ex then the prediction is set
  /// on the 0th item in the list.
  inline void predict(E& ec, size_t i = 0)
  {
    assert((is_multiline && std::is_same<multi_ex, E>::value) ||
        (!is_multiline && std::is_same<example, E>::value));  // sanity check under debug compile
    increment_offset(ec, increment, i);
    log_entry(ec, "predict");
    learn_fd.predict_f(learn_fd.data, *learn_fd.base, (void*)&ec);
    log_exit(ec);
    decrement_offset(ec, increment, i);
  }

  inline void multipredict(E& ec, size_t lo, size_t count, polyprediction* pred, bool finalize_predictions)
  {
    assert((is_multiline && std::is_same<multi_ex, E>::value) ||
        (!is_multiline && std::is_same<example, E>::value));  // sanity check under debug compile
    if (learn_fd.multipredict_f == NULL)
    {
      increment_offset(ec, increment, lo);
      log_entry(ec, "multipredict");
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
      log_exit(ec);
      decrement_offset(ec, increment, lo + count);
    }
    else
    {
      increment_offset(ec, increment, lo);
      log_entry(ec, "multipredict");
      learn_fd.multipredict_f(learn_fd.data, *learn_fd.base, (void*)&ec, count, increment, pred, finalize_predictions);
      log_exit(ec);
      decrement_offset(ec, increment, lo);
    }
  }

  template <class L>
  inline void set_predict(void (*u)(T&, L&, E&))
  {
    VW_WARNING_STATE_PUSH
    VW_WARNING_DISABLE_CAST_FUNC_TYPE
    learn_fd.predict_f = (learn_data::fn)u;
    VW_WARNING_STATE_POP
  }
  template <class L>
  inline void set_learn(void (*u)(T&, L&, E&))
  {
    VW_WARNING_STATE_PUSH
    VW_WARNING_DISABLE_CAST_FUNC_TYPE
    learn_fd.learn_f = (learn_data::fn)u;
    VW_WARNING_STATE_POP
  }
  template <class L>
  inline void set_multipredict(void (*u)(T&, L&, E&, size_t, size_t, polyprediction*, bool))
  {
    VW_WARNING_STATE_PUSH
    VW_WARNING_DISABLE_CAST_FUNC_TYPE
    learn_fd.multipredict_f = (learn_data::multi_fn)u;
    VW_WARNING_STATE_POP
  }

  inline void update(E& ec, size_t i = 0)
  {
    assert((is_multiline && std::is_same<multi_ex, E>::value) ||
        (!is_multiline && std::is_same<example, E>::value));  // sanity check under debug compile
    increment_offset(ec, increment, i);
    log_entry(ec, "update");
    learn_fd.update_f(learn_fd.data, *learn_fd.base, (void*)&ec);
    log_exit(ec);
    decrement_offset(ec, increment, i);
  }
  template <class L>
  inline void set_update(void (*u)(T& data, L& base, E&))
  {
    VW_WARNING_STATE_PUSH
    VW_WARNING_DISABLE_CAST_FUNC_TYPE
    learn_fd.update_f = (learn_data::fn)u;
    VW_WARNING_STATE_POP
  }

  // used for active learning and confidence to determine how easily predictions are changed
  inline void set_sensitivity(float (*u)(T& data, base_learner& base, example&))
  {
    sensitivity_fd.data = learn_fd.data;
    VW_WARNING_STATE_PUSH
    VW_WARNING_DISABLE_CAST_FUNC_TYPE
    sensitivity_fd.sensitivity_f = (sensitivity_data::fn)u;
    VW_WARNING_STATE_POP
  }
  inline float sensitivity(example& ec, size_t i = 0)
  {
    increment_offset(ec, increment, i);
    log_entry(ec, "sensitivity");
    const float ret = sensitivity_fd.sensitivity_f(sensitivity_fd.data, *learn_fd.base, ec);
    log_exit(ec);
    decrement_offset(ec, increment, i);
    return ret;
  }

  // called anytime saving or loading needs to happen. Autorecursive.
  inline void save_load(io_buf& io, const bool read, const bool text)
  {
    save_load_fd.save_load_f(save_load_fd.data, io, read, text);
    if (save_load_fd.base) save_load_fd.base->save_load(io, read, text);
  }
  inline void set_save_load(void (*sl)(T&, io_buf&, bool, bool))
  {
    VW_WARNING_STATE_PUSH
    VW_WARNING_DISABLE_CAST_FUNC_TYPE
    save_load_fd.save_load_f = (save_load_data::fn)sl;
    VW_WARNING_STATE_POP
    save_load_fd.data = learn_fd.data;
    save_load_fd.base = learn_fd.base;
  }

  // called to clean up state.  Autorecursive.
  void set_finish(void (*f)(T&))
  {
    VW_WARNING_STATE_PUSH
    VW_WARNING_DISABLE_CAST_FUNC_TYPE
    finisher_fd = tuple_dbf(learn_fd.data, learn_fd.base, (finish_fptr_type)(f));
    VW_WARNING_STATE_POP
  }

  inline void finish()
  {
    if (finisher_fd.data) { finisher_fd.func(finisher_fd.data); }
    learner_data.~shared_ptr<void>();
    if (finisher_fd.base)
    {
      finisher_fd.base->finish();
      free(finisher_fd.base);
    }
    name.~basic_string();
  }

  void end_pass()
  {
    end_pass_fd.func(end_pass_fd.data);
    if (end_pass_fd.base) end_pass_fd.base->end_pass();
  }  // autorecursive

  void set_end_pass(void (*f)(T&))
  {
    VW_WARNING_STATE_PUSH
    VW_WARNING_DISABLE_CAST_FUNC_TYPE
    end_pass_fd = tuple_dbf(learn_fd.data, learn_fd.base, (func_data::fn)f);
    VW_WARNING_STATE_POP
  }

  // called after parsing of examples is complete.  Autorecursive.
  void end_examples()
  {
    end_examples_fd.func(end_examples_fd.data);
    if (end_examples_fd.base) end_examples_fd.base->end_examples();
  }
  void set_end_examples(void (*f)(T&))
  {
    VW_WARNING_STATE_PUSH
    VW_WARNING_DISABLE_CAST_FUNC_TYPE
    end_examples_fd = tuple_dbf(learn_fd.data, learn_fd.base, (func_data::fn)f);
    VW_WARNING_STATE_POP
  }

  // Called at the beginning by the driver.  Explicitly not recursive.
  void init_driver() { init_fd.func(init_fd.data); }
  void set_init_driver(void (*f)(T&))
  {
    VW_WARNING_STATE_PUSH
    VW_WARNING_DISABLE_CAST_FUNC_TYPE
    init_fd = tuple_dbf(learn_fd.data, learn_fd.base, (func_data::fn)f);
    VW_WARNING_STATE_POP
  }

  // called after learn example for each example.  Explicitly not recursive.
  inline void finish_example(vw& all, E& ec)
  {
    log_entry(ec, "finish_example");
    finish_example_fd.finish_example_f(all, finish_example_fd.data, (void*)&ec);
    log_exit(ec);
  }
  // called after learn example for each example.  Explicitly not recursive.
  void set_finish_example(void (*f)(vw& all, T&, E&))
  {
    finish_example_fd.data = learn_fd.data;
    VW_WARNING_STATE_PUSH
    VW_WARNING_DISABLE_CAST_FUNC_TYPE
    finish_example_fd.finish_example_f = (end_fptr_type)(f);
    VW_WARNING_STATE_POP
  }

  template <class L>
  static learner<T, E>& init_learner(T* dat, L* base, void (*learn)(T&, L&, E&), void (*predict)(T&, L&, E&), size_t ws,
      prediction_type_t pred_type, const std::string name, bool learn_returns_prediction = false)
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
      VW_WARNING_STATE_PUSH
      VW_WARNING_DISABLE_CAST_FUNC_TYPE
      ret.finish_example_fd.finish_example_f = (finish_example_data::fn)return_simple_example;
      VW_WARNING_STATE_POP
    }

    ret.name = name;
    ret.learner_data = std::shared_ptr<T>(dat, [](T* ptr) {
      ptr->~T();
      free(ptr);
    });

    ret.learn_fd.data = dat;
    VW_WARNING_STATE_PUSH
    VW_WARNING_DISABLE_CAST_FUNC_TYPE
    ret.learn_fd.learn_f = reinterpret_cast<learn_data::fn>(learn);
    ret.learn_fd.update_f = (learn_data::fn)learn;
    ret.learn_fd.predict_f = (learn_data::fn)predict;
    VW_WARNING_STATE_POP
    ret.learn_fd.multipredict_f = nullptr;
    ret.pred_type = pred_type;
    ret.is_multiline = std::is_same<multi_ex, E>::value;
    ret.learn_returns_prediction = learn_returns_prediction;

    VW_DBG_0 << "Added Reduction: " << name << std::endl;

    return ret;
  }
};

template <class T, class E, class L>
learner<T, E>& init_learner(free_ptr<T>& dat, L* base, void (*learn)(T&, L&, E&), void (*predict)(T&, L&, E&),
    size_t ws, prediction_type_t pred_type, const std::string& name, bool learn_returns_prediction = false)
{
  auto ret =
      &learner<T, E>::init_learner(dat.get(), base, learn, predict, ws, pred_type, name, learn_returns_prediction);

  dat.release();
  return *ret;
}

// base learner/predictor
template <class T, class E, class L>
learner<T, E>& init_learner(free_ptr<T>& dat, void (*learn)(T&, L&, E&), void (*predict)(T&, L&, E&),
    size_t params_per_weight, const std::string& name, bool learn_returns_prediction = false)
{
  auto ret = &learner<T, E>::init_learner(dat.get(), (L*)nullptr, learn, predict, params_per_weight,
      prediction_type_t::scalar, name, learn_returns_prediction);

  dat.release();
  return *ret;
}

// base predictor only
template <class T, class E, class L>
learner<T, E>& init_learner(void (*predict)(T&, L&, E&), size_t params_per_weight, const std::string& name)
{
  return learner<T, E>::init_learner(
      nullptr, (L*)nullptr, predict, predict, params_per_weight, prediction_type_t::scalar, name);
}

template <class T, class E, class L>
learner<T, E>& init_learner(free_ptr<T>& dat, void (*learn)(T&, L&, E&), void (*predict)(T&, L&, E&),
    size_t params_per_weight, prediction_type_t pred_type, const std::string& name,
    bool learn_returns_prediction = false)
{
  auto ret = &learner<T, E>::init_learner(
      dat.get(), (L*)nullptr, learn, predict, params_per_weight, pred_type, name, learn_returns_prediction);
  dat.release();
  return *ret;
}

// reduction with default prediction type
template <class T, class E, class L>
learner<T, E>& init_learner(free_ptr<T>& dat, L* base, void (*learn)(T&, L&, E&), void (*predict)(T&, L&, E&),
    size_t ws, const std::string& name, bool learn_returns_prediction = false)
{
  auto ret = &learner<T, E>::init_learner(
      dat.get(), base, learn, predict, ws, base->pred_type, name, learn_returns_prediction);

  dat.release();
  return *ret;
}

// reduction with default num_params
template <class T, class E, class L>
learner<T, E>& init_learner(free_ptr<T>& dat, L* base, void (*learn)(T&, L&, E&), void (*predict)(T&, L&, E&),
    const std::string& name, bool learn_returns_prediction = false)
{
  auto ret =
      &learner<T, E>::init_learner(dat.get(), base, learn, predict, 1, base->pred_type, name, learn_returns_prediction);

  dat.release();
  return *ret;
}

// Reduction with no data.
template <class T, class E, class L>
learner<T, E>& init_learner(L* base, void (*learn)(T&, L&, E&), void (*predict)(T&, L&, E&), const std::string& name,
    bool learn_returns_prediction = false)
{
  return learner<T, E>::init_learner(nullptr, base, learn, predict, 1, base->pred_type, name, learn_returns_prediction);
}

// multiclass reduction
template <class T, class E, class L>
learner<T, E>& init_multiclass_learner(free_ptr<T>& dat, L* base, void (*learn)(T&, L&, E&),
    void (*predict)(T&, L&, E&), parser* p, size_t ws, const std::string& name,
    prediction_type_t pred_type = prediction_type_t::multiclass, bool learn_returns_prediction = false)
{
  learner<T, E>& l =
      learner<T, E>::init_learner(dat.get(), base, learn, predict, ws, pred_type, name, learn_returns_prediction);

  dat.release();
  l.set_finish_example(MULTICLASS::finish_example<T>);
  p->lbl_parser = MULTICLASS::mc_label;
  return l;
}

template <class T, class E, class L>
learner<T, E>& init_cost_sensitive_learner(free_ptr<T>& dat, L* base, void (*learn)(T&, L&, E&),
    void (*predict)(T&, L&, E&), parser* p, size_t ws, const std::string& name,
    prediction_type_t pred_type = prediction_type_t::multiclass, bool learn_returns_prediction = false)
{
  learner<T, E>& l =
      learner<T, E>::init_learner(dat.get(), base, learn, predict, ws, pred_type, name, learn_returns_prediction);
  dat.release();
  l.set_finish_example(COST_SENSITIVE::finish_example);
  p->lbl_parser = COST_SENSITIVE::cs_label;
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

  // Guard example state restore against throws
  auto restore_guard = VW::scope_exit([&saved_offsets, &examples] {
    for (size_t i = 0; i < examples.size(); i++) { examples[i]->ft_offset = saved_offsets[i]; }
  });

  if (is_learn)
    base.learn(examples, id);
  else
    base.predict(examples, id);
}
}  // namespace LEARNER
}  // namespace VW
