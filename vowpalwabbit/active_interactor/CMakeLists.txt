IF(NOT WIN32)
    vw_add_executable(
        NAME "active_interactor"
        OVERRIDE_BIN_NAME "active_interactor"
        SOURCES "src/active_interactor.cc"
        DESCRIPTION "Tool for interacting with active mode reductions"
        ENABLE_INSTALL
    )

    target_compile_definitions(vw_active_interactor_bin PUBLIC _FILE_OFFSET_BITS=64)
endif()
