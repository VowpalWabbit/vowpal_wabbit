#include <stdlib.h>
#include <iostream>

void free_it(void*ptr)
{
  if (ptr != NULL)
    free(ptr);
}

