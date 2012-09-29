/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
// This is a function which does nothing with examples.  Used when VW is used as a compressor.

#include "example.h"
#include "parser.h"
#include "gd.h"

void drive_noop(void* in)
{
  vw* all = (vw*)in;
  example* ec = NULL;
  
  while ( !parser_done(all->p)){
    ec = get_example(all->p);
    if (ec != NULL)
      finish_example(*all, ec);
  }
}

void learn_noop(void*in, example*ec) {}

