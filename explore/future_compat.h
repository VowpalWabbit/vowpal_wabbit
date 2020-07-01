#pragma once

#if __cplusplus >= 201103L || defined(_MSC_VER) && (_MSC_VER >= 1900)

#if __cplusplus >= 201402L || defined(_MSC_VER) && (_MSC_VER >= 1910) && (_MSVC_LANG >= 201402L)
#define HAS_STD14
#endif

#if __cplusplus >= 201703L  || defined(_MSC_VER) && (_MSC_VER >= 1914) && (_MSVC_LANG >= 201703L)
#define HAS_STD17
#endif

#ifdef HAS_STD17
#define VW_ATTR(name) [[ name ]]
#else
#define VW_ATTR(name)
#endif

#ifdef HAS_STD14
#define VW_STD14_CONSTEXPR constexpr
#define VW_DEPRECATED(message) [[deprecated(message)]]
#else
#define VW_STD14_CONSTEXPR
#define VW_DEPRECATED(message)
#endif

#else
#error "At least C++11 is required."
#endif

// The following section handles silencing specific warnings in the codebase. This section doesn't need to be modified, skip to the next block to add warnings.
#if defined(_MSC_VER)
    #define VW_DISABLE_WARNING_PUSH           __pragma(warning( push ))
    #define VW_DISABLE_WARNING_POP            __pragma(warning( pop ))
    #define VW_DISABLE_WARNING(warningNumber) __pragma(warning( disable : warningNumber ))
#elif defined(__GNUC__)
    #define DO_PRAGMA(X) _Pragma(#X)
    #define VW_DISABLE_WARNING_PUSH           DO_PRAGMA(GCC diagnostic push)
    #define VW_DISABLE_WARNING_POP            DO_PRAGMA(GCC diagnostic pop)
    #define VW_DISABLE_WARNING(warningName)   DO_PRAGMA(GCC diagnostic ignored #warningName)
#elif defined(__clang__)
    #define DO_PRAGMA(X) _Pragma(#X)
    #define VW_DISABLE_WARNING_PUSH           DO_PRAGMA(clang diagnostic push)
    #define VW_DISABLE_WARNING_POP            DO_PRAGMA(clang diagnostic pop)
    #define VW_DISABLE_WARNING(warningName)   DO_PRAGMA(clang diagnostic ignored #warningName)
#else
    #define VW_DISABLE_WARNING_PUSH
    #define VW_DISABLE_WARNING_POP
    #define VW_DISABLE_WARNING(warningName)
#endif

// Add new ignored warnings here:
#if defined(_MSC_VER)
    #define VW_DISABLE_WARNING_DEPRECATED_USAGE    VW_DISABLE_WARNING(4996)
    #define VW_DISABLE_WARNING_CLASS_MEMACCESS
#elif defined(__GNUC__) || defined(__clang__)
    #define VW_DISABLE_WARNING_DEPRECATED_USAGE   VW_DISABLE_WARNING(-Wdeprecated-declarations)

    // This warning was added in GCC 8
    #if __GNUC__ >= 8
        #define VW_DISABLE_WARNING_CLASS_MEMACCESS    VW_DISABLE_WARNING(-Wclass-memaccess)
    #else
        #define VW_DISABLE_WARNING_CLASS_MEMACCESS
    #endif
#else
    #define VW_DISABLE_WARNING_DEPRECATED_USAGE
    #define VW_DISABLE_WARNING_CLASS_MEMACCESS
#endif
