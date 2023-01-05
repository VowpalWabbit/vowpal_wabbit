// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
#include "vw/common/vw_exception.h"
#include "vw/common/vw_throw.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>  // memset
#include <memory>

// unistd.h is needed for ::sysconf on linux toolchains
#if defined(__linux__)
#  include <unistd.h>
#endif

namespace VW
{
#if __cplusplus >= 201402L  // C++14 and beyond
using std::make_unique;
#else
template <typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... params)
{
  static_assert(!std::is_array<T>::value, "arrays not supported");
  return std::unique_ptr<T>(new T(std::forward<Args>(params)...));
}
#endif

namespace details
{
template <class T>
T* calloc_or_throw(size_t nmemb)
{
  if (nmemb == 0) { return nullptr; }

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

template <class T>
T* calloc_mergable_or_throw(size_t nmemb)
{
#ifdef MADV_MERGEABLE
  if (nmemb == 0) { return nullptr; }
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
#else
  return calloc_or_throw<T>(nmemb);
#endif
}
}  // namespace details

}  // namespace VW
