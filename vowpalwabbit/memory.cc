#include <stdlib.h>

void free_it(void* ptr)
{
  if (ptr != NULL)
    free(ptr);
}

