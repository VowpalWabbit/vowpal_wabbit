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

# Winsock32 should be available on Windows
if(WIN32)
  target_link_libraries(vw_allreduce PUBLIC wsock32 ws2_32)
else()
  target_compile_options(vw_allreduce PUBLIC ${linux_flags})
endif()
