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

struct learner {
  void* data;
  void (*driver)(vw* all, void* data);
  void (*learner)(void* data, example*);
  void (*finisher)(void* data);

  sl_t sl;

  inline void learn(example* ec) { learner(data,ec); }
  inline void finish() { finisher(data); }
  inline void drive(vw* all) { driver(all, data); }
  inline void save_load(io_buf& io, bool read, bool text) { sl.save_loader(sl.sldata, io, read, text); }
};
#endif
