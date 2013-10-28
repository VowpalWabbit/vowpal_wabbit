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

namespace LEARNER
{
  void generic_driver(vw* all, void* data);
  void generic_learner(void* data, example*);
  void generic_end_pass(void* data);
  void generic_end_examples(void* data);
  void generic_finish(void* data);
  void generic_sl(void*, io_buf&, bool, bool);
  const sl_t generic_save_load = {NULL, generic_sl};
}

struct learner {
private:
  void* data;
  void (*driver)(vw* all, void* data);
  void (*learn_f)(void* data, example*);
  void (*finisher)(void* data);
  void (*end_pass)(void* data);
  void (*end_examples)(void* data);

public:
  sl_t sl;

  inline void learn(example* ec) { learn_f(data,ec); }
  inline void finish() { finisher(data); }
  inline void drive(vw* all) { driver(all, data); }
  inline void save_load(io_buf& io, bool read, bool text) { sl.save_loader(sl.sldata, io, read, text); }

  learner() {
    data = NULL;
    driver = LEARNER::generic_driver;
    learn_f = LEARNER::generic_learner;
    end_pass = LEARNER::generic_end_pass;
    end_examples = LEARNER::generic_end_examples;
    finisher = LEARNER::generic_finish;
    sl = LEARNER::generic_save_load;
  }

  learner(void *dat, void (*l)(void* data, example*))
  {
    learner();
    data = dat;
    learn_f = l;
  }

  learner(void *dat, void (*d)(vw* all, void* data), void (*l)(void* data, example*),   void (*f)(void* data), sl_t s)
  {
    learner();
    data = dat;
    driver = d;
    learn_f = l;
    finisher = f;
    sl = s;
  }
};

#endif
