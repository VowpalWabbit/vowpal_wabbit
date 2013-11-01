/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#ifndef LEARNER_H
#define LEARNER_H
// This is the interface for a learning algorithm
struct vw;

struct func_data {
  void* data;
  void (*func)(void* data);
};

inline func_data tuple(void* data, void (*func)(void* data))
{
  func_data foo;
  foo.data = data;
  foo.func = func;
  return foo;
}

struct learn_data {
  void* data;
  void (*learn_f)(void* data, example*);
};

struct save_load_data{
  void* data;
  void (*save_load_f)(void*, io_buf&, bool read, bool text);
};

struct finish_example_data{
  void* data;
  void (*finish_example_f)(vw&, void* data, example*);
};

void return_simple_example(vw& all, void*, example* ec);

#include<iostream>
using namespace std;

namespace LEARNER
{
  void generic_driver(vw* all);

  inline void generic_sl(void*, io_buf&, bool, bool) {}
  inline void generic_learner(void* data, example*)
  { cout << "calling generic learner\n";}
  inline void generic_func(void* data) {}

  const save_load_data generic_save_load_fd = {NULL, generic_sl};
  const learn_data generic_learn_fd = {NULL, generic_learner};
  const func_data generic_func_fd = {NULL, generic_func};
}

struct learner {
private:
  void* default_data;
  learner* base;

  func_data init_fd;
  learn_data learn_fd;
  finish_example_data finish_example_fd;
  save_load_data save_load_fd;
  func_data end_pass_fd;
  func_data end_examples_fd;
  func_data finisher_fd;
  
public:
  //called once for each example.  Must work under reduction.
  inline void learn(example* ec) { learn_fd.learn_f(learn_fd.data, ec); }

  //called anytime saving or loading needs to happen. Autorecursive.
  inline void save_load(io_buf& io, bool read, bool text) { save_load_fd.save_load_f(save_load_fd.data, io, read, text); if (base) base->save_load(io, read, text); }

  //called to clean up state.  Autorecursive.
  void set_finish(void (*f)(void*)) { finisher_fd = tuple(default_data,f); }
  inline void finish() { if (finisher_fd.data) {finisher_fd.func(finisher_fd.data); free(finisher_fd.data); } if (base) base->finish(); }

  //called after learn example for each example.  Not called under reduction.
  inline void finish_example(vw& all, example* ec) { finish_example_fd.finish_example_f(all, finish_example_fd.data, ec);}
  void set_finish_example(void (*ef)(vw& all, void*, example*))
  {finish_example_fd.data = default_data;
    finish_example_fd.finish_example_f = ef;}

  void end_pass(){ end_pass_fd.func(end_pass_fd.data); if (base) base->end_pass(); }//autorecursive
  void set_end_pass(void (*ep)(void*)) {end_pass_fd = tuple(default_data, ep);}

  //called after parsing of examples is complete.  Autorecursive.
  void end_examples() { end_examples_fd.func(end_examples_fd.data); if (base) base->end_examples(); } 
  void set_end_examples(void (*ee)(void*)) {end_examples_fd = tuple(default_data,ee);}

  //Called at the beginning by the driver.  Explicitly not recursive.
  void init_driver() { init_fd.func(init_fd.data);}
  void set_init_driver(void (*id)(void*)) { init_fd = tuple(default_data,id); }

  void set_base(learner* b) { base=b; }

  void driver(vw* all) {LEARNER::generic_driver(all);}

  inline learner()
  {
    default_data = NULL;
    base = NULL;
    learn_fd = LEARNER::generic_learn_fd;
    finish_example_fd.data = NULL;
    finish_example_fd.finish_example_f = return_simple_example;
    end_pass_fd = LEARNER::generic_func_fd;
    end_examples_fd = LEARNER::generic_func_fd;
    init_fd = LEARNER::generic_func_fd;
    finisher_fd = LEARNER::generic_func_fd;
    save_load_fd = LEARNER::generic_save_load_fd;
  }

  inline learner(void *dat, void (*l)(void* data, example*))
  {
    *this = learner();

    default_data = dat;
    learn_fd.data = dat;
    learn_fd.learn_f = l;
  }

  inline learner(void *dat, void (*l)(void* data, example*),   void (*save_load_f)(void*, io_buf&, bool read, bool text))
  {
    *this = learner(dat, l);

    save_load_fd.data = dat;
    save_load_fd.save_load_f = save_load_f;
  }
};

#endif
