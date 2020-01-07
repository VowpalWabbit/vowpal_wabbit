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
// NOTE: c++ 17 detection defined above is not working properly for msvc.
// Need to rollback the following change.
// #define VW_STD17_CONSTEXPR constexpr
#define VW_STD17_CONSTEXPR
#else
#define ATTR(name)
#define VW_STD17_CONSTEXPR
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
