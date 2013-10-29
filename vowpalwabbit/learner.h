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
  inline void generic_end_pass(void* data)
  { cout << "calling generic end_pass\n";}
  inline void generic_end_examples(void* data) {}
  inline void generic_finish(void* data)
  { cout << "calling generic finish\n";}

  const sl_t generic_save_load = {NULL, generic_sl};
}

struct learner {
private:
  void* data;
  void (*driver)(vw* all, void* data);
  void (*learn_f)(void* data, example*);
  void (*finish_example_f)(vw&, void* data, example*);
  void (*finisher)(void* data);
  void (*end_pass)(void* data);
  void (*end_examples_f)(void* data);

public:
  sl_t sl;

  inline void learn(example* ec) { learn_f(data,ec); }
  inline void finish() { finisher(data); }
  inline void finish_example(vw& all, example* ec) { finish_example_f(all, data, ec);}
  inline void drive(vw* all) { driver(all, data); }
  inline void save_load(io_buf& io, bool read, bool text) { sl.save_loader(sl.sldata, io, read, text); }

  void set_finish_example(void (*ef)(vw& all, void*, example*))
  {finish_example_f = ef;}

  void set_end_examples(void (*ee)(void*)) 
  {end_examples_f = ee;}
  void end_examples() {end_examples_f(data);}

  void set_default()
  {
    data = NULL;
    driver = LEARNER::generic_driver;
    learn_f = LEARNER::generic_learner;
    finish_example_f = return_simple_example;
    end_pass = LEARNER::generic_end_pass;
    end_examples_f = LEARNER::generic_end_examples;
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
