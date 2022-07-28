vw_add_executable(
    NAME "model_merger"
    OVERRIDE_BIN_NAME "vw-merge"
    SOURCES "src/main.cc"
    DEPS vw_core
    DESCRIPTION "Merge multiple models into a single model"
    ENABLE_INSTALL
)
