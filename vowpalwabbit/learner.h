/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#ifndef LEARNER_H
#define LEARNER_H
// This is the interface for a learning algorithm
struct vw;

struct learner {
  void* data;
  
  void (*driver)(vw* all, void* data);
  void (*learn)(vw* all, void* data, example*);
  void (*finish)(vw* all, void* data);
  void (*save_load)(vw* all, void* data, io_buf&, bool read, bool text);
};
#endif
