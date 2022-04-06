# CMake 3.13 will allow a relative path here
set(vw_common_sources
    ${CMAKE_CURRENT_LIST_DIR}/include/vw/common/hash.h
    ${CMAKE_CURRENT_LIST_DIR}/include/vw/common/future_compat.h
)

vw_add_library(
    NAME "common"
    TYPE "HEADER_ONLY"
    SOURCES ${vw_common_sources}
    ENABLE_INSTALL
)
