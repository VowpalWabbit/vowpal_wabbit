set(vw_text_parser_sources
    include/vw/text_parser/parse_example_text.h
    src/parse_example_text.cc
)

vw_add_library(
    NAME "text_parser"
    TYPE "STATIC_ONLY"
    SOURCES ${vw_text_parser_sources}
    PUBLIC_DEPS vw_common vw_core vw_io
    DESCRIPTION "Read and write VW examples with text format."
    EXCEPTION_DESCRIPTION "Yes"
    ENABLE_INSTALL
)