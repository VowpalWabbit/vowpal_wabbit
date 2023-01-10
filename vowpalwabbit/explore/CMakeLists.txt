set(vw_explore_sources
    include/vw/explore/explore_internal.h
    include/vw/explore/explore_error_codes.h
    include/vw/explore/explore.h
)

vw_add_library(
    NAME "explore"
    TYPE "HEADER_ONLY"
    SOURCES ${vw_explore_sources}
    PUBLIC_DEPS vw_common
    DESCRIPTION "Utilities for sampling and generating exploration distributions"
    EXCEPTION_DESCRIPTION "No"
    ENABLE_INSTALL
)

vw_add_test_executable(
    FOR_LIB "explore"
    SOURCES tests/explore_test.cc
    EXTRA_DEPS vw_core
)