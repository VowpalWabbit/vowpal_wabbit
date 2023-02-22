set(VW_LINUX_FLAGS "")
if(LTO)
  if(NOT "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    message(FATAL_ERROR "LTO requires Clang")
  endif()
  if("${CMAKE_CXX_COMPILER_VERSION}" VERSION_LESS "8.0.0")
    message(FATAL_ERROR "LTO requires Clang 8.0 (llvm 3.9) or later")
  endif()
  If("${CONFIG}" STREQUAL "DEBUG")
    message(FATAL_ERROR "LTO only works with Release builds")
  endif()
  set(VW_LINUX_FLAGS ${VW_LINUX_FLAGS} -flto=thin)
endif()

if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
  if(NOT "arm64" IN_LIST CMAKE_OSX_ARCHITECTURES)
    # Use sse2 by default. Change to latest simd extensions such as avx512 on supported architecture.
    set(LINUX_X86_64_OPT_FLAGS -msse2 -mfpmath=sse)
    if(UNIX AND NOT APPLE)
      set(LINUX_X86_64_OPT_FLAGS ${LINUX_X86_64_OPT_FLAGS} -mavx2)
    endif()
  endif()
endif()

# Add -ffast-math for speed, remove for testability.
# no-stack-check is added to mitigate stack alignment issue on Catalina where there is a bug with aligning stack-check instructions, and stack-check became default option
set(LINUX_RELEASE_CONFIG -fno-strict-aliasing ${LINUX_X86_64_OPT_FLAGS} -fno-stack-check -fomit-frame-pointer)
set(LINUX_DEBUG_CONFIG -fno-stack-check)

# Use default visiblity on UNIX otherwise a lot of the C++ symbols end up for exported and interpose'able
set(VW_LINUX_FLAGS $<$<CONFIG:Debug>:${LINUX_DEBUG_CONFIG}> $<$<CONFIG:Release>:${LINUX_RELEASE_CONFIG}> $<$<CONFIG:RelWithDebInfo>:${LINUX_RELEASE_CONFIG}>)
set(VW_WIN_FLAGS /MP /Zc:__cplusplus)

# Turn on warnings
set(WARNING_OPTIONS "")
if(WARNINGS)
  if(WIN32)
    set(WARNING_OPTIONS /W4)
  else()
    set(WARNING_OPTIONS -Wall -Wextra -Wpedantic)
  endif()
endif(WARNINGS)

# Turn on warnings as errors
set(WARNING_AS_ERROR_OPTIONS "")
if(WARNING_AS_ERROR)
  if(WIN32)
    set(WARNING_AS_ERROR_OPTIONS /WX)
  else()
    set(WARNING_AS_ERROR_OPTIONS -Werror)
  endif()
endif(WARNING_AS_ERROR)
