/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#pragma once
// This is the interface for a learning algorithm
#include<iostream>
#include "memory.h"
#include "cb.h"
#include "cost_sensitive.h"
#include "multiclass.h"
#include "simple_label.h"
#include "parser.h"

namespace prediction_type
{
enum prediction_type_t
{ scalar,
  scalars,
  action_scores,
  action_probs,
  multiclass,
  multilabels,
  prob,
  multiclassprobs
};

const char* to_string(prediction_type_t prediction_type);
}

namespace LEARNER
{
template<class T> struct learner;
typedef learner<char> base_learner;

struct func_data
{ void* data;
  base_learner* base;
  void (*func)(void* data);
};

inline func_data tuple_dbf(void* data, base_learner* base, void (*func)(void* data))
{ func_data foo;
  foo.data = data;
  foo.base = base;
  foo.func = func;
  return foo;
}

struct learn_data
{ void* data;
  base_learner* base;
  void (*learn_f)(void* data, base_learner& base, example&);
  void (*predict_f)(void* data, base_learner& base, example&);
  void (*update_f)(void* data, base_learner& base, example&);
  void (*multipredict_f)(void* data, base_learner& base, example&, size_t count, size_t step, polyprediction*pred, bool finalize_predictions);
};

struct sensitivity_data
{ void* data;
  float (*sensitivity_f)(void* data, base_learner& base, example&);
};

struct save_load_data
{ void* data;
  base_learner* base;
  void (*save_load_f)(void*, io_buf&, bool read, bool text);
};

struct finish_example_data
{ void* data;
  base_learner* base;
  void (*finish_example_f)(vw&, void* data, example&);
};

void generic_driver(vw& all);
void generic_driver(std::vector<vw*> alls);

inline void noop_sl(void*, io_buf&, bool, bool) {}
inline void noop(void*) {}
inline float noop_sensitivity(void*, base_learner&, example&) { return 0.; }

typedef void (*tlearn)(void* d, base_learner& base, example& ec);
typedef float (*tsensitivity)(void* d, base_learner& base, example& ec);
typedef void (*tmultipredict)(void* d, base_learner& base, example& ec, size_t, size_t, polyprediction*, bool);
typedef void (*tsl)(void* d, io_buf& io, bool read, bool text);
typedef void (*tfunc)(void*d);
typedef void (*tend_example)(vw& all, void* d, example& ec);

template<class T> learner<T>& init_learner(T*, void (*)(T&, base_learner&, example&), size_t, prediction_type::prediction_type_t pred = prediction_type::scalar);
template<class T>
learner<T>& init_learner(T*, base_learner*, void(*learn)(T&, base_learner&, example&),
                         void(*predict)(T&, base_learner&, example&), size_t ws = 1);
template<class T>
learner<T>& init_learner(T*, base_learner*, void (*learn)(T&, base_learner&, example&),
                         void (*predict)(T&, base_learner&, example&), size_t ws, prediction_type::prediction_type_t);

template<class T>
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

public:
  prediction_type::prediction_type_t pred_type;
  size_t weights; //this stores the number of "weight vectors" required by the learner.
  size_t increment;

