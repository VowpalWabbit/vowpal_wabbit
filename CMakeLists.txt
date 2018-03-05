# CMake build system for Vowpal Wabbit:
CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

# Project invocation:
project(vowpalwabbit C CXX)

# Find required packages
find_package(ZLIB REQUIRED)
find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS program_options)
find_package(Perl 5.14)
find_package(Doxygen)

include(CheckTypeSize)
include(CheckFunctionExists)
include(CheckIncludeFile)
include(CheckCSourceCompiles)

# ==================
# Hunt for features:

check_include_file(sys/types.h HAVE_SYS_TYPES_H)
check_include_file(stdint.h HAVE_STDINT_H)
check_include_file(stddef.h HAVE_STDDEF_H)
check_include_file(unistd.h HAVE_UNISTD_H)
check_include_file(xmmintrin.h HAVE_XMMINTRIN_H)

#
# Check to see if we have large file support
#
set(CMAKE_REQUIRED_DEFINITIONS -D_LARGEFILE64_SOURCE=1)

# We add these other definitions here because CheckTypeSize.cmake
# in CMake 2.4.x does not automatically do so and we want
# compatibility with CMake 2.4.x.
if(HAVE_SYS_TYPES_H)
    list(APPEND CMAKE_REQUIRED_DEFINITIONS -DHAVE_SYS_TYPES_H)
endif()
if(HAVE_STDINT_H)
    list(APPEND CMAKE_REQUIRED_DEFINITIONS -DHAVE_STDINT_H)
endif()
if(HAVE_STDDEF_H)
    list(APPEND CMAKE_REQUIRED_DEFINITIONS -DHAVE_STDDEF_H)
endif()

# Note: Also sets "SIZE_T" to the size of the data type
check_type_size(size_t SIZE_T)
if (HAVE_SIZE_T)
    set(SIZEOF_SIZE_T ${SIZE_T})
endif (HAVE_SIZE_T)

# Look for signed version of size_t. MSVC is notorious for not providing this typedef.
check_type_size(ssize_t SSIZE_T)

# Check the size of unsigned int, unsigned long, unsigned long long
check_type_size("unsigned int" UINT_T)
if (HAVE_UINT_T)
    set(SIZEOF_UINT_T ${UINT_T})
else (HAVE_UINT_T)
    set(SIZEOF_UINT_T 0)
endif (HAVE_UINT_T)

check_type_size("unsigned long" ULONG_T)
if (HAVE_ULONG_T)
    set(SIZEOF_ULONG_T ${ULONG_T})
else (HAVE_ULONG_T)
    set(SIZEOF_ULONG_T 0)
endif (HAVE_ULONG_T)

check_type_size("unsigned long long" ULONGLONG_T)
if (HAVE_ULONGLONG_T)
    set(SIZEOF_ULONGLONG_T ${ULONGLONG_T})
else(HAVE_ULONGLONG_T)
    set(SIZEOF_ULONGLONG_T 0)
endif(HAVE_ULONGLONG_T)

check_type_size("uint64_t" UINT64_T)
if (HAVE_UINT64_T)
    set(SIZEOF_UINT64_T ${UINT64_T})
else (HAVE_UINT64_T)
    set(SIZEOF_UINT64_T 0)
endif (HAVE_UINT64_T)

check_type_size("int64_t" INT64_T)
if (HAVE_INT64_T)
    set(SIZEOF_INT64_T ${INT64_T})
else (HAVE_INT64_T)
    set(SIZEOF_INT64_T 0)
endif (HAVE_INT64_T)

# 64-bit file offset:
check_type_size(off64_t OFF64_T)
if(HAVE_OFF64_T)
    add_definitions(-D_LARGEFILE64_SOURCE=1)
endif()


# TODO: Can we remove these?
set(CMAKE_REQUIRED_DEFINITIONS )
set(CMAKE_THREAD_PREFER_PTHREAD ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_LIBS OFF)
set(HAVE_BOOST_PROGRAM_OPTIONS ON)
set(HAVE_BOOST ON)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

# Add the config interface target
add_library(config INTERFACE)
target_include_directories(config INTERFACE ${CMAKE_BINARY_DIR})

if(MSVC)
    target_compile_definitions(config
        INTERFACE WIN32 _WINDOWS _USRDLL VWDLL_EXPORTS
    )
endif()

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Add subdirectories:
add_subdirectory(rapidjson)
add_subdirectory(vowpalwabbit)
add_subdirectory(library)
# add_subdirectory(test)
# add_subdirectory(c_test)
