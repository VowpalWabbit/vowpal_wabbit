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
    const char* msg = "internal error: memory allocation failed; dying!";
	// use low-level function since we're already out of memory.
	fputs(msg, stderr);
    throw std::runtime_error(msg);
  }
  return (T*)data;
}

template<class T> T& calloc_or_die()
{ return *calloc_or_die<T>(1); }

inline void free_it(void* ptr) { if (ptr != nullptr) free(ptr); }
