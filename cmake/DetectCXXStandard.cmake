include(CheckCXXCompilerFlag)

if(NOT MSVC)
  set(CXX_STANDARD17_FLAG "-std=c++17")
  set(CXX_STANDARD14_FLAG "-std=c++14")
  set(CXX_STANDARD11_FLAG "-std=c++11")
elseif(MSVC)
  set(CXX_STANDARD17_FLAG "/std:c++17")
  set(CXX_STANDARD14_FLAG "/std:c++14")
endif()

set(VW_CXX_STANDARD 11)
if(USE_LATEST_STD)
  # Check for C++ 17
  check_cxx_compiler_flag(${CXX_STANDARD17_FLAG} HAS_CXX17_FLAG)
  # CMAKE_CXX_STANDARD only supports 17 as of CMake version 3.8. Even if the compiler supports it we will have to fall back to 14.
  # TODO: once minimum version is >=3.7 collapse this into `${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.8.0"`
  if (HAS_CXX17_FLAG AND ((${CMAKE_VERSION} VERSION_EQUAL "3.8.0") OR (${CMAKE_VERSION} VERSION_GREATER "3.8.0")))
    set(VW_CXX_STANDARD 17)
  else()
    # Check for C++ 14
    check_cxx_compiler_flag(${CXX_STANDARD14_FLAG} HAS_CXX14_FLAG)
    if (HAS_CXX14_FLAG)
      set(VW_CXX_STANDARD 14)
    else()
      # MSVC support for 11 came in at the same time as 14, so we only check 14 for msvc.
      if(NOT MSVC)
        check_cxx_compiler_flag(${CXX_STANDARD11_FLAG} HAS_CXX11_FLAG)
        if (HAS_CXX11_FLAG)
          set(VW_CXX_STANDARD 11)
        else()
          message(FATAL_ERROR "Unsupported compiler -- VowpalWabbit requires C++11 support!")
        endif()
      elseif(MSVC)
        message(FATAL_ERROR "Unsupported compiler -- VowpalWabbit requires C++11 support!")
      endif()
    endif()
  endif()
endif()

message(STATUS "Using C++ standard: " ${VW_CXX_STANDARD})
set(CMAKE_CXX_STANDARD ${VW_CXX_STANDARD})
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
