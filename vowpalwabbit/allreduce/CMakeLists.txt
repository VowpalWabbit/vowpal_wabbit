set(vw_allreduce_sources
    include/vw/allreduce/allreduce.h
    src/allreduce_sockets.cc
    src/allreduce_threads.cc
)

vw_add_library(
    NAME "allreduce"
    TYPE "STATIC_ONLY"
    SOURCES ${vw_allreduce_sources}
    PUBLIC_DEPS vw_common vw_io
    DESCRIPTION "Supporting library for thread or socket based distributed learning"
    EXCEPTION_DESCRIPTION "Yes"
    ENABLE_INSTALL
)

# Suppress MSVC C4117/C5308: Modifying reserved macro '_M_CEE' in allreduce.h for managed C++ compatibility
# (C4117 in older MSVC, C5308 in newer MSVC like VS2022)
if(MSVC)
  target_compile_options(vw_allreduce PRIVATE /wd4117 /wd5308)
endif()

# Winsock32 should be available on Windows
if(WIN32)
  target_link_libraries(vw_allreduce PUBLIC wsock32 ws2_32)
else()
  target_compile_options(vw_allreduce PUBLIC ${linux_flags})
endif()
