#pragma once

#include <stdlib.h>
#include <iostream>

template<class T>
T* calloc_or_die(size_t nmemb = 1)
{
  if (nmemb == 0)
    return NULL;
  
  void* data = calloc(nmemb, sizeof(T));
  if (data == NULL) {
    std::cerr << "internal error: memory allocation failed; dying!" << std::endl;
    throw std::exception();
  }
  return (T*)data;
}

void free_it(void* ptr);
