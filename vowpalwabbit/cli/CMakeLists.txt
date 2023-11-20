vw_add_executable(
    NAME "cli"
    OVERRIDE_BIN_NAME "vw"
    SOURCES "src/main.cc"
    DEPS vw_core
    DESCRIPTION "Primary VW command line interface. The `vw` executable."

    ENABLE_INSTALL
)

if(NOT WIN32)
  find_file(HELP2MAN_EXECUTABLE help2man HINTS /bin /usr/bin /usr/local/bin)
  if (HELP2MAN_EXECUTABLE)
    add_custom_target(manpage ALL
      COMMAND ${HELP2MAN_EXECUTABLE}
        --no-info
        --name="Vowpal Wabbit -- fast online learning tool" $<TARGET_FILE:vw_cli_bin>
        --output=$<TARGET_FILE:vw_cli_bin>.1
      DEPENDS vw_cli_bin)

      if(VW_INSTALL)
        install(FILES $<TARGET_FILE:vw_cli_bin>.1 DESTINATION share/man/man1)
      endif()
  else()
    message(STATUS "help2man not found, please install it to generate manpages")
  endif()
endif()

if(STATIC_LINK_VW)
  target_link_libraries(vw_cli_bin PRIVATE ${unix_static_flag})
endif()

if (WIN32 AND VW_FEAT_DNN)
  find_package(Torch REQUIRED)
  set(TORCH_INSTALL_PREFIX ${TORCH_INSTALL_PREFIX} CACHE INTERNAL "Torch install prefix")
  file(GLOB TORCH_DLLS "${TORCH_INSTALL_PREFIX}/lib/*.dll")
  message("TORCH_DLLS: ${TORCH_DLLS}")  # Debugging line
  add_custom_command(TARGET vw_cli_bin
                     POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy_if_different
                     ${TORCH_DLLS}
                     $<TARGET_FILE_DIR:vw_cli_bin>)
endif (WIN32 AND VW_FEAT_DNN)