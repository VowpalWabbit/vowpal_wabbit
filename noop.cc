// This is a function which does nothing with examples.  Used when VW is used as a compressor.

#include "example.h"
#include "parser.h"

void start_noop()
{
  example* ec = NULL;
  
  while ( (ec = get_example(ec,0)) )
    {
      ec->threads_to_finish = 1;
      ec->done = true;
    }
}

void end_noop()
{
}