  //called once for each example.  Must work under reduction.
  inline void learn(example& ec, size_t i=0)
  { ec.ft_offset += (uint32_t)(increment*i);
    learn_fd.learn_f(learn_fd.data, *learn_fd.base, ec);
    ec.ft_offset -= (uint32_t)(increment*i);
  }
  inline void predict(example& ec, size_t i=0)
  { ec.ft_offset += (uint32_t)(increment*i);
    learn_fd.predict_f(learn_fd.data, *learn_fd.base, ec);
    ec.ft_offset -= (uint32_t)(increment*i);
  }
  inline void multipredict(example& ec, size_t lo, size_t count, polyprediction* pred, bool finalize_predictions)
  { if (learn_fd.multipredict_f == NULL)
    { ec.ft_offset += (uint32_t)(increment*lo);
      for (size_t c=0; c<count; c++)
      { learn_fd.predict_f(learn_fd.data, *learn_fd.base, ec);
        if (finalize_predictions) pred[c] = ec.pred; // TODO: this breaks for complex labels because = doesn't do deep copy!
        else                      pred[c].scalar = ec.partial_prediction;
        //pred[c].scalar = finalize_prediction ec.partial_prediction; // TODO: this breaks for complex labels because = doesn't do deep copy! // note works if ec.partial_prediction, but only if finalize_prediction is run????
        ec.ft_offset += (uint32_t)increment;
      }
      ec.ft_offset -= (uint32_t)(increment*(lo+count));
    }
    else
    { ec.ft_offset += (uint32_t)(increment*lo);
      learn_fd.multipredict_f(learn_fd.data, *learn_fd.base, ec, count, increment, pred, finalize_predictions);
      ec.ft_offset -= (uint32_t)(increment*lo);
    }
  }
  inline void set_predict(void (*u)(T& data, base_learner& base, example&)) { learn_fd.predict_f = (tlearn)u; }
  inline void set_learn(void (*u)(T&, base_learner&, example&)) { learn_fd.learn_f = (tlearn)u; }
  inline void set_multipredict(void (*u)(T&, base_learner&, example&, size_t, size_t, polyprediction*, bool)) { learn_fd.multipredict_f = (tmultipredict)u; }

  inline void update(example& ec, size_t i=0)
  { ec.ft_offset += (uint32_t)(increment*i);
    learn_fd.update_f(learn_fd.data, *learn_fd.base, ec);
    ec.ft_offset -= (uint32_t)(increment*i);
  }
  inline void set_update(void (*u)(T& data, base_learner& base, example&))
  { learn_fd.update_f = (tlearn)u; }

  //used for active learning and confidence to determine how easily predictions are changed
  inline void set_sensitivity(float (*u)(T& data, base_learner& base, example&))
  { sensitivity_fd.data = learn_fd.data;
    sensitivity_fd.sensitivity_f = (tsensitivity)u;
  }
  inline float sensitivity(example& ec, size_t i=0)
  { ec.ft_offset += (uint32_t)(increment*i);
    float ret = sensitivity_fd.sensitivity_f(sensitivity_fd.data, *learn_fd.base, ec);
    ec.ft_offset -= (uint32_t)(increment*i);
    return ret;
  }

  //called anytime saving or loading needs to happen. Autorecursive.
  inline void save_load(io_buf& io, bool read, bool text)
  { save_load_fd.save_load_f(save_load_fd.data, io, read, text);
    if (save_load_fd.base) save_load_fd.base->save_load(io, read, text);
  }
  inline void set_save_load(void (*sl)(T&, io_buf&, bool, bool))
  { save_load_fd.save_load_f = (tsl)sl;
    save_load_fd.data = learn_fd.data;
    save_load_fd.base = learn_fd.base;
  }

  //called to clean up state.  Autorecursive.
  void set_finish(void (*f)(T&))
  { finisher_fd = tuple_dbf(learn_fd.data,learn_fd.base, (tfunc)f); }
  inline void finish()
  { if (finisher_fd.data)
    {finisher_fd.func(finisher_fd.data); free(finisher_fd.data); }
    if (finisher_fd.base)
    { finisher_fd.base->finish();
      free(finisher_fd.base);
    }
  }

  void end_pass()
  { end_pass_fd.func(end_pass_fd.data);
    if (end_pass_fd.base) end_pass_fd.base->end_pass();
  }//autorecursive
  void set_end_pass(void (*f)(T&))
  {end_pass_fd = tuple_dbf(learn_fd.data, learn_fd.base, (tfunc)f);}

  //called after parsing of examples is complete.  Autorecursive.
  void end_examples()
  { end_examples_fd.func(end_examples_fd.data);
    if (end_examples_fd.base) end_examples_fd.base->end_examples();
  }
  void set_end_examples(void (*f)(T&))
  {end_examples_fd = tuple_dbf(learn_fd.data,learn_fd.base, (tfunc)f);}

  //Called at the beginning by the driver.  Explicitly not recursive.
  void init_driver() { init_fd.func(init_fd.data);}
  void set_init_driver(void (*f)(T&))
  { init_fd = tuple_dbf(learn_fd.data,learn_fd.base, (tfunc)f); }

