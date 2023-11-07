set(vw_io_sources
    include/vw/io/custom_streambuf.h
    include/vw/io/io_adapter.h
    include/vw/io/logger.h
    include/vw/io/owning_stream.h
    include/vw/io/errno_handling.h
    src/io_adapter.cc
    src/errno_handling.cc
    src/logger.cc
)

vw_add_library(
    NAME "io"
    TYPE "STATIC_ONLY"
    SOURCES ${vw_io_sources}
    PUBLIC_DEPS vw_common fmt::fmt
    PRIVATE_DEPS ZLIB::ZLIB ${spdlog_target}
    DESCRIPTION "Utilities for input and output"
    EXCEPTION_DESCRIPTION "Yes"
    ENABLE_INSTALL
)

if(WIN32)
  target_link_libraries(vw_io PRIVATE wsock32 ws2_32)
endif()

if(SPDLOG_SYS_DEP)
  # this doesn't get defined when using a system-installed spdlog
  target_compile_definitions(vw_io PUBLIC SPDLOG_FMT_EXTERNAL)
endif()

vw_add_test_executable(
    FOR_LIB "io"
    SOURCES
      tests/errno_test.cc
      tests/io_adapter_test.cc
      tests/ostream_test.cc
)
