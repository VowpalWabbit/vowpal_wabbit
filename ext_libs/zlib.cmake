# This is a patched version of zlib's CMakeLists.txt.
#  - Removes old version requirement for cmake
#  - Removes examples from build
#  - Prevents source directory from being written to during build
#  - Remove dynamic version of lib and associated pieces

set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS ON)

set(VERSION "1.2.11")

option(ASM686 "Enable building i686 assembly implementation")
option(AMD64 "Enable building amd64 assembly implementation")

include(CheckTypeSize)
include(CheckFunctionExists)
include(CheckIncludeFile)
include(CheckCSourceCompiles)
enable_testing()

check_include_file(sys/types.h HAVE_SYS_TYPES_H)
check_include_file(stdint.h    HAVE_STDINT_H)
check_include_file(stddef.h    HAVE_STDDEF_H)

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
check_type_size(off64_t OFF64_T)
if(HAVE_OFF64_T)
   add_definitions(-D_LARGEFILE64_SOURCE=1)
endif()
set(CMAKE_REQUIRED_DEFINITIONS) # clear variable

#
# Check for fseeko
#
check_function_exists(fseeko HAVE_FSEEKO)
if(NOT HAVE_FSEEKO)
    add_definitions(-DNO_FSEEKO)
endif()

#
# Check for unistd.h
#
check_include_file(unistd.h Z_HAVE_UNISTD_H)

if(MSVC)
    set(CMAKE_DEBUG_POSTFIX "d")
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
    add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
    include_directories(${CMAKE_CURRENT_LIST_DIR}/zlib)
endif()

configure_file(	${CMAKE_CURRENT_LIST_DIR}/zlib/zconf.h.cmakein
        ${CMAKE_CURRENT_BINARY_DIR}/zconf.h @ONLY)

#============================================================================
# zlib
#============================================================================

set(ZLIB_PUBLIC_HDRS
    ${CMAKE_CURRENT_BINARY_DIR}/zconf.h
    ${CMAKE_CURRENT_LIST_DIR}/zlib/zlib.h
)
set(ZLIB_PRIVATE_HDRS
    ${CMAKE_CURRENT_LIST_DIR}/zlib/crc32.h
    ${CMAKE_CURRENT_LIST_DIR}/zlib/deflate.h
    ${CMAKE_CURRENT_LIST_DIR}/zlib/gzguts.h
    ${CMAKE_CURRENT_LIST_DIR}/zlib/inffast.h
    ${CMAKE_CURRENT_LIST_DIR}/zlib/inffixed.h
    ${CMAKE_CURRENT_LIST_DIR}/zlib/inflate.h
    ${CMAKE_CURRENT_LIST_DIR}/zlib/inftrees.h
    ${CMAKE_CURRENT_LIST_DIR}/zlib/trees.h
    ${CMAKE_CURRENT_LIST_DIR}/zlib/zutil.h
)
set(ZLIB_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/zlib/adler32.c
    ${CMAKE_CURRENT_LIST_DIR}/zlib/compress.c
    ${CMAKE_CURRENT_LIST_DIR}/zlib/crc32.c
    ${CMAKE_CURRENT_LIST_DIR}/zlib/deflate.c
    ${CMAKE_CURRENT_LIST_DIR}/zlib/gzclose.c
    ${CMAKE_CURRENT_LIST_DIR}/zlib/gzlib.c
    ${CMAKE_CURRENT_LIST_DIR}/zlib/gzread.c
    ${CMAKE_CURRENT_LIST_DIR}/zlib/gzwrite.c
    ${CMAKE_CURRENT_LIST_DIR}/zlib/inflate.c
    ${CMAKE_CURRENT_LIST_DIR}/zlib/infback.c
    ${CMAKE_CURRENT_LIST_DIR}/zlib/inftrees.c
    ${CMAKE_CURRENT_LIST_DIR}/zlib/inffast.c
    ${CMAKE_CURRENT_LIST_DIR}/zlib/trees.c
    ${CMAKE_CURRENT_LIST_DIR}/zlib/uncompr.c
    ${CMAKE_CURRENT_LIST_DIR}/zlib/zutil.c
)

if(CMAKE_COMPILER_IS_GNUCC)
    if(ASM686)
        set(ZLIB_ASMS ${CMAKE_CURRENT_LIST_DIR}/zlib/contrib/asm686/match.S)
    elseif (AMD64)
        set(ZLIB_ASMS ${CMAKE_CURRENT_LIST_DIR}/zlib/contrib/amd64/amd64-match.S)
    endif ()

    if(ZLIB_ASMS)
        add_definitions(-DASMV)
        set_source_files_properties(${ZLIB_ASMS} PROPERTIES LANGUAGE C COMPILE_FLAGS -DNO_UNDERLINE)
    endif()
endif()

if(MSVC)
    if(ASM686)
        ENABLE_LANGUAGE(ASM_MASM)
        set(ZLIB_ASMS
            ${CMAKE_CURRENT_LIST_DIR}/zlib/contrib/masmx86/inffas32.asm
            ${CMAKE_CURRENT_LIST_DIR}/zlib/contrib/masmx86/match686.asm
        )
    elseif (AMD64)
        ENABLE_LANGUAGE(ASM_MASM)
        set(ZLIB_ASMS
            ${CMAKE_CURRENT_LIST_DIR}/zlib/contrib/masmx64/gvmat64.asm
            ${CMAKE_CURRENT_LIST_DIR}/zlib/contrib/masmx64/inffasx64.asm
        )
    endif()

    if(ZLIB_ASMS)
        add_definitions(-DASMV -DASMINF)
    endif()
endif()

# parse the full version number from zlib.h and include in ZLIB_FULL_VERSION
file(READ ${CMAKE_CURRENT_LIST_DIR}/zlib/zlib.h _zlib_h_contents)
string(REGEX REPLACE ".*#define[ \t]+ZLIB_VERSION[ \t]+\"([-0-9A-Za-z.]+)\".*"
    "\\1" ZLIB_FULL_VERSION ${_zlib_h_contents})

add_library(zlibstatic STATIC ${ZLIB_SRCS} ${ZLIB_ASMS} ${ZLIB_PUBLIC_HDRS} ${ZLIB_PRIVATE_HDRS})
add_library(zlibstatic ALIAS ZLIB::ZLIB)
target_include_directories(zlibstatic PUBLIC ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_LIST_DIR})

if(UNIX)
    # On unix-like platforms the library is almost always called libz
   set_target_properties(zlibstatic PROPERTIES OUTPUT_NAME z)
endif()
