/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
// This is a function which does nothing with examples.  Used when VW is used as a compressor.

#include "example.h"
#include "parser.h"
#include "gd.h"
#include "simple_label.h"
#include "vw.h"

namespace NOOP {
  void learn(void* d, example*ec) {}
  void finish(void* d) {}

  void save_load(void* d, io_buf& model_file, bool read, bool text) {}
  
  learner setup(vw& all)
  {
    sl_t sl = {NULL,save_load};
    all.is_noop = true;
    learner l(NULL,LEARNER::generic_driver,learn,finish,sl);
    return l;
  }
}
