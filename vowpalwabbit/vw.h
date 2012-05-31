#ifndef VW_H
#define VW_H

#include "global_data.h"
#include "example.h"
vw vw_initialize(char* c);
example* vw_read_example(vw& all, char* example_line);
void vw_finish_example(vw& all, example* ec);

#endif
