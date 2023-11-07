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
#    define VW_FALLTHROUGH VW_ATTR(fallthrough);
#  else
#    define VW_STD17_CONSTEXPR
#    define VW_ATTR(name)
#    if defined(__GNUC__) && __GNUC__ >= 7
#      define VW_FALLTHROUGH [[gnu::fallthrough]];
#    else
#      define VW_FALLTHROUGH
#    endif
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
#  define VW_WARNING_DISABLE(warningName) DO_PRAGMA(clang diagnostic ignored warningName)
#else
#  define VW_WARNING_STATE_PUSH
#  define VW_WARNING_STATE_POP
#  define VW_WARNING_DISABLE(warningName)
#endif

// Add new ignored warnings here:
#if defined(_MSC_VER)
#  define VW_WARNING_DISABLE_DEPRECATED_USAGE VW_WARNING_DISABLE(4996)
#  define VW_WARNING_DISABLE_BADLY_FORMED_XML VW_WARNING_DISABLE(4635)
#  define VW_WARNING_DISABLE_COND_CONST_EXPR VW_WARNING_DISABLE(4127)
#  define VW_WARNING_DISABLE_CLASS_MEMACCESS
#  define VW_WARNING_DISABLE_CAST_FUNC_TYPE
#  define VW_WARNING_DISABLE_UNUSED_PARAM
#  define VW_WARNING_DISABLE_UNUSED_INTERNAL_DECLARATION
#  define VW_WARNING_DISABLE_UNUSED_FUNCTION
#elif defined(__GNUC__) || defined(__clang__)
#  define VW_WARNING_DISABLE_DEPRECATED_USAGE VW_WARNING_DISABLE("-Wdeprecated-declarations")
#  define VW_WARNING_DISABLE_BADLY_FORMED_XML
#  define VW_WARNING_DISABLE_COND_CONST_EXPR
#  define VW_WARNING_DISABLE_UNUSED_PARAM VW_WARNING_DISABLE("-Wunused-parameter")
#  define VW_WARNING_DISABLE_UNUSED_FUNCTION VW_WARNING_DISABLE("-Wunused-function")

// Clang only warnings
#  if defined(__clang__)
#    define VW_WARNING_DISABLE_UNUSED_INTERNAL_DECLARATION VW_WARNING_DISABLE("-Wunneeded-internal-declaration")
#  else
#    define VW_WARNING_DISABLE_UNUSED_INTERNAL_DECLARATION
#  endif

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
#  define VW_WARNING_DISABLE_BADLY_FORMED_XML
#  define VW_WARNING_DISABLE_CPP_17_LANG_EXT
#endif

#if defined(_MSC_VER)
#  define FORCE_INLINE __forceinline
#elif defined(__GNUC__)
#  define FORCE_INLINE __attribute__((__always_inline__))
#elif defined(__CLANG__)
#  if __has_attribute(__always_inline__)
#    define FORCE_INLINE __attribute__((__always_inline__))
#  else
#    define FORCE_INLINE
#  endif
#else
#  define FORCE_INLINE
#endif

#ifdef VW_USE_ASAN
#  if defined(_MSC_VER)
#    define NO_SANITIZE_ADDRESS __declspec(no_sanitize_address)
#  elif defined(__GNUC__) || defined(__CLANG__)
#    define NO_SANITIZE_ADDRESS __attribute__((no_sanitize("address")))
#  else
#    define NO_SANITIZE_ADDRESS
#  endif
#else
#  define NO_SANITIZE_ADDRESS
#endif

#ifdef VW_USE_UBSAN
#  if defined(__GNUC__) || defined(__CLANG__)
#    define NO_SANITIZE_UNDEFINED __attribute__((no_sanitize("undefined")))
#  else
#    define NO_SANITIZE_UNDEFINED
#  endif
#else
#  define NO_SANITIZE_UNDEFINED
#endif

#define _UNUSED(x) ((void)(x))  // NOLINT
