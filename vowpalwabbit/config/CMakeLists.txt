set(vw_config_sources
    include/vw/config/cli_help_formatter.h
    include/vw/config/cli_options_serializer.h
    include/vw/config/help_formatter.h
    include/vw/config/option_group_definition.h
    include/vw/config/option.h
    include/vw/config/options_cli.h
    include/vw/config/options_name_extractor.h
    include/vw/config/options_serializer.h
    include/vw/config/options.h
    src/cli_help_formatter.cc
    src/cli_options_serializer.cc
    src/help_formatter.cc
    src/option_group_definition.cc
    src/options_cli.cc
    src/options_name_extractor.cc
    src/options.cc
    src/option.cc
)

vw_add_library(
    NAME "config"
    TYPE "STATIC_ONLY"
    SOURCES ${vw_config_sources}
    PUBLIC_DEPS vw_common
    PRIVATE_DEPS fmt::fmt
    DESCRIPTION "Option parsing, and command line utilities"
    EXCEPTION_DESCRIPTION "Yes"
    ENABLE_INSTALL
)

set(vw_config_test_sources
    tests/options_cli_test.cc
    tests/options_test.cc
)

vw_add_test_executable(
    FOR_LIB "config"
    SOURCES ${vw_config_test_sources}
    EXTRA_DEPS vw_test_common
)