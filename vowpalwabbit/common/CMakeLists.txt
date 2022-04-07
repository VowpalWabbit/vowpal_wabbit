set(vw_common_sources
    include/vw/common/vw_exception.h
    include/vw/common/future_compat.h
    include/vw/common/hash.h
    include/vw/common/string_view.h
)

vw_add_library(
    NAME "common"
    TYPE "HEADER_ONLY"
    SOURCES ${vw_common_sources}
    PUBLIC_DEPS string_view_lite
    ENABLE_INSTALL
)
