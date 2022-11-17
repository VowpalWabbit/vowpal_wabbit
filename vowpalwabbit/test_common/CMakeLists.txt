set(vw_test_common_sources
    include/vw/test_common/test_common.h
)

# Do not install this target. Strictly for test code only.
vw_add_library(
    NAME "test_common"
    TYPE "HEADER_ONLY"
    SOURCES ${vw_test_common_sources}
    PUBLIC_DEPS vw_common vw_config
    DESCRIPTION "Test utilties"
    EXCEPTION_DESCRIPTION "Yes"
)
