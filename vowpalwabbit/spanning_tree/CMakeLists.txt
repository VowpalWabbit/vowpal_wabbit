set(vw_spanning_tree_sources
    include/vw/spanning_tree/spanning_tree.h
    src/spanning_tree.cc
)

vw_add_library(
    NAME "spanning_tree"
    TYPE "STATIC_ONLY"
    SOURCES ${vw_spanning_tree_sources}
    PUBLIC_DEPS vw_common
    PRIVATE_DEPS ${LINK_THREADS} vw_io
    DESCRIPTION "Supporting code for connecting instances of VW for distributed learning"
    EXCEPTION_DESCRIPTION "Yes"
    ENABLE_INSTALL
)

if(WIN32)
  target_link_libraries(vw_spanning_tree PRIVATE wsock32 ws2_32)
endif()
