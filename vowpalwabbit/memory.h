// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
#include <cstdio>
#include <cstdlib>
#include <cstring>  // memset
#include <memory>

#include "vw_exception.h"

// unistd.h is needed for ::sysconf on linux toolchains
#if defined(__linux__)
#  include <unistd.h>
#endif

template <class T>
T* calloc_or_throw(size_t nmemb)
{
  if (nmemb == 0) return nullptr;

  void* data = calloc(nmemb, sizeof(T));
  if (data == nullptr)
  {
    const char* msg = "internal error: memory allocation failed!\n";
    // use low-level function since we're already out of memory.
    fputs(msg, stderr);
    THROW_OR_RETURN(msg, nullptr);
  }
  return (T*)data;
}

template <class T>
T& calloc_or_throw()
{
  return *calloc_or_throw<T>(1);
}

using free_fn = void (*)(void*);

template <class T>
using free_ptr = std::unique_ptr<T, free_fn>;

template <class T>
void destroy_free(void* temp)
{
  ((T*)temp)->~T();
  free(temp);
}

template <class T, typename... Args>
free_ptr<T> scoped_calloc_or_throw(Args&&... args)
{
  T* temp = calloc_or_throw<T>(1);
  try
  {
    new (temp) T(std::forward<Args>(args)...);
  }
  catch (...)
  {
    free(temp);
    throw;
  }
  return std::unique_ptr<T, free_fn>(temp, destroy_free<T>);
}

namespace VW
{
template <typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... params)
{
  return std::unique_ptr<T>(new T(std::forward<Args>(params)...));
}
}  // namespace VW

#ifdef MADV_MERGEABLE
template <class T>
T* calloc_mergable_or_throw(size_t nmemb)
{
  if (nmemb == 0) return nullptr;
  size_t length = nmemb * sizeof(T);
#  if defined(ANDROID)
  // posix_memalign is not available on Android
  void* data = memalign(sysconf(_SC_PAGE_SIZE), length);
  if (!data)
#  else
  void* data;
  if (0 != posix_memalign(&data, sysconf(_SC_PAGE_SIZE), length))
#  endif
  {
    const char* msg = "internal error: memory allocation failed!\n";
    fputs(msg, stderr);
    THROW_OR_RETURN(msg, nullptr);
  }
  if (data == nullptr)
  {
    const char* msg = "internal error: memory allocation failed!\n";
    fputs(msg, stderr);
    THROW_OR_RETURN(msg, nullptr);
  }
  memset(data, 0, length);
  // mark weight vector as KSM sharable
  // it allows to save memory if you run multiple instances of the same model
  // see more https://www.kernel.org/doc/Documentation/vm/ksm.txt
  // you need to have Linux kernel >= 2.6.32 and KSM enabled
  // to check is KSM enabled run the command
  // $ grep KSM /boot/config-`uname -r`
  // if KSM is enabled you should see:
  // >> CONFIG_KSM=y
  // you can enable ksmd with sudo "echo 1 > /sys/kernel/mm/ksm/run"
  // mark address space as a candidate for merging

  if (0 != madvise(data, length, MADV_MERGEABLE))
  {
    const char* msg = "internal warning: marking memory as ksm mergeable failed!\n";
    fputs(msg, stderr);
  }
  return (T*)data;
}
#else
#  define calloc_mergable_or_throw calloc_or_throw
#endif

inline void free_it(void* ptr)
{
  if (ptr != nullptr) free(ptr);
}
