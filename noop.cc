// This is a function which does nothing with examples.  Used when VW is used as a compressor.

#include "example.h"
#include "parser.h"
#include "gd.h"

void start_noop()
{
  example* ec = NULL;
  
  while ( (ec = get_example(0)) )
    finish_example(ec);
}

void end_noop()
{
}
