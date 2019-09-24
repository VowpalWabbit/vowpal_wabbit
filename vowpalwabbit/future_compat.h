#pragma once

#if __cplusplus < 201103L
#pragma error "At least C++11 is required."
#endif

#if __cplusplus >= 201402L
#define HAS_STD14
#endif

#if __cplusplus >= 201703L
#define HAS_STD17
#endif

#ifdef HAS_STD17
#define ATTR(name) [[ name ]]
#else
#define ATTR(name)
#endif

#ifdef HAS_STD14
#define STD14_CONSTEXPR constexpr
#else
#define STD14_CONSTEXPR
#endif
