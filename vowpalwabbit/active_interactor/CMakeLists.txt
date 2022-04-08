vw_add_executable(
    NAME "active_interactor"
    OVERRIDE_BIN_NAME "active_interactor"
    SOURCES "src/active_interactor.cc"
    ENABLE_INSTALL
)

target_compile_definitions(vw_active_interactor_bin PUBLIC _FILE_OFFSET_BITS=64)
target_compile_options(vw_active_interactor_bin PUBLIC ${linux_flags})
