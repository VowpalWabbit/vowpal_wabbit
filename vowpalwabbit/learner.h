/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#ifndef LEARNER_H
#define LEARNER_H
// This is the interface for a learning algorithm
struct vw;

struct learner;

struct func_data {
  void* data;
  learner* base;
  void (*func)(void* data);
};

inline func_data tuple_dbf(void* data, learner* base, void (*func)(void* data))
{
  func_data foo;
  foo.data = data;
  foo.base = base;
  foo.func = func;
  return foo;
}

struct learn_data {
  void* data;
  learner* base;
  void (*learn_f)(void* data, learner& base, example*);
  void (*predict_f)(void* data, learner& base, example*);
};

struct save_load_data{
  void* data;
  learner* base;
  void (*save_load_f)(void*, io_buf&, bool read, bool text);
};

struct finish_example_data{
  void* data;
  learner* base;
  void (*finish_example_f)(vw&, void* data, example*);
};

void return_simple_example(vw& all, void*, example* ec);

#include<iostream>
using namespace std;

namespace LEARNER
{
  void generic_driver(vw* all);
  
  inline void generic_sl(void*, io_buf&, bool, bool) {}
  inline void generic_learner(void* data, learner& base, example*)
  { cout << "calling generic learner\n";}
  inline void generic_func(void* data) {}

  const save_load_data generic_save_load_fd = {NULL, NULL, generic_sl};
  const learn_data generic_learn_fd = {NULL, NULL, generic_learner, NULL};
  const func_data generic_func_fd = {NULL, NULL, generic_func};
}

struct learner {
private:
  func_data init_fd;
  learn_data learn_fd;
  finish_example_data finish_example_fd;
  save_load_data save_load_fd;
  func_data end_pass_fd;
  func_data end_examples_fd;
  func_data finisher_fd;
  
public:
  size_t weights; //this stores the number of "weight vectors" required by the learner.
  size_t increment;

  //called once for each example.  Must work under reduction.
  inline void learn(example* ec, size_t i=0) 
  { 
    ec->ft_offset += (uint32_t)(increment*i);
    learn_fd.learn_f(learn_fd.data, *learn_fd.base, ec);
    ec->ft_offset -= (uint32_t)(increment*i);
  }

  inline void predict(example* ec, size_t i=0) 
  { 
    ec->ft_offset += (uint32_t)(increment*i);
    learn_fd.predict_f(learn_fd.data, *learn_fd.base, ec);
    ec->ft_offset -= (uint32_t)(increment*i);
  }

  //called anytime saving or loading needs to happen. Autorecursive.
  inline void save_load(io_buf& io, bool read, bool text) { save_load_fd.save_load_f(save_load_fd.data, io, read, text); if (save_load_fd.base) save_load_fd.base->save_load(io, read, text); }
  inline void set_save_load(void (*sl)(void*, io_buf& io, bool read, bool text))
  { save_load_fd.save_load_f = sl; 
    save_load_fd.data = learn_fd.data; 
    save_load_fd.base = learn_fd.base;}

  //called to clean up state.  Autorecursive.
  void set_finish(void (*f)(void*)) { finisher_fd = tuple_dbf(learn_fd.data,learn_fd.base,f); }
  inline void finish() 
  { 
    if (finisher_fd.data) 
      {finisher_fd.func(finisher_fd.data); free(finisher_fd.data); } 
    if (finisher_fd.base) { 
      finisher_fd.base->finish();
      delete finisher_fd.base;
    }
  }

  void end_pass(){ 
    end_pass_fd.func(end_pass_fd.data);
    if (end_pass_fd.base) end_pass_fd.base->end_pass(); }//autorecursive
  void set_end_pass(void (*ep)(void*)) {end_pass_fd = tuple_dbf(learn_fd.data, learn_fd.base, ep);}

  //called after parsing of examples is complete.  Autorecursive.
  void end_examples() 
  { end_examples_fd.func(end_examples_fd.data); 
    if (end_examples_fd.base) end_examples_fd.base->end_examples(); } 
  void set_end_examples(void (*ee)(void*)) {end_examples_fd = tuple_dbf(learn_fd.data,learn_fd.base,ee);}

  //Called at the beginning by the driver.  Explicitly not recursive.
  void init_driver() { init_fd.func(init_fd.data);}
  void set_init_driver(void (*id)(void*)) { init_fd = tuple_dbf(learn_fd.data,learn_fd.base, id); }

  //called after learn example for each example.  Explicitly not recursive.
  inline void finish_example(vw& all, example* ec) { finish_example_fd.finish_example_f(all, finish_example_fd.data, ec);}
  void set_finish_example(void (*ef)(vw& all, void*, example*))
  {finish_example_fd.data = learn_fd.data;
    finish_example_fd.finish_example_f = ef;}

  void driver(vw* all) {LEARNER::generic_driver(all);}

  inline learner()
  {
    weights = 1;
    increment = 1;

    learn_fd = LEARNER::generic_learn_fd;
    finish_example_fd.data = NULL;
    finish_example_fd.finish_example_f = return_simple_example;
    end_pass_fd = LEARNER::generic_func_fd;
    end_examples_fd = LEARNER::generic_func_fd;
    init_fd = LEARNER::generic_func_fd;
    finisher_fd = LEARNER::generic_func_fd;
    save_load_fd = LEARNER::generic_save_load_fd;
  }

  inline learner(void *dat, void (*l)(void*, learner&, example*), void (*sl)(void*, io_buf& io, bool read, bool text), size_t params_per_weight)
  { // the constructor for all learning algorithms.
    *this = learner();

    learn_fd.data = dat;
    learn_fd.learn_f = l;
    learn_fd.predict_f = l;

    finisher_fd.data = dat;
    finisher_fd.base = NULL;
    finisher_fd.func = LEARNER::generic_func;

    set_save_load(sl);
    increment = params_per_weight;
  }

  inline learner(void *dat, void (*l)(void*, learner&, example*), learner* base, size_t ws = 1) 
  { //the reduction constructor.
    *this = *base;
    
    learn_fd.learn_f = l;
    learn_fd.predict_f = l;
    learn_fd.data = dat;
    learn_fd.base = base;

    finisher_fd.data = dat;
    finisher_fd.base = base;
    finisher_fd.func = LEARNER::generic_func;

    increment = base->increment * base->weights;
    weights = ws;
  }
};

#endif
