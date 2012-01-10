// This is a function which does nothing with examples.  Used when VW is used as a compressor.

#include "example.h"
#include "parser.h"
#include "gd.h"

void start_noop()
{
  example* ec = NULL;
  
  while ( !parser_done()){
    ec = global.get_example();
    if (ec != NULL)
      finish_example(ec);
  }
}

void end_noop()
{
}
