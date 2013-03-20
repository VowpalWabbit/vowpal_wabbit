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
  void (*save_load)(void* sldata, io_buf&, bool read, bool text);
};

struct learner {
  void* data;
  void (*driver)(vw* all, void* data);
  void (*learn)(void* data, example*);
  void (*finish)(void* data);

  sl_t sl;
};
#endif
