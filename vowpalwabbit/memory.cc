#include <stdlib.h>
#include <iostream>

void* calloc_or_die(size_t nmemb, size_t size)
{
  if (nmemb == 0 || size == 0)
    return NULL;
  
  void* data = calloc(nmemb, size);
  if (data == NULL) {
    std::cerr << "internal error: memory allocation failed; dying!" << std::endl;
    throw std::exception();
  }
  return data;
}

void free_it(void*ptr)
{
  if (ptr != NULL)
    free(ptr);
}

