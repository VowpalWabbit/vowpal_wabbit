vw_add_executable(
    NAME "model_merger"
    OVERRIDE_BIN_NAME "vw-merge"
    SOURCES "src/main.cc"
    DEPS vw_core vw_io vw_config vw_common
    DESCRIPTION "Merge multiple models into a single model"
)
