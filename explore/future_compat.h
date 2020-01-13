#pragma once

#if __cplusplus >= 201103L || defined(_MSC_VER) && (_MSC_VER >= 1900)

#if __cplusplus >= 201402L || defined(_MSC_VER) && (_MSC_VER >= 1910)
#define HAS_STD14
#endif

#if __cplusplus >= 201703L  || defined(_MSC_VER) && (_MSC_VER >= 1914)
#define HAS_STD17
#endif

#ifdef HAS_STD17
#define ATTR(name) [[ name ]]
#else
#define ATTR(name)
#endif

#ifdef HAS_STD14
#define VW_STD14_CONSTEXPR constexpr
#define VW_DEPRECATED(message) [[deprecated(message)]]
#if defined(__clang__)
    #define IGNORE_DEPRECATED_USAGE_START   \
        _Pragma("clang diagnostic push")      \
        _Pragma("clang diagnostic ignored \"-Wdeprecated\"")
    #define IGNORE_DEPRECATED_USAGE_END _Pragma("GCC diagnostic pop")
#elif defined(__GNUC__) || defined(__GNUG__)
    #define IGNORE_DEPRECATED_USAGE_START   \
    _Pragma("GCC diagnostic push")        \
    _Pragma("GCC diagnostic ignored \"-Wdeprecated\"")
    #define IGNORE_DEPRECATED_USAGE_END _Pragma("GCC diagnostic pop")
#elif defined(_MSC_VER)
    #define IGNORE_DEPRECATED_USAGE_START __pragma(warning (disable : 4996))
    #define IGNORE_DEPRECATED_USAGE_END __pragma(warning (default : 4996))
#endif
#else
#define VW_STD14_CONSTEXPR
#define VW_DEPRECATED(message)
#define IGNORE_DEPRECATED_USAGE_START
#define IGNORE_DEPRECATED_USAGE_END
#endif

#else
#error "At least C++11 is required."
#endif