  //called after learn example for each example.  Explicitly not recursive.
  inline void finish_example(vw& all, example& ec)
  { finish_example_fd.finish_example_f(all, finish_example_fd.data, ec);}
  void set_finish_example(void (*f)(vw& all, T&, example&))
  { finish_example_fd.data = learn_fd.data;
    finish_example_fd.finish_example_f = (tend_example)f;
  }

  friend learner<T>& init_learner<>(T*, base_learner*, void(*l)(T&, base_learner&, example&),
                                    void(*pred)(T&, base_learner&, example&), size_t);

  friend learner<T>& init_learner<>(T*, void (*learn)(T&, base_learner&, example&), size_t, prediction_type::prediction_type_t);
  friend learner<T>& init_learner<>(T*, base_learner*, void (*l)(T&, base_learner&, example&),
                                    void (*pred)(T&, base_learner&, example&), size_t, prediction_type::prediction_type_t);
};

template<class T>
learner<T>& init_learner(T* dat, void (*learn)(T&, base_learner&, example&),
                         size_t params_per_weight, prediction_type::prediction_type_t pred_type)
{ // the constructor for all learning algorithms.
  learner<T>& ret = calloc_or_throw<learner<T> >();
  ret.weights = 1;
  ret.increment = params_per_weight;
  ret.end_pass_fd.func = noop;
  ret.end_examples_fd.func = noop;
  ret.init_fd.func = noop;
  ret.save_load_fd.save_load_f = noop_sl;
  ret.finisher_fd.data = dat;
  ret.finisher_fd.func = noop;

  ret.learn_fd.data = dat;
  ret.learn_fd.learn_f = (tlearn)learn;
  ret.learn_fd.update_f = (tlearn)learn;
  ret.learn_fd.predict_f = (tlearn)learn;
  ret.learn_fd.multipredict_f = nullptr;
  ret.sensitivity_fd.sensitivity_f = (tsensitivity)noop_sensitivity;
  ret.finish_example_fd.data = dat;
  ret.finish_example_fd.finish_example_f = return_simple_example;
  ret.pred_type = pred_type;

  return ret;
}

template<class T>
learner<T>& init_learner(T* dat, base_learner* base,
                         void(*learn)(T&, base_learner&, example&),
                         void(*predict)(T&, base_learner&, example&), size_t ws)
{ return init_learner<T>(dat, base, learn, predict, ws, base->pred_type);
}

template<class T>
learner<T>& init_learner(T* dat, base_learner* base,
                         void (*learn)(T&, base_learner&, example&),
                         void (*predict)(T&, base_learner&, example&), size_t ws,
                         prediction_type::prediction_type_t pred_type)
{ //the reduction constructor, with separate learn and predict functions
  learner<T>& ret = calloc_or_throw<learner<T> >();
  ret = *(learner<T>*)base;

  ret.learn_fd.data = dat;
  ret.learn_fd.learn_f = (tlearn)learn;
  ret.learn_fd.update_f = (tlearn)learn;
  ret.learn_fd.predict_f = (tlearn)predict;
  ret.learn_fd.multipredict_f = nullptr;
  ret.learn_fd.base = base;

  ret.finisher_fd.data = dat;
  ret.finisher_fd.base = base;
  ret.finisher_fd.func = noop;
  ret.pred_type = pred_type;

  ret.weights = ws;
  ret.increment = base->increment * ret.weights;
  return ret;
}

template<class T> learner<T>&
init_multiclass_learner(T* dat, base_learner* base,
                        void (*learn)(T&, base_learner&, example&),
                        void (*predict)(T&, base_learner&, example&), parser* p, size_t ws,
                        prediction_type::prediction_type_t pred_type = prediction_type::multiclass)
{ learner<T>& l = init_learner(dat,base,learn,predict,ws,pred_type);
  l.set_finish_example(MULTICLASS::finish_example<T>);
  p->lp = MULTICLASS::mc_label;
  return l;
}

template<class T> base_learner* make_base(learner<T>& base) { return (base_learner*)&base; }
}
