/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#ifndef LEARNER_H
#define LEARNER_H
// This is the interface for a learning algorithm
struct vw;

struct sl_t {
  void* sldata;
  void (*save_loader)(void* sldata, io_buf&, bool read, bool text);
};

void return_simple_example(vw& all, void*, example* ec);

#include<iostream>
using namespace std;

namespace LEARNER
{
  void generic_driver(vw* all, void* data);

  inline void generic_sl(void*, io_buf&, bool, bool)
  { cout << "calling generic_save_load";}
  inline void generic_learner(void* data, example*)
  { cout << "calling generic learner\n";}
  inline void generic_end_pass(void* data) {}
  inline void generic_end_examples(void* data) {}
  inline void generic_init_driver(void* data) {}
  inline void generic_finish(void* data)
  { cout << "calling generic finish\n";}

  const sl_t generic_save_load = {NULL, generic_sl};
}

struct learner {
private:
  void* data;
  learner* base;
  void (*driver)(vw* all, void* data);
  void (*learn_f)(void* data, example*);
  void (*finish_example_f)(vw&, void* data, example*);
  void (*finisher)(void* data);
  void (*end_pass_f)(void* data);
  void (*end_examples_f)(void* data);
  void (*init_driver_f)(void* data);

public:
  sl_t sl;

  //called once for each example.  Must work under reduction.
  inline void learn(example* ec) { learn_f(data,ec); }

  //called anytime saving or loading needs to happen.  Must work under reduction.
  inline void save_load(io_buf& io, bool read, bool text) { sl.save_loader(sl.sldata, io, read, text); }

  //called to clean up state.  Must work under reduction.
  inline void finish() { finisher(data); }

  //called after learn example for each example.  Not called under reduction.
  inline void finish_example(vw& all, example* ec) { finish_example_f(all, data, ec);}
  void set_finish_example(void (*ef)(vw& all, void*, example*))
  {finish_example_f = ef;}

  void end_pass(){
    end_pass_f(data); 
    if (base) 
      base->end_pass(); 
  }
  void set_end_pass(void (*ep)(void*)) {end_pass_f = ep;}

  //called after parsing of examples is complete.  Not called under Reduction.
  void end_examples() {end_examples_f(data);} 
  void set_end_examples(void (*ee)(void*)) 
  {end_examples_f = ee;}

  void init_driver() { init_driver_f(data); }
  void set_init_driver(void (*id)(void*)) { init_driver_f = id; }

  void set_base(learner* b) { base=b; }

  //disappearing shortly.
  inline void drive(vw* all) { driver(all, data); }

  void set_default()
  {
    data = NULL;
    base = NULL;
    driver = LEARNER::generic_driver;
    learn_f = LEARNER::generic_learner;
    finish_example_f = return_simple_example;
    end_pass_f = LEARNER::generic_end_pass;
    end_examples_f = LEARNER::generic_end_examples;
    init_driver_f = LEARNER::generic_init_driver;
    finisher = LEARNER::generic_finish;
    sl = LEARNER::generic_save_load;
  }

  learner() {
    set_default();
  }

  learner(void *dat, void (*l)(void* data, example*))
  {
    set_default();
    data = dat;
    learn_f = l;
  }

  learner(void *dat, void (*d)(vw* all, void* data), void (*l)(void* data, example*),   void (*f)(void* data), sl_t s)
  {
    set_default();
    data = dat;
    driver = d;
    learn_f = l;
    finisher = f;
    sl = s;
  }
};

#endif
