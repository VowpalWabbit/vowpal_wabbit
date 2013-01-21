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

namespace NOOP {
  void learn(void*in, void* d, example*ec) {}
  void finish(void*in, void* d) {}

  void save_load(void* in, void* d, io_buf& model_file, bool read, bool text) {}
  
  void drive(void* in, void* d)
  {
    vw* all = (vw*)in;
    example* ec = NULL;
    
    while ( !parser_done(all->p)){
      ec = get_example(all->p);
      if (ec != NULL)
	return_simple_example(*all, ec);
    }
  }
  
  void parse_flags(vw& all)
  {
    learner t = {NULL,drive,learn,finish,save_load};
    all.l = t;
    all.is_noop = true;
  }

}
