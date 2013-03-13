/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#ifndef LEARNER_H
#define LEARNER_H
// This is the interface for a learning algorithm

struct learner {
  void* data;
  
  void (*driver)(void* all, void* data);
  void (*learn)(void* all, void* data, example*);
  void (*finish)(void* all, void* data);
  void (*save_load)(void* all, void* data, io_buf&, bool read, bool text);
};
#endif
