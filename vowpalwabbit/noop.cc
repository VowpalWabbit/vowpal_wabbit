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

