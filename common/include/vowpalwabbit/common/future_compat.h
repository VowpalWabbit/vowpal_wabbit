// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#if __cplusplus >= 201103L || defined(_MSC_VER) && (_MSC_VER >= 1900)

#  if __cplusplus >= 201402L || defined(_MSC_VER) && (_MSC_VER >= 1910) && (_MSVC_LANG >= 201402L)
#    define HAS_STD14
#  endif

#  if __cplusplus >= 201703L || defined(_MSC_VER) && (_MSC_VER >= 1914) && (_MSVC_LANG >= 201703L)
#    define HAS_STD17
#  endif

#  ifdef HAS_STD17
#    define VW_STD17_CONSTEXPR constexpr
#    define VW_ATTR(name) [[name]]
#  else
#    define VW_STD17_CONSTEXPR
#    define VW_ATTR(name)
#  endif

#  ifdef HAS_STD14
#    define VW_STD14_CONSTEXPR constexpr
#    define VW_DEPRECATED(message) [[deprecated(message)]]
#  else
#    define VW_STD14_CONSTEXPR
#    define VW_DEPRECATED(message)
#  endif

#else
#  error "At least C++11 is required."
#endif

// The following section handles silencing specific warnings in the codebase. This section doesn't need to be modified,
// skip to the next block to add warnings.
#if defined(_MSC_VER)
#  define VW_WARNING_STATE_PUSH __pragma(warning(push))
#  define VW_WARNING_STATE_POP __pragma(warning(pop))
#  define VW_WARNING_DISABLE(warningNumber) __pragma(warning(disable : warningNumber))
#elif defined(__GNUC__)
#  define DO_PRAGMA(X) _Pragma(#  X)
#  define VW_WARNING_STATE_PUSH DO_PRAGMA(GCC diagnostic push)
#  define VW_WARNING_STATE_POP DO_PRAGMA(GCC diagnostic pop)
#  define VW_WARNING_DISABLE(warningName) DO_PRAGMA(GCC diagnostic ignored warningName)
#elif defined(__clang__)
#  define DO_PRAGMA(X) _Pragma(#  X)
#  define VW_WARNING_STATE_PUSH DO_PRAGMA(clang diagnostic push)
#  define VW_WARNING_STATE_POP DO_PRAGMA(clang diagnostic pop)
#  define VW_WARNING_DISABLE(warningName) DO_PRAGMA(clang diagnostic ignored = warningName)
#else
#  define VW_WARNING_STATE_PUSH
#  define VW_WARNING_STATE_POP
#  define VW_WARNING_DISABLE(warningName)
#endif

// Add new ignored warnings here:
#if defined(_MSC_VER)
#  define VW_WARNING_DISABLE_DEPRECATED_USAGE VW_WARNING_DISABLE(4996)
#  define VW_WARNING_DISABLE_CLASS_MEMACCESS
#  define VW_WARNING_DISABLE_CAST_FUNC_TYPE
#  define VW_WARNING_DISABLE_CPP_17_LANG_EXT VW_WARNING_DISABLE(4984)
#elif defined(__GNUC__) || defined(__clang__)
#  define VW_WARNING_DISABLE_DEPRECATED_USAGE VW_WARNING_DISABLE("-Wdeprecated-declarations")
#  define VW_WARNING_DISABLE_CPP_17_LANG_EXT

// This warning was added in GCC 8
#  if __GNUC__ >= 8
#    define VW_WARNING_DISABLE_CLASS_MEMACCESS VW_WARNING_DISABLE("-Wclass-memaccess")
#    define VW_WARNING_DISABLE_CAST_FUNC_TYPE VW_WARNING_DISABLE("-Wcast-function-type")
#  else
#    define VW_WARNING_DISABLE_CLASS_MEMACCESS
#    define VW_WARNING_DISABLE_CAST_FUNC_TYPE
#  endif
#else
#  define VW_WARNING_DISABLE_DEPRECATED_USAGE
#  define VW_WARNING_DISABLE_CLASS_MEMACCESS
#  define VW_WARNING_DISABLE_CPP_17_LANG_EXT
#endif
