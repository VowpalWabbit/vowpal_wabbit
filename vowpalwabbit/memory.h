#pragma once
#include <stdlib.h>
#include <iostream>

template<class T>
T* calloc_or_die(size_t nmemb)
{
  if (nmemb == 0)
    return nullptr;
  
  void* data = calloc(nmemb, sizeof(T));
  if (data == nullptr) {
    std::cerr << "internal error: memory allocation failed; dying!" << std::endl;
    throw std::exception();
  }
  return (T*)data;
}

template<class T> T& calloc_or_die()
{ return *calloc_or_die<T>(1); }

inline void free_it(void* ptr) { if (ptr != nullptr) free(ptr); }
