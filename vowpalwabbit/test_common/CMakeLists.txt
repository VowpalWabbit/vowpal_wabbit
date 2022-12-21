set(vw_test_common_sources
    include/vw/test_common/test_common.h
    include/vw/test_common/matchers.h
    src/test_common.cc
)

# Do not install this target. Strictly for test code only.
vw_add_library(
    NAME "test_common"
    TYPE "STATIC_ONLY"
    SOURCES ${vw_test_common_sources}
    PUBLIC_DEPS vw_common vw_config vw_core
    DESCRIPTION "Test utilties"
    EXCEPTION_DESCRIPTION "Yes"
)

target_include_directories(vw_test_common PRIVATE $<TARGET_PROPERTY:vw_core,INCLUDE_DIRECTORIES>)